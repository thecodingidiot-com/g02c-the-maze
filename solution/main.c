#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include "libtci.h"
#include "camera.h"
#include "map.h"
#include "player.h"
#include "render.h"

int main(int argc, char **argv)
{
    SDL_Window      *win;
    SDL_Renderer    *ren;
    SDL_Texture     *tileset;
    SDL_Texture     *sheet;
    SDL_Event        ev;
    Uint8 const     *keys;
    t_map            map;
    t_player         player;
    t_camera         cam;
    int              running;

    if (argc < 2) {
        tci_printf("usage: %s <maze.txt>\n", argv[0]);
        return (1);
    }
    if (!map_load(&map, argv[1]))
        return (1);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        tci_printf("SDL_Init: %s\n", SDL_GetError());
        map_free(&map);
        return (1);
    }
    IMG_Init(IMG_INIT_PNG);
    win = SDL_CreateWindow("The Maze", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, 0);
    /*
    ** Ask for acceleration, accept software. A machine with no GPU
    ** driver -- a CI runner, a headless box under SDL_VIDEODRIVER=dummy,
    ** a plain VM -- has no accelerated renderer, and SDL answers
    ** "Couldn't find matching render driver" rather than quietly giving
    ** you something. Without this fallback the game is unrunnable
    ** anywhere without a display, which is exactly where a tester runs.
    */
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren)
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    tileset = IMG_LoadTexture(ren, "assets/tileset.png");
    sheet = IMG_LoadTexture(ren, "assets/spritesheet.png");
    if (!win || !ren || !tileset || !sheet) {
        tci_printf("error: failed to create window or load assets — did you run gen_assets.sh?\n");
        map_free(&map);
        return (1);
    }
    player_init(&player, (float)map.start_x, (float)map.start_y);
    camera_init(&cam, &player);
    running = 1;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                running = 0;
            if (ev.type == SDL_KEYDOWN && (ev.key.keysym.sym == SDLK_ESCAPE
                    || ev.key.keysym.sym == SDLK_q))
                running = 0;
        }
        keys = SDL_GetKeyboardState(NULL);
        player_handle_input(&player, keys, &map);
        player_update_animation(&player);
        camera_update(&cam, &player);
        if (map_check_exit(&map, &player) == OUTCOME_ESCAPED) {
            tci_printf("You found the way out.\n");
            running = 0;
        }
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        render_room(&map, &cam, ren, tileset);
        render_player(&player, &cam, ren, sheet);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    SDL_DestroyTexture(sheet);
    SDL_DestroyTexture(tileset);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    map_free(&map);
    return (0);
}
