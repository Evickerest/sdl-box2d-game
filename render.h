#pragma once

#include "game.h"
#include <SDL3/SDL_render.h>
#include <box2d/math_functions.h>

// Renders a rectangle on the screen
void renderRectangle(SDL_Renderer* renderer, Object* object); 

// Renders a circle on the screen
void renderCircle(SDL_Renderer* renderer, Object* object); 
