#include "ui.h"

void window_renderer_init(window_struct window_info, SDL_Window **window, SDL_Renderer **renderer) {
    //SDL_SetHint("SDL_VIDEODRIVER", "x11");
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
}


void cleanup(SDL_Window *inp_window, SDL_Renderer *inp_renderer){

    SDL_DestroyRenderer(inp_renderer);
    SDL_DestroyWindow(inp_window);
    SDL_Quit();

}