NAME = gbmu
FRONTEND = frontend-gtk3
CORE = ./gearboy_libretro.so
SRC = $(wildcard $(FRONTEND)/*.c)
OBJ = $(SRC:.c=.o)
.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJ)
	cc -o $@ $^ $(CORE) `pkg-config gtk+-3.0 --libs`

%.o: %.c
	cc -c -o $@ $< `pkg-config gtk+-3.0 --cflags` -g

clean:
	rm -rf $(OBJ)

fclean:
	rm -rf $(OBJ) $(NAME)

re: fclean all
	
