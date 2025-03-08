game: game.c
	gcc game.c -I/usr/local/include/box2d -L/usr/local/lib -lSDL3 -lbox2d -lm -g -o game 
