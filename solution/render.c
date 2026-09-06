#include "render.h"

/*
** Exactly one room, every frame. g02a computed a first and last column
** from a scrolling offset because its view could straddle any two
** columns; here the bounds come from the room itself, so the loop is
** both simpler and incapable of showing a neighbouring room by accident.
*/
void    render_room(t_map const *map, t_camera const *cam,
        SDL_Renderer *ren, SDL_Texture *tileset)
{
    int         ox;
    int         oy;
    int         x;
    int         y;
    int         tile;
    SDL_Rect    src;
    SDL_Rect    dst;

    ox = cam->room_x * ROOM_W;
    oy = cam->room_y * ROOM_H;
    y = 0;
    while (y < ROOM_H) {
        x = 0;
        while (x < ROOM_W) {
            tile = map_tile_at(map, ox + x, oy + y);
            if (tile != TILE_EMPTY) {
                src.x = (tile - 1) * TILE_SIZE;
                src.y = 0;
                src.w = TILE_SIZE;
                src.h = TILE_SIZE;
                dst.x = x * TILE_SIZE;
                dst.y = y * TILE_SIZE;
                dst.w = TILE_SIZE;
                dst.h = TILE_SIZE;
                SDL_RenderCopy(ren, tileset, &src, &dst);
            }
            x++;
        }
        y++;
    }
}

void    render_player(t_player const *p, t_camera const *cam,
        SDL_Renderer *ren, SDL_Texture *spritesheet)
{
    SDL_Rect            src;
    SDL_Rect            dst;
    SDL_RendererFlip    flip;

    src.x = player_sheet_frame(p) * PLAYER_WIDTH;
    src.y = 0;
    src.w = PLAYER_WIDTH;
    src.h = PLAYER_HEIGHT;
    dst.x = (int)p->x - camera_origin_x(cam);
    dst.y = (int)p->y - camera_origin_y(cam);
    dst.w = PLAYER_WIDTH;
    dst.h = PLAYER_HEIGHT;
    flip = SDL_FLIP_NONE;
    if (p->facing < 0)
        flip = SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(ren, spritesheet, &src, &dst, 0.0, NULL, flip);
}
