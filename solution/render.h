#ifndef RENDER_H
# define RENDER_H

# include <SDL2/SDL.h>
# include "camera.h"
# include "map.h"
# include "player.h"

void    render_room(t_map const *map, t_camera const *cam,
            SDL_Renderer *ren, SDL_Texture *tileset);
void    render_player(t_player const *p, t_camera const *cam,
            SDL_Renderer *ren, SDL_Texture *spritesheet);

#endif
