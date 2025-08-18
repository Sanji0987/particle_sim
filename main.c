#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "hashtable.h"

#define NK_IMPLEMENTATION
#include "nuklear.h"

#define no_of_particles 1000
#define upper_bound 1500
#define lower_bound 100

#define window_height 900
#define window_width 1600
#define particle_width 6
#define particle_height 6


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
        SDL_Quit();;
    }

    *renderer = SDL_CreateRenderer(*window, -1 , SDL_RENDERER_ACCELERATED);


    if(*renderer == NULL){
        printf("Renderer init failed:%s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
    }


    for (int i= 0 ; i < no_of_particles ; i++){
        double angle = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
        double random_vx = ((rand() % (5 - 1 + 1)+ 1) * cos(angle));
        double random_vy =  ((rand() % (6 - 2 + 1)+ 2) * sin(angle));



        float vx = (float)(random_vx * pow(-1,i));
        float vy = (float)(random_vy * pow(-1,i+1));
        particle_type particle_var;
        if (pow(-1, i)< 0) particle_var = proton;
        else   particle_var = electron;

        particle_array[i]=(Particle){(float)((rand() % (upper_bound - lower_bound + 1)+ lower_bound)), (float)((rand() % (700 - 200 + 1)+ 200)) , vx, vy, particle_var};
    }
}



void run(SDL_Window *window , SDL_Renderer *renderer, Particle *p){

    bool running = true;
    SDL_Event event;
    float repulsion_factor = 5.0f;

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
                if (repulsion_factor > 50.0f) {
                    printf("\ntoo too much value\n");
                }
                else repulsion_factor += 25.0f;

            }

        }
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
            addindex(hashtable,(int)p[i].x/10,i);
        }

        for (int i = 0; i < no_of_particles; i++) {
            int cell = (int)p[i].x/10;
            int left_cell = cell -1;
            int right_cell = cell +1;
            // abstrct the bucket code

            int *bucket = getindex( hashtable, cell);
            int bucket_count= bucketcount(hashtable, cell) ;

            for (int k = 0 ; k < bucket_count; k++) {


                int j = bucket[k];
                if (j <= i) continue;

                float dx = p[i].x - p[j].x;
                float dy = p[i].y - p[j].y;
                float distSq = (dx * dx) + (dy * dy);
                float dist = sqrtf(distSq);


                if (dist == 0.0f) dist = 0.0001f; // prevent div-by-zero

                // d^2 = repulsion factor / Fmin
                const float f_min = 0.01f;
                const float min_distSq = repulsion_factor / f_min;
                if (distSq > min_distSq ) continue;


                // unit vector between two part
                float nx = dx / dist;
                float ny = dy / dist;

                // Attraction / Repulsion factor
                // Same type → repulsion (+), different type → attraction (-)
                float forceDir = (p[i].pType == p[j].pType) ? 1.0f : -1.0f;

                // Strength of force (inverse square falloff)
                float strength = forceDir * (repulsion_factor / distSq);

                if (strength > 15.0f) strength = 30.0f;
                if (strength < -15.0f) strength = -30.0f;

                // Apply acceleration

                p[i].vx += nx * strength;
                p[i].vy += ny * strength;
                p[j].vx -= nx * strength;
                p[j].vy -= ny * strength;

            }

            bucket = getindex( hashtable, left_cell);
            bucket_count= bucketcount(hashtable, left_cell) ;
            for (int k = 0 ; k < bucket_count; k++) {


                int j = bucket[k];
                if (j <= i) continue;

                float dx = p[i].x - p[j].x;
                float dy = p[i].y - p[j].y;
                float distSq = (dx * dx) + (dy * dy);
                float dist = sqrtf(distSq);


                if (dist == 0.0f) dist = 0.0001f; // prevent div-by-zero

                // d^2 = repulsion factor / Fmin
                const float f_min = 0.01f;
                const float min_distSq = repulsion_factor / f_min;
                if (distSq > min_distSq ) continue;


                // unit vector between two part
                float nx = dx / dist;
                float ny = dy / dist;

                // Attraction / Repulsion factor
                // Same type → repulsion (+), different type → attraction (-)
                float forceDir = (p[i].pType == p[j].pType) ? 1.0f : -1.0f;

                // Strength of force (inverse square falloff)
                float strength = forceDir * (repulsion_factor / distSq);

                if (strength > 15.0f) strength = 30.0f;
                if (strength < -15.0f) strength = -30.0f;

                // Apply acceleration

                p[i].vx += nx * strength;
                p[i].vy += ny * strength;
                p[j].vx -= nx * strength;
                p[j].vy -= ny * strength;

            }

            bucket = getindex( hashtable, right_cell);
            bucket_count= bucketcount(hashtable, right_cell) ;
            for (int k = 0 ; k < bucket_count; k++) {


                int j = bucket[k];
                if (j <= i) continue;

                float dx = p[i].x - p[j].x;
                float dy = p[i].y - p[j].y;
                float distSq = (dx * dx) + (dy * dy);
                float dist = sqrtf(distSq);


                if (dist == 0.0f) dist = 0.0001f; // prevent div-by-zero

                // d^2 = repulsion factor / Fmin
                const float f_min = 0.01f;
                const float min_distSq = repulsion_factor / f_min;
                if (distSq > min_distSq ) continue;


                // unit vector between two part
                float nx = dx / dist;
                float ny = dy / dist;

                // Attraction / Repulsion factor
                // Same type → repulsion (+), different type → attraction (-)
                float forceDir = (p[i].pType == p[j].pType) ? 1.0f : -1.0f;

                // Strength of force (inverse square falloff)
                float strength = forceDir * (repulsion_factor / distSq);

                if (strength > 15.0f) strength = 30.0f;
                if (strength < -15.0f) strength = -30.0f;

                // Apply acceleration

                p[i].vx += nx * strength;
                p[i].vy += ny * strength;
                p[j].vx -= nx * strength;
                p[j].vy -= ny * strength;

            }

        }




        // my weird way of a debug
        /*
        for (int i= 0 ; i < hashtable[calchash(0)].capacity; i++) {
            if (hashtable[calchash(0)].index_arr[i] != 0 && hashtable[calchash(0)].index_arr[i] != '\0' ) {
                printf("%d\n", hashtable[calchash(0)].index_arr[i]);
                printf("\nTHIS IS A BREAK LINE\n");
            }
        }
        */



        /*
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < no_of_particles; i++) {
            for (int j = i + 1; j < no_of_particles; j++) {

                float dx = p[i].x - p[j].x;
                float dy = p[i].y - p[j].y;
                float distSq = (dx * dx) + (dy * dy);
                float dist = sqrt(distSq);



                if (dist == 0.0f) dist = 0.0001f; // prevent div-by-zero

                // unit vector between two part
                float nx = dx / dist;
                float ny = dy / dist;

                // Attraction / Repulsion factor
                // Same type → repulsion (+), different type → attraction (-)
                float forceDir = (p[i].pType == p[j].pType) ? 1.0f : -1.0f;

                // Strength of force (inverse square falloff)
                float strength = forceDir * (repulsion_factor / distSq);

                if (strength > 15.0f) strength = 30.0f;
                if (strength < -15.0f) strength = -30.0f;


                // Apply acceleration

                p[i].vx += nx * strength;
                p[i].vy += ny * strength;
                p[j].vx -= nx * strength;
                p[j].vy -= ny * strength;
            }
        }
        */




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
        SDL_Delay((Uint32)16); // 60 fps
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