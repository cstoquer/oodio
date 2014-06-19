#include "AmsFont.h"
#include "constants.h"

const int MAX_COLUMN = SCREEN_W / 24;
const int MAX_LINE   = SCREEN_H / 24;

AmsFont::AmsFont(char* fileName) {
	font = SDL_LoadBMP(fileName);
	SDL_Surface* surface = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 32, 0, 0, 0, 0);
	ctx = new RenderingContext(surface);
	_x = 0;
	_y = 0;
}

AmsFont::~AmsFont() {
	delete ctx;
}

SDL_Surface* AmsFont::get() {
	SDL_SetColorKey(ctx->getContext(), SDL_SRCCOLORKEY, SDL_MapRGB(ctx->getContext()->format, 0, 0, 0));
	return ctx->getContext();
};

void AmsFont::locate(int x, int y) {
	_x = x;
	_y = y;
}

void AmsFont::print(char* text) {
	while (*text) {
		int c = *text;
		int posX = (c % 16) * 24;
		int posY = (c / 16) * 24;

		// TODO
		SDL_SetColorKey(font, SDL_SRCCOLORKEY, SDL_MapRGB(font->format, 0, 0, 0));
		ctx->drawImage(font, posX, posY, 24, 24, _x * 24, _y * 24);

		text++;
		_x++;
		if (_x >= MAX_COLUMN) {
			_x = 0;
			_y++;
		}
	}
}

void AmsFont::chr(int) {
	// TODO
}

void AmsFont::scroll(int) {
	// TODO
}
