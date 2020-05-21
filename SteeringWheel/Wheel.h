#pragma once

#include <SDL.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <ctime>
#include <ctype.h> // TODO do we need this?
#include "hapticEffects.h"
#include "Profile.h"

// Send debug output messages to console
const bool WHEEL_DEBUG_OUTPUT = true;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

// Max number of joysticks / wheels 
const int MAX_WHEELS = 1;

const uint8_t HAPTIC = 1;
const uint8_t NON_HAPTIC = 2;
const uint8_t ERROR = -1;
const clock_t GENERAL_TIMEOUT = 1000;

// Wheel inherits effects
class Wheel : public HE::hapticEffects
{
private:
	bool error;
	void displayWheelAbilities();
	void toConsole(const std::string msg);
	void hapticSine();
	void hapticTriangle();
	void hapticSawToothUp();
	void spring();
	void initEffect();

	// Min holds the left most position when stopped - NOT the actual position
	// because an end stop bounce may have happened
	void leftMostPositionWhenStopped(SDL_Event& e, Sint16 &min);

	int uploadExecuteEffect();
	SDL_HapticEffect effect;

public:
	Wheel();
	void init();
	void hapticTest();
	int readWheelPosition();
	int getNumberOfButtons();
	void centre();
	bool waitForNoMovement();
	void profiler();

	// Multiple devices may or may not be haptic 
	uint8_t deviceIndex[MAX_WHEELS]= { ERROR };			
	
	SDL_Joystick* wheel = nullptr;
	SDL_Haptic* haptic = nullptr;

};
