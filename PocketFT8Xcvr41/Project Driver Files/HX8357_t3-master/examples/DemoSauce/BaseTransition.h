#ifndef BASE_TRANSITION_H__
#define BASE_TRANSITION_H__

#include <Arduino.h>
#include "HX8357_t3.h"
#include "MathUtil.h"

class BaseTransition {
public:
	BaseTransition(){};

	virtual void init( HX8357_t3 tft );
	virtual void restart( HX8357_t3 tft, uint_fast16_t color );
	virtual void perFrame( HX8357_t3 tft, FrameParams frameParams );
	virtual boolean isComplete();
};

void BaseTransition::init( HX8357_t3 tft ) {
	// Extend me
}

void BaseTransition::restart( HX8357_t3 tft, uint_fast16_t color ) {
	// Extend me
}

void BaseTransition::perFrame( HX8357_t3 tft, FrameParams frameParams ) {
	// Extend me
}

boolean BaseTransition::isComplete() {
	// Extend me
	return false;
}

#endif
