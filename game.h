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
// Returns 1 if active, 0 or -1 if not
int gameLoop(void);

// Cleans up
void cleanup(void);

const static int WIDTH = 1000;
const static int HEIGHT = 500;
const static float MS_PER_SECOND = 16.67; 
const static float PIXELS_PER_METER = 50.0f;

// Object type for Box2D and Others
typedef enum ObjectType {
	STATIC,
	DYNAMIC
} ObjectType;

// Defines information relating to a Game level
typedef struct Level {
	float levelWidth;
	float levelHeight;
	float cameraLeftOffset;
	float cameraRightOffset;
	float cameraTopOffset;
	float cameraBottomOffset;
	int levelStatus;
} Level;

// Defines information related to the world, with some globals
typedef struct World {
	const bool *keys;
	b2WorldId worldId;
	SDL_Window *window;
	SDL_Renderer *renderer;
	Uint64 lastTime;
	float xoffset;
	float yoffset;
} World;

// Color struct for rendering objects in SDL
typedef struct Color {
	int r;
	int g;
	int b;
	int a;
} Color;

// Defines posistion and size information
typedef struct Position {
	float x;
	float y;
	float w;
	float h;
} p;

// Defines some generic rectangle object in the world, 
// Keeps information for Box2D and SDL
typedef struct Object {
	p p;
	SDL_FRect rect;
	b2BodyId bodyId;
	ObjectType type;
	b2Polygon polygon;
	Color color;
} Object;

// Defines player information and velocity constraints
typedef struct Player {
	bool canJump;
	bool canWallJump;
	int bufferFrames;
	int jumpBuffer;
	float maxVelocityX;
	float xForce;
	float yForce; 
	b2Vec2 desiredVelocity;
} Player;

