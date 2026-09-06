NAME    = maze
CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99 -I libtci
LIBS    = libtci/libtci.a libtci/libtciutil.a -lSDL2 -lSDL2_image
SRCS    = main.c map.c player.c camera.c render.c
OBJS    = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
