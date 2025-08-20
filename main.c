#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "hashtable.h"
#include "ui.h"

#define no_of_particles 1000
#define upper_bound 1500
#define lower_bound 100

#define window_height 900
#define window_width 1600
#define particle_width 6
#define particle_height 6


typedef enum  {
    proton =1 ,
    electron=-1
}particle_type;

typedef struct {
    float x, y;      // Position
    float vx, vy;// Velocity
    particle_type pType;
} Particle;


void particle_array_init(Particle *particle_array){

    for (int i= 0 ; i < no_of_particles ; i++){
        double angle = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
        double random_vx = ((rand() % (5 - 1 + 1)+ 1) * cos(angle));
        double random_vy =  ((rand() % (6 - 2 + 1)+ 2) * sin(angle));



        float vx = (float)(random_vx * pow(-1,i));
        float vy = (float)(random_vy * pow(-1,i+1));
        particle_type particle_var;
        if (pow(-1, i)< 0) particle_var = proton;
        else   particle_var = electron;

        particle_array[i]=(Particle)
        {(float)((rand() % (upper_bound - lower_bound + 1)+ lower_bound)),
            (float)((rand() % (700 - 200 + 1)+ 200)) ,
            vx,
            vy,
            particle_var
        };
    }
}


void run(SDL_Window *window , SDL_Renderer *renderer, SDL_Window *ui_window,SDL_Renderer *ui_renderer,Particle *p,
         window_struct ui_window_info){

    bool ui_present = false;

    bool running = true;
    SDL_Event event;
    float repulsion_factor = 5.0f;

    while (running){
        entry *hashtable = hashtableint();
        Uint32 main_id = SDL_GetWindowID(window);
        Uint32 ui_id = ui_window ? SDL_GetWindowID(ui_window) : 0;

        // main window
        while (SDL_PollEvent(&event)){

            if (event.type == SDL_QUIT){
                running= false;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE){
                if (main_id == event.window.windowID) {
                    running = false;
                }
                if (ui_present == true & ui_id == event.window.windowID) {
                    cleanup(ui_window, ui_renderer);
                    ui_present = false;
                    ui_window = NULL;
                    ui_renderer = NULL;
                }

            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (ui_present == false) window_renderer_init(ui_window_info, &ui_window, &ui_renderer);
                ui_present = true;

            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB) {
                if (repulsion_factor > 50.0f) {
                    printf("\nThe value is now : %f", repulsion_factor);
                    repulsion_factor += 25.0f;
                }
                else repulsion_factor += 25.0f;


            }
        }
        if (!running) break;
        for (int i = 0; i < no_of_particles; i++) {
            // initial vx , vy = 0
            float bottom_line = (float)(window_height-particle_height);
            float top_line = (float)(0 + particle_height);

            //p[i].vy += 0.03f; // gravity

            p[i].x += p[i].vx;
            p[i].y += p[i].vy;


            float energy_loss = 1.0f ;         // 1 for no loss

            // Bottom
            if (p[i].y >= bottom_line) {
                p[i].y = bottom_line;
                p[i].vy *= -energy_loss; // bounce with 80% init velocity
            }

            // Top
            if (p[i].y <= top_line) {
                p[i].y = top_line;
                p[i].vy *= -energy_loss; // push downward
            }

            // Left wall
            if (p[i].x <= 0) {
                p[i].x = 0;
                p[i].vx *= -energy_loss;
            }

            // Right wall
            if (p[i].x >= 1600) {
                p[i].x = 1600;
                p[i].vx *= -energy_loss;
            }
        }

        //collision handling
        for (int i= 0 ; i < no_of_particles; i++) {
            int cell = (int)p[i].x/10;
            addindex(hashtable,cell,i);
        }

        int neighbour_calc[3] = {-1 , 0 , 1};
        for (int i = 0; i < no_of_particles; i++) {
            for (int z = 0 ; z < 3; z++) {
                // force calc as k , outer loop z for neighbouring cells
                int cell = (int) p[i].x / 10 + neighbour_calc[z];

                int *bucket = getindex(hashtable, cell);
                int bucket_count = bucketcount(hashtable, cell);

                for (int k = 0 ; k < bucket_count; k++) {

                    int j = bucket[k];
                    if (j <= i) continue;

                    float dx = p[i].x - p[j].x;
                    float dy = p[i].y - p[j].y;
                    float distSq = (dx * dx) + (dy * dy);
                    float dist = sqrtf(distSq);


                    if (dist == 0.0f) dist = 0.0001f; // prevent div-by-zero


                    // d^2 = repulsion factor / Fmin
                    const float f_min = 0.1f;
                    const float min_distSq = repulsion_factor / f_min;
                    if (distSq > min_distSq ) continue;


                    // unit vector between two part
                    float nx = dx / dist;
                    float ny = dy / dist;

                    // Attraction / Repulsion factor
                    // Same type → repulsion (+), different type → attraction (-)
                    float forceDir = (p[i].pType == p[j].pType) ? 1.0f : -1.0f;

                    // Strength of force (inverse square falloff)
                    // without epsilon all particles reach forces that end to inf very fast
                    const float epsilon = 0.001f;
                    float distance_factor = distSq + epsilon;
                    float strength = forceDir * (repulsion_factor / distance_factor);

                    // Apply acceleration
                    p[i].vx += nx * strength;
                    p[i].vy += ny * strength;
                    p[j].vx -= nx * strength;
                    p[j].vy -= ny * strength;

                }

            }
        }



        //clear last frame
        SDL_SetRenderDrawColor(renderer,30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i= 0 ; i < no_of_particles ; i++) {
            if (p[i].pType== proton) SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            SDL_Rect particle_rect = {(int)p[i].x, (int)p[i].y, particle_width, particle_height};
            SDL_RenderFillRect(renderer, &particle_rect);

        }

        free(hashtable);
        SDL_RenderPresent(renderer);
        if (ui_present) SDL_RenderPresent(ui_renderer);
        SDL_Delay((Uint32)16); // 60 fps
    }
    cleanup(ui_window, ui_renderer);
         }

         int main(){
             window_struct window_info = {"Particle Sim",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width , window_height , 0};
             window_struct ui_window_info = {"UI",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,500 ,500 ,0};
             SDL_Window *window = NULL;
             SDL_Renderer *renderer= NULL;
             SDL_Window *ui_window = NULL;
             SDL_Renderer *ui_renderer= NULL;

             Particle particle_array[no_of_particles];

             window_renderer_init(window_info, &window , &renderer);
             particle_array_init(particle_array);

             run(window , renderer ,ui_window, ui_renderer, particle_array, ui_window_info);

             cleanup(window , renderer);

             return 0;
         }
