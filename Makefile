game: game.c main.c utils.c game.h utils.h
	gcc main.c game.c utils.c -I/usr/local/include/box2d -L/usr/local/lib -lSDL3 -lbox2d -lm -g -o game
