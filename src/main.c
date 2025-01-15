#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <emscripten.h>
#include "textures.h"
#include "gameloop.h"
#include "options.h"
#include "terrain.h"
#include "data.h"
#include "main.h"
#include "gui.h"

#include <emscripten/html5.h>

#define WINDOW_W 800
#define WINDOW_H 600
#define BUFFER_SCALE 1

#define MAX_FPS 60
#define MIN_FRAME_MILLISECONDS (1000 / MAX_FPS)

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

static Inputs inputs = {0};
static const uint8_t *keyboard = NULL;
static int running = 1;

// Declare missing functions
static int controlLoop(Inputs *, const uint8_t *);
static int handleEvent(Inputs *, const uint8_t *, SDL_Event);
int gameLoop(Inputs *inputs, SDL_Renderer *renderer);



void main_loop() {
    uint32_t frameStartTime = SDL_GetTicks();

    //SDL_RenderSetScale(renderer, (float)WINDOW_W / (float) (WINDOW_W/2), (float)WINDOW_H / (float)(WINDOW_H/2));
    int canvas_width, canvas_height;
    
    // Get the current canvas size
    emscripten_get_canvas_element_size("#canvas", &canvas_width, &canvas_height);
    
    // Update the SDL window size
    SDL_SetWindowSize(window, canvas_width, canvas_height);
    printf("%d x %d" , canvas_width, canvas_height);
    // Adjust the renderer scale based on the new window size
    SDL_RenderSetScale(renderer, (float)canvas_width / (float)(WINDOW_W/4), 
                                 (float)canvas_height / (float)(WINDOW_H/4));

    if (!controlLoop(&inputs, keyboard) || !gameLoop(&inputs, renderer)) {
        running = 0; // Exit the loop on failure
        emscripten_cancel_main_loop(); // Stop the loop
    }

    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(SDL_GetWindowFromID(1)); // Get window ID 1 by default

    // Clean up input struct
    inputs.keyTyped = 0;
    inputs.keySym = 0;

    // Limit FPS
    uint32_t frameDuration = SDL_GetTicks() - frameStartTime;
    if (frameDuration < MIN_FRAME_MILLISECONDS) {
        SDL_Delay(MIN_FRAME_MILLISECONDS - frameDuration);
    }
}

int main(int argc, char *argv[]) {
    (void)(argc);
    (void)(argv);

    // SDL Initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("M4KC",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_W, WINDOW_H,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_RenderSetScale(renderer, BUFFER_SCALE, BUFFER_SCALE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    keyboard = SDL_GetKeyboardState(NULL);

    // Initialize modules
    if (data_init() || options_init()) {
        printf("Failed to initialize modules.\n");
        return 1;
    }

    // Generate textures
    genTextures(45390874);

    // Set up the main loop
    emscripten_set_main_loop(main_loop, 0, 1);

    SDL_Quit();
    return 0;
}


static int controlLoop (Inputs *inputs, const Uint8 *keyboard) {
        SDL_PumpEvents();
        int mouseX = 0, mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Detect movement controls
        inputs->keyboard.space = keyboard[SDL_SCANCODE_SPACE];
        inputs->keyboard.w     = keyboard[SDL_SCANCODE_W];
        inputs->keyboard.s     = keyboard[SDL_SCANCODE_S];
        inputs->keyboard.a     = keyboard[SDL_SCANCODE_A];
        inputs->keyboard.d     = keyboard[SDL_SCANCODE_D];

        if (!SDL_GetRelativeMouseMode()) {
                inputs->mouse.x = mouseX;
                inputs->mouse.y = mouseY;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                if (!handleEvent(inputs, keyboard, event)) {
                        return 0;
                }
        }

        return 1;
}

static int handleEvent (Inputs *inputs, const uint8_t *keyboard, SDL_Event event) {
        switch (event.type) {
        case SDL_QUIT:
                return 0;
          
        case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                        inputs->mouse.left = 1;
                        break;
                case SDL_BUTTON_RIGHT:
                        inputs->mouse.right = 1;
                        break;
                }
                break;
          
        case SDL_MOUSEBUTTONUP:
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                        inputs->mouse.left = 0;
                        break;
                case SDL_BUTTON_RIGHT:
                        inputs->mouse.right = 0;
                        break;
                }
                break;

        case SDL_KEYDOWN:
                inputs->keySym = event.key.keysym.sym;
        case SDL_KEYUP:
                if (event.key.repeat == 0) {
                        // Detect UI hotkeys
                        inputs->keyboard.esc = keyboard[SDL_SCANCODE_ESCAPE];
                        inputs->keyboard.f1  = keyboard[SDL_SCANCODE_F1];
                        inputs->keyboard.f2  = keyboard[SDL_SCANCODE_F2];
                        inputs->keyboard.f3  = keyboard[SDL_SCANCODE_F3];
                        inputs->keyboard.f4  = keyboard[SDL_SCANCODE_F4];
                        inputs->keyboard.e   = keyboard[SDL_SCANCODE_E];
                        inputs->keyboard.t   = keyboard[SDL_SCANCODE_T];
                        inputs->keyboard.f   = keyboard[SDL_SCANCODE_F];

                        inputs->keyboard.num0 = keyboard[SDL_SCANCODE_0];
                        inputs->keyboard.num1 = keyboard[SDL_SCANCODE_1];
                        inputs->keyboard.num2 = keyboard[SDL_SCANCODE_2];
                        inputs->keyboard.num3 = keyboard[SDL_SCANCODE_3];
                        inputs->keyboard.num4 = keyboard[SDL_SCANCODE_4];
                        inputs->keyboard.num5 = keyboard[SDL_SCANCODE_5];
                        inputs->keyboard.num6 = keyboard[SDL_SCANCODE_6];
                        inputs->keyboard.num7 = keyboard[SDL_SCANCODE_7];
                        inputs->keyboard.num8 = keyboard[SDL_SCANCODE_8];
                        inputs->keyboard.num9 = keyboard[SDL_SCANCODE_9];
                }
                
                break;

        case SDL_MOUSEWHEEL:
                inputs->mouse.wheel = event.wheel.y;
                break;

        case SDL_MOUSEMOTION:
                if (SDL_GetRelativeMouseMode()) {
                        inputs->mouse.x = event.motion.xrel;
                        inputs->mouse.y = event.motion.yrel;
                }
                break;

        case SDL_TEXTINPUT:
                inputs->keyTyped = event.text.text[0];
                break;
        }

        return 1;
}
