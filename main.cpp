#include <stdint.h>
#ifdef __cplusplus
	#include <cstdlib>
#else
	#include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#define round(x) ((x)>=0?(int16_t)((x)+0.5):(int16_t)((x)-0.5))

#include "constants.h"
#include "RenderingContext.h"
#include "AmsFont.h"
#include "Button.h"
#include "Slider.h"
#include "src/oscillator/Oscillator.h"
#include "src/oscillator/OscRamp.h"
#include "src/oscillator/OscPulse.h"
#include "src/oscillator/OscTri.h"
#include "src/filter/FastFilter.h"
#include "src/filter/Analog4Poles.h"
#include "src/filter/RCFilter.h"
#include "src/sequencer/FreqSeq.h"

bool     filterActive = true;
int16_t  mute = 1;
double   amp = 0.1;

OscPulse     osc1;
OscTri       osc2;
OscTri       osc3;
FastFilter   glide;
RCFilter     fltr;
FreqSeq      seq;

double map(double value, double iMin, double iMax, double oMin, double oMax) {
	return oMin + (oMax - oMin) * (value - iMin) / (iMax - iMin);
}


void audioCallback(void* udata, uint8_t* stream0, int len) {

	int16_t* stream = (int16_t*) stream0;

	for (len >>= 1; len; len--) {
		// update sequencer
		double f = seq.tic();
		f = glide.tic(f);
		osc1.freq = f;
		osc2.freq = f / 3.01;

		// update oscillators
		double o1 = osc1.tic();
		double o2 = osc2.tic();
		double o3 = osc3.tic();

		// o3 = (1 - o3) / 2;
		o3 = map(o3, -1, 1, 0, 0.5);
		osc1.width = o3;
		osc2.width = o3;

		// simple mix + amplification
		double o = (o1 + o2);

		// apply filter
		// if (filterActive) o = o - fltr.tic(o);
		if (filterActive) o = fltr.tic(o);
		o *= amp;

		// trim overload
		if (o < -1) o = -1;
		if (o >  1) o =  1;

		// convert to output format
		o = 0x8000 * o * mute;

		// write in buffer
		*stream++ = round(o);
	}
}

void playStop() {
	mute = mute == 0 ? 1 : 0;
}

void activateFilter() {
	filterActive = !filterActive;
}

void cutoff(double value) {
	// fltr.freq = value;
	fltr.cutoff = value;
}

void resonance(double value) {
	fltr.reso = value;
}

void changeLfo(double value) {
	osc3.freq = value;
}


int main(int argc, char* argv[]) {
	// initialize SDL video and audio
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	// init audio
	{
		SDL_AudioSpec audioSpec;
		audioSpec.freq     = SAMPLE_RATE;
		audioSpec.format   = AUDIO_S16;
		audioSpec.channels = 1;
		audioSpec.samples  = 512;
		audioSpec.callback = audioCallback;
		SDL_OpenAudio(&audioSpec, NULL);
	}

	osc3.freq  = 0.03;
	osc3.width = 0.9;
	glide.freq = 0.005;

	SDL_PauseAudio(0); // start audio

	SDL_Surface* screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("oodio", NULL);
	RenderingContext ctx(screen);
	ctx.backgroundColor(0, 0, 127);

	// load images
	AmsFont font("amstradFont.bmp");
	Button  btn(5, 3, "start/stop");
	Button  btnf(5, 5, "filter");
	Slider  sld(5, 7, 10, 0.1, 2);
	Slider  cut(12, 5, 30, 0.0, 1.0);
	Slider  res(16, 3, 26, 0.0, 0.7);
	btn.onClic(&playStop);
	btnf.onClic(&activateFilter);
	sld.onChange(&changeLfo);
	cut.onChange(&cutoff);
	res.onChange(&resonance);

	font.paper(19);
	font.pen(6);
	for (int i = 0; i < argc; i++) {
		font.print(argv[i]);
		font.print("\n");
	}
	btn.draw(&font);
	btnf.draw(&font);
	font.paper(1);
	font.pen(24);


	// program main loop
	bool run = true;
	while (run) {

		//-------------------------
		// message processing loop
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// check for messages
			switch (event.type) {
			case SDL_QUIT:
				// exit if the window is closed
				run = false;
				break;

			// check for keypresses
			case SDL_KEYDOWN:
				// exit if ESCAPE is pressed
				if (event.key.keysym.sym == SDLK_ESCAPE) run = false;
				// start / stop sound with F1 key
				else if (event.key.keysym.sym == SDLK_F1) playStop();
				else if (event.key.keysym.sym == SDLK_F2) osc1.freq = 110;
				else if (event.key.keysym.sym == SDLK_F3) osc1.freq = 440;
				// keyboard
				else if (event.key.keysym.sym <= 256 && event.key.keysym.sym >= 0 ) {
					font.print(event.key.keysym.sym);
				}
				break;

			case SDL_MOUSEMOTION:
				// font.locate(event.motion.x / (8 * PIXEL), event.motion.y / (8 * PIXEL));
				// font.paper(-1);
				// font.print('*');
				sld.move(event.motion.x, event.motion.y);
				cut.move(event.motion.x, event.motion.y);
				res.move(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				btn.clic(event.button.x, event.button.y);
				btnf.clic(event.button.x, event.button.y);
				sld.clic(event.button.x, event.button.y);
				cut.clic(event.button.x, event.button.y);
				res.clic(event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				sld.unclic(event.button.x, event.button.y);
				cut.unclic(event.button.x, event.button.y);
				res.unclic(event.button.x, event.button.y);
				break;
			}
		}

		ctx.clear();
		sld.draw(&font);
		cut.draw(&font);
		res.draw(&font);
		ctx.drawImage(font.getImage(), 0, 0);
		ctx.update();

		SDL_Delay(40); // 25 FPS
	}

	SDL_CloseAudio(); // close audio

	return 0;
}
