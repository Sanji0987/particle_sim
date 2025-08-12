#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>


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
}partype;

typedef struct {
    float x, y;      // Position
    float vx, vy;// Velocity
    partype partype;
} Particle;


typedef struct{
        const char *title;
        int xpos, ypos;
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




    *window = SDL_CreateWindow(window_info.title,window_info.xpos , window_info.ypos , window_info.width , window_info.height, window_info.flags);

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
    partype parvar;
    for (int i= 0 ; i < no_of_particles ; i++){
        double angle = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
        float random_vx = (float) ((rand() % (5 - 1 + 1)+ 1) * cos(angle));
        float random_vy = (float) ((rand() % (6 - 2 + 1)+ 2) * sin(angle));



        float vx = random_vx * pow(-1,i);
        float vy = random_vy * pow(-1,i+1);

        if (pow(-1, i)< 0) parvar = proton;
        else  parvar = electron;

        particle_array[i]=(Particle){(float)((rand() % (upper_bound - lower_bound + 1)+ lower_bound)), (float)((rand() % (700 - 200 + 1)+ 200)) , vx, vy, parvar};
    }

}

void run(SDL_Window *window , SDL_Renderer *renderer, Particle *p){

    bool running = true;
    SDL_Event event;

    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running= false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                running = false;

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
        float repulsion_factor;

        for (int i = 0; i < no_of_particles; i++) {
            for (int j = i + 1; j < no_of_particles; j++) {

                float dx = p[i].x - p[j].x;
                float dy = p[i].y - p[j].y;
                float distSq = (dx * dx) + (dy * dy);
                float dist = sqrtf(distSq);



                if (dist == 0.0f) dist = 0.0001f; // prevent div-by-zero

                // unit vector between two part
                float nx = dx / dist;
                float ny = dy / dist;

                // Attraction / Repulsion factor
                // Same type → repulsion (+), different type → attraction (-)
                float forceDir = (p[i].partype == p[j].partype) ? 1.0f : -1.0f;

                // Strength of force (inverse square falloff)
                repulsion_factor = 5.0f;
                float strength = forceDir * (repulsion_factor / distSq);

                if (strength > 5.0f) strength = 5.0f;
                if (strength < -5.0f) strength = -5.0f;


                // Apply acceleration

                p[i].vx += nx * strength;
                p[i].vy += ny * strength;
                p[j].vx -= nx * strength;
                p[j].vy -= ny * strength;
            }
        }


        //clear last frame
        SDL_SetRenderDrawColor(renderer,30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i= 0 ; i < no_of_particles ; i++) {
            if (p[i].partype== proton) SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            SDL_Rect particle_rect = {(int)p[i].x, (int)p[i].y, particle_width, particle_height};
            SDL_RenderFillRect(renderer, &particle_rect);

        }


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