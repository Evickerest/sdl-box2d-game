#pragma once 
#include <box2d/math_functions.h>
#include "game.h"

// Convert SDL Pixel unit to Box 2d Meter Unit
float pixelToMeter(const float value);

// Convert box2d meter unit to SDL pixel unit
float meterToPixel(const float value);

// Converts Box2d posistion to SDL Object position
b2Vec2 box2DToSDL(b2Vec2 vector, Object *object);

// Converts SDL Object position to Box2d Position
b2Vec2 SDLToBox2D(b2Vec2 vector, Object *object);

// Converts Box2D x,y to SDL x,y
b2Vec2 Box2DXYToSDL(float x, float y);

// Converts SDL x,y to Box2D x,y
b2Vec2 SDLXYToBox2D(float x, float y);

// Convert SDL x,y position to Box2D x,y position
b2Vec2 SDLPositionToBox2D(Object *object);

// Convert Box2D x,y position to SDL x,y position
b2Vec2 SDLSizeToBox2D(Object *object);
