#include <SDL3/SDL_render.h>
#include "render.h"

void renderRectangle(SDL_Renderer *renderer, Object* object) {
	const Color c = object->color;

	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
	SDL_RenderFillRect(renderer, &object->rect);
}

// https://stackoverflow.com/questions/65723827/sdl2-function-to-draw-a-filled-circle
void renderCircle(SDL_Renderer *renderer, Object *object) {
	const int radius = object->p.w / 2;
	const int centerX = object->rect.x + radius;
	const int centerY = object->rect.y + radius;
	const Color c = object->color; 

	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	for (int y = -radius; y <= radius; y++) {
		for (int x = -radius; x <= radius; x++) {
			if ((x * x + y * y) <= radius * radius) {
				SDL_RenderPoint(renderer, centerX + x, centerY + y);
			}
		}
	}
}
