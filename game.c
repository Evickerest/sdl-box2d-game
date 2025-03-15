#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>
#include <box2d/box2d.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h" 
#include "utils.h"

#define NUM_BODIES 7

World world;
Player player;
Object staticObjects[NUM_BODIES];

void initGameObjects() {
	Object square, ground, platform1, platform2, platform3, platform4, platform5;

	// Units are in pixels
	square.x = 100;
	square.y = 100;
	square.w = 50;
	square.h = 50;
	square.type = DYNAMIC;
	square.color = (Color){255, 0, 0, 255};

	ground.w = WIDTH;
	ground.h = 20;
	ground.x = 0;
	ground.y = HEIGHT - ground.h;
	ground.type = STATIC;
	ground.color = (Color){0, 255, 0, 255};

	platform1.w = 200;
	platform1.h = 20;
	platform1.x = 200;
	platform1.y = 300;
	platform1.type = STATIC;
	platform1.color = (Color){0, 255, 255, 255};

	platform2.w = 100;
	platform2.h = 20;
	platform2.x = 20;
	platform2.y = 400;
	platform2.type = STATIC;
	platform2.color = (Color){0, 0, 255, 255};

	platform3.w = 200;
	platform3.h = 40;
	platform3.x = 400;
	platform3.y = 100;
	platform3.type = STATIC;
	platform3.color = (Color){0, 0, 255, 255};

	platform4.w = 50;
	platform4.h = 50;
	platform4.x = 400;
	platform4.y = 50;
	platform4.type = DYNAMIC;
	platform4.color = (Color){255, 20, 255, 255};

	platform5.w = 150;
	platform5.h = 20;
	platform5.x = 175;
	platform5.y = 190;
	platform5.type = STATIC;
	platform5.color = (Color){125, 255, 30, 255};

	player.isJumping = true;

	staticObjects[0] = square;
	staticObjects[1] = ground;
	staticObjects[2] = platform1;
	staticObjects[3] = platform2;
	staticObjects[4] = platform3;
	staticObjects[5] = platform4;
	staticObjects[6] = platform5;
}

void initSDL() {
	// Initalize the SDL library
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    if (!SDL_CreateWindowAndRenderer("Game", WIDTH, HEIGHT, 0, &world.window, &world.renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        exit(1);
    }

	// Set the defined pixel units in initGameObjects() to SDL Frect 
	for (int i = 0; i < NUM_BODIES; i++) {
		staticObjects[i].rect.x = staticObjects[i].x;
		staticObjects[i].rect.y = staticObjects[i].y;
		staticObjects[i].rect.w = staticObjects[i].w;
		staticObjects[i].rect.h = staticObjects[i].h;
	}

	// Gets a pointer to an array that defines what keys are being pressed
	world.keys = SDL_GetKeyboardState(NULL);
	world.lastTime = SDL_GetTicks();
}

void initBox2D() {
	// Create Box2d World
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = (b2Vec2){0.0f, -10.0f};
	world.worldId = b2CreateWorld(&worldDef);

	// Create Static bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		// Create Body definition
		b2BodyDef bodyDef = b2DefaultBodyDef();
		b2Vec2 pos = SDLPositionToBox2D(&staticObjects[i]);
		bodyDef.position = pos; 

		if (staticObjects[i].type == DYNAMIC) bodyDef.type = b2_dynamicBody;

		// Create Body
		staticObjects[i].bodyId = b2CreateBody(world.worldId, &bodyDef);
		b2Vec2 size = SDLSizeToBox2D(&staticObjects[i]);

		staticObjects[i].polygon = b2MakeBox(size.x, size.y);

		// Create Polygon Shape
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.friction = 0.1f;
		shapeDef.userData = "shapeDef";

		b2CreatePolygonShape(staticObjects[i].bodyId, &shapeDef, &staticObjects[i].polygon);

		if (staticObjects[i].type == STATIC) {
			// Create 4 zero mass edges around body
			b2ShapeDef edge = b2DefaultShapeDef();
			edge.userData = "edge";
			edge.isSensor = true;

			b2Polygon edgePolygon = b2MakeOffsetBox(size.x, size.y * 0.1, (b2Vec2){0, size.y}, (b2Rot){1, 0});
			b2CreatePolygonShape(staticObjects[i].bodyId, &edge, &edgePolygon);
		}
	}
int isRunning = 1;
}

bool gameLoop() {
	const Uint64 current = SDL_GetTicks();
	const Uint64 elapsed = current - world.lastTime;
	SDL_Event e;
	SDL_PumpEvents();

	// Poll for Events
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_EVENT_QUIT) {
			return false;
		}
	}

	const b2BodyId playerId = staticObjects[0].bodyId; 

	// Check if an arrow key is being pressed, if so, apply force in that direction
	double speed = 5 * elapsed;
	if (world.keys[SDL_SCANCODE_UP] && player.isJumping == false) {
		player.isJumping = true;
		b2Vec2 force = {0, speed * 15};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_LEFT]) {
		b2Vec2 force = {-speed, 0};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_RIGHT]) {
		b2Vec2 force = {speed, 0};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}

	b2SensorEvents sensorEvents = b2World_GetSensorEvents(world.worldId);
	
	for (int i = 0; i < sensorEvents.beginCount; i++) {
		b2SensorBeginTouchEvent* beginTouch = sensorEvents.beginEvents + i;
		char* sensor = b2Shape_GetUserData(beginTouch->sensorShapeId);
		player.isJumping = false;
	}

	// Step physics simulation
	b2World_Step(world.worldId, 1.0f / 60.0f, 8);

	// Render background
	SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(world.renderer);

	// Draw Static bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		// Update position
		Color color = staticObjects[i].color;

		if (staticObjects[i].type == DYNAMIC) {
			b2Vec2 position = box2DToSDL(b2Body_GetPosition(staticObjects[i].bodyId), &staticObjects[i]);
			staticObjects[i].rect.x = position.x;
			staticObjects[i].rect.y = position.y;
		}

		SDL_SetRenderDrawColor(world.renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(world.renderer, &staticObjects[i].rect);
	}

	// Display Screen
	SDL_RenderPresent(world.renderer);

	// Wait for frame
	const Uint64 elapsedTime = SDL_GetTicks() - current;
	if (elapsedTime < MS_PER_SECOND) {
		SDL_Delay(MS_PER_SECOND - elapsedTime);
	}
	world.lastTime = current;

	return true;
}

void kill() {
	// Clean up SDL
	SDL_DestroyRenderer(world.renderer);
	SDL_DestroyWindow(world.window);
	SDL_Quit();
}
