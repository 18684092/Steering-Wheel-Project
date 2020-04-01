#pragma once

#include <SDL.h>
#include <cstdint>
#include <iostream>
#include <string>

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

// Max number of joysticks / wheels 
const int MAX_WHEELS = 8;

const uint8_t HAPTIC = 1;
const uint8_t NON_HAPTIC = 2;
const uint8_t ERROR = -1;

class Wheel
{
private:
	bool error;
	void displayWheelAbilities();
	void toConsole(std::string msg);
	void hapticSine();
	void hapticTriangle();
	void hapticSawToothUp();
	void hapticConstantRight();
	void hapticConstantLeft();
	void spring();
	void initEffect();
	void hapticSetDirectionC(char d);
	int uploadExecuteEffect();
	SDL_HapticEffect effect;

public:
	void init();
	void hapticTest();
	uint8_t deviceIndex[MAX_WHEELS]= { ERROR };			
	SDL_Joystick* wheel = nullptr;
	SDL_Haptic* haptic = nullptr;

};

