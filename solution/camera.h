#ifndef CAMERA_H
# define CAMERA_H

# include "map.h"
# include "player.h"

/*
** g02a's camera held a pixel offset and chased the player every frame.
** This one holds a room, and the difference is the entire game.
**
** A camera that follows shows you where you came from: the wall you just
** passed is still on screen, sliding away, and your sense of place is
** maintained for you by the renderer. A camera that snaps to a room
** shows you one screen and nothing else. Cross the threshold and the
** previous room is gone -- not scrolled away, gone -- and the only place
** the connection between the two still exists is in your head.
**
** That is why this is a maze rather than a big level.
*/
typedef struct s_camera
{
    int room_x;
    int room_y;
}   t_camera;

void    camera_init(t_camera *cam, t_player const *p);
int     camera_update(t_camera *cam, t_player const *p);
int     camera_origin_x(t_camera const *cam);
int     camera_origin_y(t_camera const *cam);

#endif
