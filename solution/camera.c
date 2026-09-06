#include "camera.h"

/* Which room is a point in? Integer division on the tile index, so the
** answer changes exactly on a room boundary and nowhere else. */
static void room_of(t_player const *p, int *rx, int *ry)
{
    *rx = tile_index((int)p->x + PLAYER_WIDTH / 2) / ROOM_W;
    *ry = tile_index((int)p->y + PLAYER_HEIGHT / 2) / ROOM_H;
}

void    camera_init(t_camera *cam, t_player const *p)
{
    room_of(p, &cam->room_x, &cam->room_y);
}

/*
** Returns 1 on the frame the room changes, 0 otherwise. Nothing in this
** chapter needs that yet -- but it is the hook a transition, a sound, or
** a step counter would hang from, and it costs one comparison.
*/
int camera_update(t_camera *cam, t_player const *p)
{
    int rx;
    int ry;

    room_of(p, &rx, &ry);
    if (rx == cam->room_x && ry == cam->room_y)
        return (0);
    cam->room_x = rx;
    cam->room_y = ry;
    return (1);
}

int camera_origin_x(t_camera const *cam)
{
    return (cam->room_x * ROOM_W * TILE_SIZE);
}

int camera_origin_y(t_camera const *cam)
{
    return (cam->room_y * ROOM_H * TILE_SIZE);
}
