#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <cuda_runtime_api.h> 
#include <cuda.h>

#include <SDL.h>

#include "gui.h"

// Time step size
#define DT 0.075

// Gravitational constant
#define G 100

// Threads per block
#define THREADS_PER_BLOCK 32

// This struct holds data for a single star
typedef struct star {
    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;
    float mass;
} star_t;

// Generate a random float in a given range
float drand(float min, float max) {
    return ((float)rand() / RAND_MAX) * (max - min) + min;
}

// Compute the radius of a star based on its mass
float star_radius(float mass) {
    return sqrt(mass);
}

__global__ void updatePosition(star_t* starsGPU, int num_stars){
    // Calculate the index of starsGPU this should handle. If it exceeds the bounds, return
    int i = threadIdx.x + blockIdx.x * THREADS_PER_BLOCK;
    if (i >= num_stars) return;

    starsGPU[i].x_position += starsGPU[i].x_velocity * DT;
    starsGPU[i].y_position += starsGPU[i].y_velocity * DT;
    for(int j=0; j<num_stars; j++) {
        // Don't compute the force of a star on itself
        if(i == j) continue;

        // Compute the distance between the two stars in each dimension
        float x_diff = starsGPU[i].x_position - starsGPU[j].x_position;
        float y_diff = starsGPU[i].y_position - starsGPU[j].y_position;

        // Compute the magnitude of the distance vector
        float dist = sqrt(x_diff * x_diff + y_diff * y_diff);

        // Normalize the distance vector components
        x_diff /= dist;
        y_diff /= dist;

        // Keep a minimum distance, otherwise we get 
        float combined_radius = sqrt(starsGPU[i].mass) + sqrt(starsGPU[j].mass);
        if(dist < combined_radius) {
            dist = combined_radius;
        }

        // Compute the x and y accelerations
        float x_acceleration = -x_diff * G * starsGPU[j].mass / (dist * dist);
        float y_acceleration = -y_diff * G * starsGPU[j].mass / (dist * dist);

        // Update the star velocity
        starsGPU[i].x_velocity += x_acceleration * DT;
        starsGPU[i].y_velocity += y_acceleration * DT;

        // Handle edge collisiosn
        if(starsGPU[i].x_position < 0 && starsGPU[i].x_velocity < 0) starsGPU[i].x_velocity *= -0.5;
        if(starsGPU[i].x_position >= SCREEN_WIDTH && starsGPU[i].x_velocity > 0) starsGPU[i].x_velocity *= -0.5;
        if(starsGPU[i].y_position < 0 && starsGPU[i].y_velocity < 0) starsGPU[i].y_velocity *= -0.5;
        if(starsGPU[i].y_position >= SCREEN_HEIGHT && starsGPU[i].y_velocity > 0) starsGPU[i].y_velocity *= -0.5;
    }
}

int main(int argc, char** argv) {
    // Initialize the graphical interface
    gui_init();

    // Run as long as this is true
    bool running = true;

    // Is the mouse currently clicked?
    bool clicked = false;

    // This will hold our array of stars for CPU and GPU
    star_t* stars = NULL;
    star_t* starsGPU = NULL;
    int num_stars = 0;
    // Keep a count of the previous count of stars, so that we only copy if the number of stars increase
    int prevStarCount = num_stars;

    // Start main loop
    while(running) {
        // Check for events
        SDL_Event event;
        while(SDL_PollEvent(&event) == 1) {
            // If the event is a quit event, then leave the loop
            if(event.type == SDL_QUIT) running = false;
        }

        // Get the current mouse state
        int mouse_x, mouse_y;
        uint32_t mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);

        // Is the mouse pressed?
        if(mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            // Is this the beginning of a mouse click?
            if(!clicked) {
                clicked = true;
                stars = (star_t*)realloc(stars, (num_stars + 1) * sizeof(star_t));
                stars[num_stars].x_position = mouse_x + drand(-1, 1);
                stars[num_stars].y_position = mouse_y + drand(-1, 1);
                stars[num_stars].x_velocity = 0;
                stars[num_stars].y_velocity = 0;
                // Generate a random mass skewed toward small sizes
                stars[num_stars].mass = drand(0, 1) * drand(0, 1) * 50;
                num_stars++;
            }
        } else {
            // The mouse click is finished
            clicked = false;
        }

        // Draw stars
        for(int i=0; i<num_stars; i++) {
            color_t color = {255, 255, 255, 255};
            gui_draw_circle(stars[i].x_position, stars[i].y_position, star_radius(stars[i].mass), color);
        }

        // If there are different number of stars (i.e. stars are added) then, free starsGPU list, allocate and copy stars to starsGPU
        if (num_stars != prevStarCount){
            if (starsGPU != NULL) cudaFree(starsGPU);
            cudaMalloc((void**) &starsGPU, num_stars*sizeof(star_t));
            if (cudaMemcpy(starsGPU, stars, sizeof(star_t)*num_stars, cudaMemcpyHostToDevice) != cudaSuccess){
                fprintf(stderr, "Failed to copy stars to starsGPU\n");
            }
        }

        // Calculate the number of blocks that should be run
        int blocks = (num_stars + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

        // Update stars, run calculations on GPU, wait for completion and copy back to host to display
        updatePosition<<<blocks, THREADS_PER_BLOCK>>>(starsGPU, num_stars);
        if(cudaDeviceSynchronize() != cudaSuccess) {
            fprintf(stderr, "CUDA Error: %s\n", cudaGetErrorString(cudaPeekAtLastError()));         
        }
        if (cudaMemcpy(stars, starsGPU, sizeof(star_t)*num_stars, cudaMemcpyDeviceToHost) != cudaSuccess){
            fprintf(stderr, "Failed to copy starsGPU to stars\n");
        }
        // update prevStarCount
        prevStarCount = num_stars;

        // Display the rendered image
        gui_update_display();

        // Fade out the rendered image to leave movement trails
        gui_fade(0.85);
    }

    // Free the stars array
    free(stars);

    // Free the starsGPU array
    cudaFree(starsGPU);
    // Clean up the graphical interface
    gui_shutdown();

    return 0;
}
