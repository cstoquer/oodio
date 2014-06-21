#include "OscRamp.h"
#include "constants.h"

const float rate = (float) SAMPLE_RATE * 2;

float OscRamp::tic() {
	pos += (cycle * freq) / rate;
	if (pos > cycle) pos -= cycle;
	return pos - 1;
}
