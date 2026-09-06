#ifndef MAP_H
# define MAP_H

# define TILE_SIZE      32

/*
** A room is exactly one screen. That is the whole idea of this game, so
** it is a definition rather than a coincidence: 25 x 19 tiles at 32
** pixels is 800 x 608, and the window is 800 x 608. The camera never
** shows part of two rooms at once, which is what makes a map you are
** never given worth building in your head.
*/
# define ROOM_W         25
# define ROOM_H         19
# define WINDOW_W       (ROOM_W * TILE_SIZE)
# define WINDOW_H       (ROOM_H * TILE_SIZE)

# define TILE_EMPTY     0
# define TILE_FLOOR     1
# define TILE_WALL      2
# define TILE_EXIT      3

typedef struct s_map
{
    int     width;
    int     height;
    int     *tiles;
    int     start_x;
    int     start_y;
}   t_map;

int     map_load(t_map *map, char const *path);
void    map_free(t_map *map);
int     map_tile_at(t_map const *map, int tile_x, int tile_y);
int     map_is_solid(t_map const *map, int tile_x, int tile_y);
int     tile_index(int pixel);

/* How many rooms the map is across and down. The loader rejects a map
** that is not a whole number of rooms in both directions -- half a room
** at the edge would be a screen the camera could never sit on. */
int     map_rooms_x(t_map const *map);
int     map_rooms_y(t_map const *map);

#endif
