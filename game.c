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
#include <string.h>
#include "game.h" 
#include "utils.h"

#define NUM_BODIES 12

World world;
Player player;
Level level1;
Object objects[NUM_BODIES];

void initGameObjects() {
	Object square, ground, platform1, platform2, platform3, platform4, platform5, platform6, platform7;
	Object platform8, platform9, platform10;

	level1.levelWidth = WIDTH * 3;
	level1.levelHeight = HEIGHT * 2;
 	level1.cameraLeftOffset = (float)WIDTH / 2;
	level1.cameraRightOffset = level1.levelWidth - level1.cameraLeftOffset;
	level1.cameraBottomOffset = (float)HEIGHT / 2;
	level1.cameraTopOffset = level1.levelHeight - level1.cameraBottomOffset;

	// Units are in pixels
	square.x = 100;
	square.y = 100 + HEIGHT;
	square.w = 50;
	square.h = 50;
	square.type = DYNAMIC;
	square.color = (Color){255, 0, 0, 255};

	ground.w = level1.levelWidth; 
	ground.h = 20;
	ground.x = 0;
	ground.y = HEIGHT - ground.h + HEIGHT;
	ground.type = STATIC;
	ground.color = (Color){0, 255, 0, 255};

	platform1.w = 200;
	platform1.h = 20;
	platform1.x = 200;
	platform1.y = 300 + HEIGHT;
	platform1.type = STATIC;
	platform1.color = (Color){0, 255, 255, 255};

	platform2.w = 100;
	platform2.h = 20;
	platform2.x = 40;
	platform2.y = 400 + HEIGHT;
	platform2.type = STATIC;
	platform2.color = (Color){0, 0, 255, 255};

	platform3.w = 200;
	platform3.h = 40;
	platform3.x = 400;
	platform3.y = 100 + HEIGHT;
	platform3.type = STATIC;
	platform3.color = (Color){0, 0, 255, 255};

	platform4.w = 50;
	platform4.h = 50;
	platform4.x = 400;
	platform4.y = 50 + HEIGHT;
	platform4.type = DYNAMIC;
	platform4.color = (Color){255, 20, 255, 255};

	platform5.w = 150;
	platform5.h = 20;
	platform5.x = 175;
	platform5.y = 190 + HEIGHT;
	platform5.type = STATIC;
	platform5.color = (Color){125, 255, 30, 255};

	platform6.w = 20;
	platform6.h = level1.levelHeight;
	platform6.x = level1.levelWidth - platform6.w;
	platform6.y = 0;
	platform6.type = STATIC;
	platform6.color = (Color){20, 255, 100, 255};

	platform7.w = 20;
	platform7.h = level1.levelHeight;
	platform7.x = 0; 
	platform7.y = 0;
	platform7.type = STATIC;
	platform7.color = (Color){20, 255, 100, 255};

	platform8.w = 200;
	platform8.h = 50;
	platform8.x = WIDTH + 200;
	platform8.y = 400 + HEIGHT;
	platform8.type = STATIC;
	platform8.color = (Color){40, 80, 90, 255};

	platform9.w = 100;
	platform9.h = 10;
	platform9.x = 2 * WIDTH + 200;
	platform9.y = 450 + HEIGHT;
	platform9.type = STATIC;
	platform9.color = (Color){40, 80, 90, 255};

	platform10.w = 40;
	platform10.h = 40;
	platform10.x = 2 * WIDTH + 400;
	platform10.y = 350 + HEIGHT;
	platform10.type = STATIC;
	platform10.color = (Color){40, 80, 90, 255};

	player.canJump = false;
	player.maxVelocityX = 10.0f;
	player.jumpBuffer = 0;
	player.bufferFrames = 10;
	player.xForce = 2.0f;
	player.yForce = 3.0f;

	objects[0] = square;
	objects[1] = ground;
	objects[2] = platform1;
	objects[3] = platform2;
	objects[4] = platform3;
	objects[5] = platform4;
	objects[6] = platform5;
	objects[7] = platform6;
	objects[8] = platform7;
	objects[9] = platform8;
	objects[10] = platform9;
	objects[11] = platform10;
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
		objects[i].rect.x = objects[i].x;
		objects[i].rect.y = objects[i].y;
		objects[i].rect.w = objects[i].w;
		objects[i].rect.h = objects[i].h;
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

		b2Vec2 pos = SDLPositionToBox2D(&objects[i]);
		bodyDef.position = pos; 

		if (objects[i].type == DYNAMIC){
			bodyDef.type = b2_dynamicBody;
		}

		// Create Body
		objects[i].bodyId = b2CreateBody(world.worldId, &bodyDef);
		b2Vec2 size = SDLSizeToBox2D(&objects[i]);
		b2MassData mass;
		mass.mass = 15.0f;
		b2Body_SetMassData(objects[i].bodyId, mass); 

		objects[i].polygon = b2MakeBox(size.x, size.y);

		// Create Polygon Shape
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.friction = 0.5f;
		shapeDef.userData = "shapeDef";

		// Add square shape to polygon
		b2CreatePolygonShape(objects[i].bodyId, &shapeDef, &objects[i].polygon);

		// If the object is static, add a ground and 2 wall sensors to detect
		// what side of the object we have hit
		if (objects[i].type == STATIC) {
			b2ShapeDef ground = b2DefaultShapeDef();
			b2ShapeDef lwall = b2DefaultShapeDef();
			b2ShapeDef rwall = b2DefaultShapeDef();

			ground.userData = "ground";
			lwall.userData = "wall";
			rwall.userData = "wall";

			ground.isSensor = lwall.isSensor = rwall.isSensor = true;

			// This creates a shape that is offset from the center of the main body
			// That "1" in the b2Rot took me like 2 hours to figure out :(
			b2Polygon groundPol = b2MakeOffsetBox(size.x * .95, size.y * 0.1, (b2Vec2){0, size.y * .9}, (b2Rot){1, 0});
			b2Polygon lwallPol = b2MakeOffsetBox(size.x * 0.1, size.y * 0.95, (b2Vec2){-size.x * 0.9, 0}, (b2Rot){1, 0});
			b2Polygon rwallPol = b2MakeOffsetBox(size.x * 0.1, size.y * 0.1, (b2Vec2){size.x + 0.9, 0}, (b2Rot){1, 0});

			b2CreatePolygonShape(objects[i].bodyId, &ground, &groundPol);
			b2CreatePolygonShape(objects[i].bodyId, &lwall, &lwallPol);
			b2CreatePolygonShape(objects[i].bodyId, &rwall, &rwallPol);
		}
	}
}

