#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <box2d/box2d.h>

// Initalize the custom objects to manage Box2D and SDL properties
void initGameObjects(void);

// Initalizes the SDL libraries and related elementes
void initSDL(void);

// Initialized the Box2D library, and creates related elements
void initBox2D(void);

// Main game loop: handles inputs, calculates, and renders
bool gameLoop(void);

// Cleans up
void kill(void);

const static int WIDTH = 1000;
const static int HEIGHT = 500;
const static float MS_PER_SECOND = 16.67; 
const static float PIXELS_PER_METER = 50.0f;

typedef enum ObjectType {
	STATIC,
	DYNAMIC
} ObjectType;

typedef struct World {
	const bool *keys;
	b2WorldId worldId;
	SDL_Window *window;
	SDL_Renderer *renderer;
	Uint64 lastTime;
	float xoffset;
} World;

typedef struct Color {
	int r;
	int g;
	int b;
	int a;
} Color;

typedef struct Object {
	float x;
	float y;
	float w;
	float h;
	SDL_FRect rect;
	b2BodyId bodyId;
	ObjectType type;
	b2Polygon polygon;
	Color color;
} Object;

typedef struct Player {
	bool canJump;
	bool canWallJump;
	int bufferFrames;
	int jumpBuffer;
	float maxVelocityX;
	float xForce;
	float yForce; 
} Player;

