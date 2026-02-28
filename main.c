#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include "hashtable.h"

#define no_of_particles 1000
#define max_particles 5000
#define world_width 16000
#define world_height 9000
#define upper_bound 14000
#define lower_bound 2000
#define spawn_y_min 1000
#define spawn_y_max 8000

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
#define proton_mass 3.0f
#define electron_mass 1.0f
#define damping_factor 0.999f
#define cam_pan_speed 10.0f
#define cam_initial_zoom 0.1f
#define cam_zoom_min 0.01f
#define cam_zoom_max 10.0f
#define circle_tex_size 32


void cleanup(SDL_Window *window, SDL_Renderer *renderer);

typedef enum  {
    proton =1 ,
    electron=-1
}particle_type;

typedef struct {
    float x, y;      // Position
    float vx, vy;// Velocity
    particle_type pType;
    float mass;
    bool visible;
} Particle;


typedef struct{
    const char *title;
    int x_pos, y_pos;
    int width ;
    int height;
    Uint32 flags;
}window_struct;



SDL_Texture *create_circle_texture(SDL_Renderer *renderer) {
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, circle_tex_size, circle_tex_size, 32, SDL_PIXELFORMAT_RGBA8888);
    if (surf == NULL) return NULL;
    SDL_LockSurface(surf);
    Uint32 *pixels = (Uint32 *)surf->pixels;
    float center = (circle_tex_size - 1) / 2.0f;
    float radius = center;
    for (int y = 0; y < circle_tex_size; y++) {
        for (int x = 0; x < circle_tex_size; x++) {
            float dx = x - center;
            float dy = y - center;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist <= radius)
                pixels[y * circle_tex_size + x] = 0xFFFFFFFF;
            else
                pixels[y * circle_tex_size + x] = 0x00000000;
        }
    }
    SDL_UnlockSurface(surf);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (tex != NULL)
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

