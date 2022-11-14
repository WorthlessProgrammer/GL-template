NAME = main
CC = gcc
FLAGS = -Wall -Wextra
LINK_UBU = /usr/lib64/libGLEW.a ~/.local/lib/libglfw3.a -ldl -lGL -lm -lX11 -lpthread 
LINK_UNIX = -lGLEW -lglfw -ldl -lGL -lm -lX11 -lpthread 

$(NAME):
	$(CC) $(FLAGS) main.c $(LINK_UNIX) -o main
clear:
	rm main
re: clear $(NAME)
