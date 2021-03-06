NAME = gbmu
FRONTEND = frontend-gtk3
CORE = ./gearboy_libretro.so
SRC = $(wildcard $(FRONTEND)/*.c)
OBJ = $(SRC:.c=.o)
BACKEND = src
SRC_BACKEND = main.cpp Emulator.cpp
OBJ_BACKEND = $(addprefix obj/,$(SRC_BACKEND:.cpp=.o))
TEMPLATE_BACKEND = $(wildcard $(BACKEND)/*.tpp)
CXX = g++ -Wall -Wextra
.PHONY: all clean fclean re

all: back-end #$(NAME)

$(NAME): $(OBJ)
	cc -o $@ $^ $(CORE) `pkg-config gtk+-3.0 --libs`

back-end: $(OBJ_BACKEND)
	$(CXX) -o $@ $^ 

%.o: %.c
	cc -c -o $@ $< `pkg-config gtk+-3.0 --cflags` -g
	
obj/%.o: $(BACKEND)/%.cpp $(TEMPLATE_BACKEND)
	@mkdir -p obj
	$(CXX) -c -o $@ $< -Wall -Wextra

clean:
	rm -rf obj
	rm -rf $(OBJ) $(OBJ_BACKEND)

fclean:clean
	rm -rf $(NAME)

re: fclean all
	
