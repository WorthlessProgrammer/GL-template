NAME = main
CC = gcc
FLAGS = -Wall -Wextra
LINK = /usr/lib64/libGLEW.a ~/.local/lib/libglfw3.a -ldl -lGL -lm -lX11 -lpthread 

#~/glfw-3.3.8/src/libglfw3.a

$(NAME):
	$(CC) $(FLAGS) main.c $(LINK) -o main
clear:
	rm main
re: clear $(NAME)
