#include "player.h"

void    player_init(t_player *p, float x, float y)
{
    p->x = x;
    p->y = y;
    p->facing = 1;
    p->moving = 0;
    p->frame = 0;
    p->frame_timer = 0;
}

/*
** Does the player's box overlap any solid tile at this position? The box
** can straddle two tiles on each axis, so both edges are tested -- a
** single centre check walks through corners.
*/
static int  blocked(t_map const *map, float px, float py)
{
    int left;
    int right;
    int top;
    int bottom;

    left = tile_index((int)px);
    right = tile_index((int)px + PLAYER_WIDTH - 1);
    top = tile_index((int)py);
    bottom = tile_index((int)py + PLAYER_HEIGHT - 1);
    return (map_is_solid(map, left, top) || map_is_solid(map, right, top)
        || map_is_solid(map, left, bottom) || map_is_solid(map, right, bottom));
}

/*
** The two axes are resolved separately, and that is not an
** implementation detail -- it is what lets the player slide along a wall
** instead of stopping dead against it. Move in x; if that lands in a
** wall, undo only x. Then the same for y. Pressing into a corner still
** leaves whichever component is free.
**
** There is no gravity here and no jump. g02a's player fell every frame
** and could only push left or right; this one is pushed by input on
** both axes and by nothing else. Removing a force turns out to be a
** smaller change than adding one.
*/
void    player_handle_input(t_player *p, Uint8 const *keys, t_map const *map)
{
    float   dx;
    float   dy;

    dx = 0.0f;
    dy = 0.0f;
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_H])
        dx -= MOVE_SPEED;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_L])
        dx += MOVE_SPEED;
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_K])
        dy -= MOVE_SPEED;
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_J])
        dy += MOVE_SPEED;
    p->moving = (dx != 0.0f || dy != 0.0f);
    if (dx < 0.0f)
        p->facing = -1;
    else if (dx > 0.0f)
        p->facing = 1;
    p->x += dx;
    if (blocked(map, p->x, p->y))
        p->x -= dx;
    p->y += dy;
    if (blocked(map, p->x, p->y))
        p->y -= dy;
}

void    player_update_animation(t_player *p)
{
    if (!p->moving) {
        p->frame = 0;
        p->frame_timer = 0;
        return ;
    }
    p->frame_timer++;
    if (p->frame_timer >= 8) {
        p->frame_timer = 0;
        p->frame = 1 + ((p->frame + 1) % 2);
    }
}

int player_sheet_frame(t_player const *p)
{
    return (p->frame);
}

t_outcome   map_check_exit(t_map const *map, t_player const *p)
{
    int cx;
    int cy;

    cx = tile_index((int)p->x + PLAYER_WIDTH / 2);
    cy = tile_index((int)p->y + PLAYER_HEIGHT / 2);
    if (map_tile_at(map, cx, cy) == TILE_EXIT)
        return (OUTCOME_ESCAPED);
    return (OUTCOME_NONE);
}
