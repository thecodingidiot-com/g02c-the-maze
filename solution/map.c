#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "libtci.h"
#include "libtciutil.h"
#include "map.h"

int tile_index(int pixel)
{
    return (pixel / TILE_SIZE);
}

int map_rooms_x(t_map const *map)
{
    return (map->width / ROOM_W);
}

int map_rooms_y(t_map const *map)
{
    return (map->height / ROOM_H);
}

int map_tile_at(t_map const *map, int tile_x, int tile_y)
{
    if (tile_x < 0 || tile_x >= map->width || tile_y < 0 || tile_y >= map->height)
        return (TILE_WALL);
    return (map->tiles[tile_y * map->width + tile_x]);
}

int map_is_solid(t_map const *map, int tile_x, int tile_y)
{
    return (map_tile_at(map, tile_x, tile_y) == TILE_WALL);
}

static int  parse_dimensions(int fd, int *width, int *height)
{
    char    *line;
    char    *space;

    line = tci_getline(fd);
    if (!line)
        return (0);
    space = tci_strchr(line, ' ');
    if (!space) {
        free(line);
        return (0);
    }
    *space = '\0';
    *width = tci_atoi(line);
    *height = tci_atoi(space + 1);
    free(line);
    return (*width > 0 && *height > 0);
}

static int  tile_from_char(char c)
{
    if (c == '#')
        return (TILE_WALL);
    if (c == 'E')
        return (TILE_EXIT);
    return (TILE_FLOOR);
}

int map_load(t_map *map, char const *path)
{
    int     fd;
    char    *line;
    int     y;
    int     x;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        tci_printf("error: cannot open '%s'\n", path);
        return (0);
    }
    if (!parse_dimensions(fd, &map->width, &map->height)) {
        tci_printf("error: '%s' does not start with '<width> <height>'\n", path);
        close(fd);
        return (0);
    }
    /*
    ** A map that is not a whole number of rooms would leave a strip the
    ** camera can never rest on, so refuse it here rather than render
    ** something subtly wrong later.
    */
    if (map->width % ROOM_W != 0 || map->height % ROOM_H != 0) {
        tci_printf("error: %dx%d is not a whole number of %dx%d rooms\n",
            map->width, map->height, ROOM_W, ROOM_H);
        close(fd);
        return (0);
    }
    map->tiles = tci_calloc(map->width * map->height, sizeof(int));
    if (!map->tiles) {
        close(fd);
        return (0);
    }
    map->start_x = TILE_SIZE;
    map->start_y = TILE_SIZE;
    y = 0;
    while (y < map->height && (line = tci_getline(fd)) != NULL) {
        x = 0;
        while (x < map->width && line[x] && line[x] != '\n') {
            if (line[x] == 'S') {
                map->start_x = x * TILE_SIZE;
                map->start_y = y * TILE_SIZE;
            }
            map->tiles[y * map->width + x] = tile_from_char(line[x]);
            x++;
        }
        free(line);
        y++;
    }
    close(fd);
    return (y == map->height);
}

void    map_free(t_map *map)
{
    free(map->tiles);
    map->tiles = NULL;
}
