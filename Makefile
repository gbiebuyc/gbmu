NAME = gbmu
BACKEND = src
SRC_BACKEND = main.cpp Emulator.cpp
OBJ_BACKEND = $(addprefix obj/,$(SRC_BACKEND:.cpp=.o))
TEMPLATE_BACKEND = $(wildcard $(BACKEND)/*.tpp)
CXX = g++ -Wall -Wextra
.PHONY: all clean fclean re

all: back-end $(NAME)

$(NAME): frontend-gtk3/main.cpp src/Emulator.cpp
	g++ -c -o frontend-gtk3/main.o `pkg-config gtk+-3.0 --cflags` frontend-gtk3/main.cpp
	g++ -o $(NAME) frontend-gtk3/main.o obj/Emulator.o `pkg-config gtk+-3.0 --libs`

back-end: $(OBJ_BACKEND)
	$(CXX) -o $@ $^ 
	
obj/%.o: $(BACKEND)/%.cpp $(TEMPLATE_BACKEND)
	@mkdir -p obj
	$(CXX) -c -o $@ $< -Wall -Wextra

clean:
	rm -rf obj
	rm -rf $(OBJ) $(OBJ_BACKEND)

fclean:clean
	rm -rf $(NAME)

re: fclean all
	
