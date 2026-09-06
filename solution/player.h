#ifndef PLAYER_H
# define PLAYER_H

# include <SDL2/SDL.h>
# include "map.h"

/*
** Narrower and shorter than g02a's platformer sprite, and deliberately
** smaller than a tile: a doorway is three tiles wide but the gaps
** between interior walls are often one, and a 32-pixel body in a
** 32-pixel gap catches on rounding for the rest of the chapter.
*/
# define PLAYER_WIDTH    24
# define PLAYER_HEIGHT   24
# define MOVE_SPEED      3.0f

typedef enum e_outcome
{
    OUTCOME_NONE,
    OUTCOME_ESCAPED
}   t_outcome;

typedef struct s_player
{
    float   x;
    float   y;
    int     facing;
    int     moving;
    int     frame;
    int     frame_timer;
}   t_player;

void        player_init(t_player *p, float x, float y);
void        player_handle_input(t_player *p, Uint8 const *keys, t_map const *map);
void        player_update_animation(t_player *p);
int         player_sheet_frame(t_player const *p);
t_outcome   map_check_exit(t_map const *map, t_player const *p);

#endif