void init(window_struct window_info, SDL_Window **window, SDL_Renderer **renderer, Particle *particle_array){
    SDL_SetHint("SDL_VIDEODRIVER", "x11");
    if (SDL_Init(SDL_INIT_VIDEO) <0 ){
        fprintf(stderr, "Could not init sdl : %s\n", SDL_GetError());
        exit(1);
    }




    *window = SDL_CreateWindow(window_info.title,window_info.x_pos , window_info.y_pos , window_info.width , window_info.height, window_info.flags);

    if (*window == NULL){
        fprintf(stderr, "error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    *renderer = SDL_CreateRenderer(*window, -1 , SDL_RENDERER_ACCELERATED);


    if(*renderer == NULL){
        fprintf(stderr, "Renderer init failed:%s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        exit(1);
    }


    for (int i= 0 ; i < max_particles ; i++){
        double angle = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
        double speed = (rand() % 5) + 1;
        float vx = (float)(speed * cos(angle));
        float vy = (float)(speed * sin(angle));

        particle_type particle_var = (i % 2 == 0) ? proton : electron;
        float mass_val = (particle_var == proton) ? proton_mass : electron_mass;

        particle_array[i]=(Particle){(float)((rand() % (upper_bound - lower_bound + 1)+ lower_bound)), (float)((rand() % (spawn_y_max - spawn_y_min + 1)+ spawn_y_min)) , vx, vy, particle_var, mass_val, true};
    }
}



bool slider_open = false;
bool help_open = false;

void run(SDL_Window *window , SDL_Renderer *renderer, Particle *p, int *shared_count){

    bool running = true;
    SDL_Event event;
    float repulsion_factor = initial_force;
    const int64_t hash_prime_x = 73856093;
    const int64_t hash_prime_y = 19349669;
    float cam_x = 0.0f;
    float cam_y = 0.0f;
    float cam_zoom = cam_initial_zoom;
    SDL_Texture *circle_tex = create_circle_texture(renderer);

    while (running){
        int active_count = *shared_count;
        if (active_count < 1) active_count = 1;
        if (active_count > max_particles) active_count = max_particles;

        entry *hashtable = hashtableint();
        if (hashtable == NULL) break;
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
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p && !slider_open) {
                slider_open = true;
                if (fork() == 0) {
                    execlp("python3", "python3", "controls.py", "slider", NULL);
                    _exit(1);
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h && !help_open) {
                help_open = true;
                if (fork() == 0) {
                    execlp("python3", "python3", "controls.py", "help", NULL);
                    _exit(1);
                }
            }
            if (event.type == SDL_MOUSEWHEEL) {
                const float zoom_in = 1.1f;
                const float zoom_out = 0.9f;
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                float world_mx = cam_x + mx / cam_zoom;
                float world_my = cam_y + my / cam_zoom;
                if (event.wheel.y > 0)
                    cam_zoom *= zoom_in;
                else if (event.wheel.y < 0)
                    cam_zoom *= zoom_out;
                if (cam_zoom < cam_zoom_min) cam_zoom = cam_zoom_min;
                if (cam_zoom > cam_zoom_max) cam_zoom = cam_zoom_max;
                cam_x = world_mx - mx / cam_zoom;
                cam_y = world_my - my / cam_zoom;
            }

        }
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float pan_step = cam_pan_speed / cam_zoom;
        if (keys[SDL_SCANCODE_W]) cam_y -= pan_step;
        if (keys[SDL_SCANCODE_S]) cam_y += pan_step;
        if (keys[SDL_SCANCODE_A]) cam_x -= pan_step;
        if (keys[SDL_SCANCODE_D]) cam_x += pan_step;
        for (int i = 0; i < active_count; i++) {
            p[i].vx *= damping_factor;
            p[i].vy *= damping_factor;
            p[i].x += p[i].vx;
            p[i].y += p[i].vy;

            int screen_x = (int)((p[i].x - cam_x) * cam_zoom);
            int screen_y = (int)((p[i].y - cam_y) * cam_zoom);
            int draw_w = (int)(particle_width * cam_zoom);
            int draw_h = (int)(particle_height * cam_zoom);
            if (draw_w < 1) draw_w = 1;
            if (draw_h < 1) draw_h = 1;
            p[i].visible = !(screen_x + draw_w < 0 || screen_x > window_width ||
                             screen_y + draw_h < 0 || screen_y > window_height);
        }

        for (int i= 0 ; i < active_count; i++) {
            int cell_x = (int)p[i].x / grid_cell_size;
            int cell_y = (int)p[i].y / grid_cell_size;
            int64_t cell = cell_x * hash_prime_x + cell_y * hash_prime_y;
            addindex(hashtable,cell,i);
        }

        for (int i = 0; i < active_count; i++) {
            int base_cx = (int)(p[i].x / grid_cell_size);
            int base_cy = (int)(p[i].y / grid_cell_size);
            for (int ox = -1; ox <= 1; ox++) {
                int64_t hashed_cx = (base_cx + ox) * hash_prime_x;
                for (int oy = -1; oy <= 1; oy++) {
                    int64_t cell = hashed_cx + (base_cy + oy) * hash_prime_y;

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

                        p[i].vx += (nx * strength) / p[i].mass;
                        p[i].vy += (ny * strength) / p[i].mass;
                        p[j].vx -= (nx * strength) / p[j].mass;
                        p[j].vy -= (ny * strength) / p[j].mass;

                    }

                }
            }
        }



        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i= 0 ; i < active_count ; i++) {
            if (!p[i].visible) continue;

            int screen_x = (int)((p[i].x - cam_x) * cam_zoom);
            int screen_y = (int)((p[i].y - cam_y) * cam_zoom);
            int draw_w = (int)(particle_width * cam_zoom);
            int draw_h = (int)(particle_height * cam_zoom);
            if (draw_w < 1) draw_w = 1;
            if (draw_h < 1) draw_h = 1;

            if (p[i].pType== proton)
                SDL_SetTextureColorMod(circle_tex, 0, 0, 255);
            else
                SDL_SetTextureColorMod(circle_tex, 255, 0, 0);

            SDL_Rect particle_rect = {screen_x, screen_y, draw_w, draw_h};
            SDL_RenderCopy(renderer, circle_tex, NULL, &particle_rect);

        }

        hashtablefree(hashtable);
        SDL_RenderPresent(renderer);
        SDL_Delay((Uint32)target_frame_ms);
    }

    if (circle_tex != NULL) SDL_DestroyTexture(circle_tex);
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer){

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}

int main(){
    signal(SIGCHLD, SIG_IGN);
    window_struct window_info = {"Particle Sim",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width , window_height , 0};
    SDL_Window *window = NULL;
    SDL_Renderer *renderer= NULL;
    Particle *particle_array = malloc(sizeof(Particle) * max_particles);
    if (particle_array == NULL){
        fprintf(stderr, "Failed to allocate particle array\n");
        exit(1);
    }

    int shm_fd = shm_open("/particle_count", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1){
        fprintf(stderr, "Failed to create shared memory\n");
        free(particle_array);
        exit(1);
    }
    ftruncate(shm_fd, sizeof(int));
    int *shared_count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_count == MAP_FAILED){
        fprintf(stderr, "Failed to mmap shared memory\n");
        close(shm_fd);
        free(particle_array);
        exit(1);
    }
    *shared_count = no_of_particles;

    init(window_info, &window , &renderer ,particle_array);

    help_open = true;
    if (fork() == 0) {
        execlp("python3", "python3", "controls.py", "help", NULL);
        _exit(1);
    }

    run(window , renderer, particle_array, shared_count);

    munmap(shared_count, sizeof(int));
    shm_unlink("/particle_count");
    close(shm_fd);
    free(particle_array);
    cleanup(window , renderer);

    return 0;
}
