#ifndef UI_H
#define UI_H
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct{
    const char *title;
    int x_pos, y_pos;
    int width ;
    int height;
    Uint32 flags;
}window_struct;

void window_renderer_init(window_struct window_info, SDL_Window **window, SDL_Renderer **renderer);
void cleanup(SDL_Window *inp_window, SDL_Renderer *inp_renderer);

#endif //UI_H