int gameLoop() {
	const Uint64 current = SDL_GetTicks();
	const Uint64 elapsed = current - world.lastTime;
	SDL_PumpEvents();
	SDL_Event e;

	// Poll for Events
	while (SDL_PollEvent(&e) != 0) {
		// Quit game
		if (e.type == SDL_EVENT_QUIT) return -1;
	}

	// Get player velocity
	const b2BodyId playerId = objects[0].bodyId; 
	b2Vec2 velocity = b2Body_GetLinearVelocity(playerId);
	double speed = elapsed;

	// Check if an arrow key is being pressed, if so, apply force in that direction
	// Jump if we can 
	if (world.keys[SDL_SCANCODE_UP] && (player.canJump || player.jumpBuffer > 0)) {
		player.canJump = false;
		if (player.jumpBuffer > 0) player.jumpBuffer--;

		b2Vec2 force = {0, player.yForce * speed};
		b2Body_ApplyForceToCenter(playerId, force, true);
	} 

	// Move left, up to a certain speed
	if (world.keys[SDL_SCANCODE_LEFT] && velocity.x >= -player.maxVelocityX) {
		b2Vec2 force = {-player.xForce * speed, 0};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}

	// Move right, up to a certain speed
	if (world.keys[SDL_SCANCODE_RIGHT] && velocity.x <= player.maxVelocityX) {
		b2Vec2 force = {player.xForce * speed, 0};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}

	// Get the sensor events in the world, i.e., if we hit a ground or wall sensor
	b2SensorEvents sensorEvents = b2World_GetSensorEvents(world.worldId);
	
	// If we are touching the ground, set that we can jump
	for (int i = 0; i < sensorEvents.beginCount; i++) {
		b2SensorBeginTouchEvent* beginTouch = sensorEvents.beginEvents + i;
		char* sensor = b2Shape_GetUserData(beginTouch->sensorShapeId);

		if (strcmp(sensor, "ground") == 0) {
			player.canJump = true;
			player.jumpBuffer = player.bufferFrames;
		}
	}

	// If we leave the ground, set that we can't jump
	for (int i = 0; i < sensorEvents.endCount; i++) {
		b2SensorEndTouchEvent* endTouch = sensorEvents.endEvents + i;
		char* sensor = b2Shape_GetUserData(endTouch->sensorShapeId);

		if (strcmp(sensor, "ground") == 0 && player.canJump) {
			player.canJump = false;
			player.jumpBuffer = 0;
		}
	}

	// Step physics simulation
	b2World_Step(world.worldId, 1.0f / 60.0f, 8);

	// Render background
	SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(world.renderer);

	// Get Camera Offset
	b2Vec2 playerPosition = b2Body_GetPosition(playerId);
	b2Vec2 position = box2DToSDL(playerPosition, &objects[0]);

	// Get the x and y offset from the center of the first screen
	world.xoffset = (float)WIDTH / 2 - position.x;
	world.yoffset = (float)HEIGHT / 2 - position.y;

	if (position.x <= level1.cameraLeftOffset) world.xoffset = 0;
	if (position.x >= level1.cameraRightOffset) world.xoffset = WIDTH - level1.levelWidth;
	if (position.y <= level1.cameraBottomOffset) world.yoffset = 0;
	if (position.y >= level1.cameraTopOffset) world.yoffset = HEIGHT - level1.levelHeight;

	// Draw Static bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		// Update position
		Color color = objects[i].color;

		// Get the Box2D object's position as SDL, add offsets to it
		b2Vec2 position = box2DToSDL(b2Body_GetPosition(objects[i].bodyId), &objects[i]); 
		objects[i].rect.x = position.x + world.xoffset;
		objects[i].rect.y = position.y + world.yoffset;

		// Render object will color
		SDL_SetRenderDrawColor(world.renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(world.renderer, &objects[i].rect);
	}

	// Display Screen
	SDL_RenderPresent(world.renderer);

	// Wait for frame
	const Uint64 elapsedTime = SDL_GetTicks() - current;
	if (elapsedTime < MS_PER_SECOND) {
		SDL_Delay(MS_PER_SECOND - elapsedTime);
	}
	world.lastTime = current;

	// Means continue game loop
	return 0;
}

void kill() {
	// Clean up SDL
	SDL_DestroyRenderer(world.renderer);
	SDL_DestroyWindow(world.window);
	b2DestroyWorld(world.worldId);
	SDL_Quit();
}
