#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "hashtable.h"

#define no_of_particles 1000
#define upper_bound 1500
#define lower_bound 100
#define spawn_y_min 200
#define spawn_y_max 700

#define window_height 900
#define window_width 1600
#define particle_width 6
#define particle_height 6

#define grid_cell_size 10
#define target_frame_ms 16
#define initial_force 5.0f
#define force_increment 25.0f
#define energy_loss_factor 1.0f
#define epsilon 0.01f
#define f_min 0.1f


void cleanup(SDL_Window *window, SDL_Renderer *renderer);

typedef enum  {
    proton =1 ,
    electron=-1
}particle_type;

typedef struct {
    float x, y;      // Position
    float vx, vy;// Velocity
    particle_type pType;
} Particle;


typedef struct{
        const char *title;
        int x_pos, y_pos;
        int width ;
        int height;
        Uint32 flags;
}window_struct;



void init(window_struct window_info, SDL_Window **window, SDL_Renderer **renderer, Particle *particle_array){
    SDL_SetHint("SDL_VIDEODRIVER", "x11");
    if (SDL_Init(SDL_INIT_VIDEO) <0 ){
        printf("Could not init sdl : %s\n", SDL_GetError());
        SDL_Quit();
    }




    *window = SDL_CreateWindow(window_info.title,window_info.x_pos , window_info.y_pos , window_info.width , window_info.height, window_info.flags);

    if (*window == NULL){
        printf("error creating window: %s\n", SDL_GetError());
        SDL_Quit();
    }

    *renderer = SDL_CreateRenderer(*window, -1 , SDL_RENDERER_ACCELERATED);


    if(*renderer == NULL){
        printf("Renderer init failed:%s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
    }


    for (int i= 0 ; i < no_of_particles ; i++){
        double angle = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
        double speed = (rand() % 5) + 1;
        float vx = (float)(speed * cos(angle));
        float vy = (float)(speed * sin(angle));

        particle_type particle_var = (i % 2 == 0) ? proton : electron;

        particle_array[i]=(Particle){(float)((rand() % (upper_bound - lower_bound + 1)+ lower_bound)), (float)((rand() % (spawn_y_max - spawn_y_min + 1)+ spawn_y_min)) , vx, vy, particle_var};
    }
}



void run(SDL_Window *window , SDL_Renderer *renderer, Particle *p){

    bool running = true;
    SDL_Event event;
    float repulsion_factor = initial_force;

    while (running){
        entry *hashtable = hashtableint();
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running= false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                running = false;

            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                repulsion_factor += force_increment;
            }

        }
        for (int i = 0; i < no_of_particles; i++) {
            float bottom_line = (float)(window_height-particle_height);
            float top_line = (float)(0 + particle_height);

            p[i].x += p[i].vx;
            p[i].y += p[i].vy;

            if (p[i].y >= bottom_line) {
                p[i].y = bottom_line;
                p[i].vy *= -energy_loss_factor;
            }

            if (p[i].y <= top_line) {
                p[i].y = top_line;
                p[i].vy *= -energy_loss_factor;
            }

            if (p[i].x <= 0) {
                p[i].x = 0;
                p[i].vx *= -energy_loss_factor;
            }

            if (p[i].x >= (float)(window_width - particle_width)) {
                p[i].x = (float)(window_width - particle_width);
                p[i].vx *= -energy_loss_factor;
            }
        }

        for (int i= 0 ; i < no_of_particles; i++) {
            int cell = (int)p[i].x / grid_cell_size;
            addindex(hashtable,cell,i);
        }

        int neighbour_calc[3] = {-1 , 0 , 1};
        for (int i = 0; i < no_of_particles; i++) {
            for (int z = 0 ; z < 3; z++) {
                int cell = (int)(p[i].x / grid_cell_size) + neighbour_calc[z];

                int *bucket = getindex(hashtable, cell);
                int bucket_count = bucketcount(hashtable, cell);

                for (int k = 0 ; k < bucket_count; k++) {

                    int j = bucket[k];
                    if (j <= i) continue;

                    float dx = p[i].x - p[j].x;
                    float dy = p[i].y - p[j].y;
                    float distSq = (dx * dx) + (dy * dy);
                    float dist = sqrtf(distSq);


                    if (dist == 0.0f) dist = 0.0001f;

                    const float min_distSq = repulsion_factor / f_min;
                    if (distSq > min_distSq ) continue;


                    float nx = dx / dist;
                    float ny = dy / dist;

                    float forceDir = (p[i].pType == p[j].pType) ? 1.0f : -1.0f;

                    float strength = forceDir * (repulsion_factor / (distSq + epsilon));

                    p[i].vx += nx * strength;
                    p[i].vy += ny * strength;
                    p[j].vx -= nx * strength;
                    p[j].vy -= ny * strength;

                }

            }
        }



        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i= 0 ; i < no_of_particles ; i++) {
            if (p[i].pType== proton) SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            SDL_Rect particle_rect = {(int)p[i].x, (int)p[i].y, particle_width, particle_height};
            SDL_RenderFillRect(renderer, &particle_rect);

        }

        free(hashtable);
        SDL_RenderPresent(renderer);
        SDL_Delay((Uint32)target_frame_ms);
    }




}

void cleanup(SDL_Window *window, SDL_Renderer *renderer){

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}

int main(){
    window_struct window_info = {"Particle Sim",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width , window_height , 0};
    SDL_Window *window = NULL;
    SDL_Renderer *renderer= NULL;
    Particle particle_array[no_of_particles];

    init(window_info, &window , &renderer ,particle_array);

    run(window , renderer, particle_array);

    cleanup(window , renderer);

    return 0;
}