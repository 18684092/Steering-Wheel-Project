#include "Wheel.h"


// Simply display on console
void Wheel::toConsole(std::string msg)
{
	std::cout << "Wheel: " << msg;
}

Wheel::Wheel()
{
	memset(&effect, 0, sizeof(SDL_HapticEffect)); // 0 is safe default
}

int Wheel::readWheelPosition()
{
	int pos = SDL_JoystickGetAxis(wheel, 0);
	return pos;
}

int Wheel::getNumberOfButtons()
{
	return SDL_JoystickNumButtons(wheel);
}

// For diagnostic purposes
void Wheel::displayWheelAbilities()
{
	toConsole("Haptic Abilities\n");
	toConsole("----------------\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SINE)  toConsole("has sine effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_CONSTANT) toConsole("has constant effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_LEFTRIGHT) toConsole("has leftright effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_TRIANGLE) toConsole("has triangle wave effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SAWTOOTHUP) toConsole("has saw tooth up effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SAWTOOTHDOWN) toConsole("has saw tooth down effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_RAMP) toConsole("has ramp effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SPRING) toConsole("has spring effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_DAMPER) toConsole("has damper effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_INERTIA) toConsole("has inertia effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_FRICTION) toConsole("has friction effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_CUSTOM) toConsole("has custom effect\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_GAIN) toConsole("can set gain\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_AUTOCENTER) toConsole("has auto centre effect\n ");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_PAUSE)  toConsole("can be paused\n");
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_STATUS)  toConsole("can have its status queried\n");
	if (SDL_HapticRumbleSupported(haptic)) toConsole("support haptic rumble\n");
}

// See SDL_Haptic.h
//   __      __      __      __
//  /  \    /  \    /  \    /
// /    \__/    \__/    \__/
//
void Wheel::hapticSine()
{
	effect.type = SDL_HAPTIC_SINE;
	// TODO why do we need to reset these?
	effect.periodic.period = 100; // 100 ms
	effect.periodic.magnitude = 32000; // 32000 of 32767 strength
	effect.periodic.offset = 0;
	effect.periodic.phase = 0;
}

// See SDL_Haptic.h
//   /\    /\    /\    /\    /\
//  /  \  /  \  /  \  /  \  /
// /    \/    \/    \/    \/
void Wheel::hapticTriangle()
{
	effect.type = SDL_HAPTIC_TRIANGLE;
	// TODO why do we need to reset these?
	effect.periodic.period = 100; // 100 ms
	effect.periodic.magnitude = 32000; // 32000 of 32767 strength
	effect.periodic.offset = 0;
	effect.periodic.phase = 0;
}

// See SDL_Haptic.h
//   /|  /|  /|  /|  /|  /|  /|
//  / | / | / | / | / | / | / |
// /  |/  |/  |/  |/  |/  |/  |
void Wheel::hapticSawToothUp()
{
	effect.type = SDL_HAPTIC_SAWTOOTHUP;
	// TODO why do we need to reset these?
	effect.periodic.period = 100; // 100 ms
	effect.periodic.magnitude = 32000; // 32000 of 32767 strength
	effect.periodic.offset = 0;
	effect.periodic.phase = 0;
}

void Wheel::spring()
{
	effect.type = SDL_HAPTIC_SPRING;
	effect.condition.length = 5000;
	effect.condition.right_sat[0] = 0xFFFF;
	effect.condition.left_sat[0] = 0xFFFF;
	effect.condition.right_coeff[0] = 0x2000;
	effect.condition.left_coeff[0] = 0x2000;
	effect.condition.deadband[0] = 0x100;
	effect.condition.center[0] = 0x1000;     /* Displace the center for it to move. */
}

// Set default initial effect
void Wheel::initEffect()
{
	

	// SDL_HAPTIC_TRIANGLE SDL_HAPTIC_SINE SDL_HAPTIC_SAWTOOTHUP
	effect.periodic.direction.type = SDL_HAPTIC_CARTESIAN;
	effect.periodic.direction.dir[0] = 0;
	effect.periodic.direction.dir[1] = -1; // Force comes from south
	effect.periodic.direction.dir[2] = 0;

	effect.periodic.length = 3000; // 5 seconds long
	effect.periodic.delay = 0;

	effect.periodic.period = 100; // 100 ms
	effect.periodic.magnitude = 32000; // 32000 of 32767 strength
	effect.periodic.offset = 0;
	effect.periodic.phase = 0;

	effect.periodic.attack_length = 50; // Takes 50ms to get max strength
	effect.periodic.attack_level = 32000;
	effect.periodic.fade_length = 50; // Takes 50ms to fade away
	effect.periodic.fade_level = 0;

}

// Wait till wheel stops moving
bool Wheel::waitForNoMovement()
{
	// Start timeout timer
	std::clock_t start = std::clock();
	int last = 1;
	int current = 0;

	// Wait till wheel stops moving
	while (last != current)
	{
		SDL_JoystickUpdate();
		current = readWheelPosition();
		SDL_Delay(8);
		last = readWheelPosition();

		// Timed out return
		if ((std::clock() - start) >= GENERAL_TIMEOUT) return false;
	}

	// return wheel stopped
	return true;
}

void Wheel::centre()
{
	// Time of effect
	Uint32 duration = 10;

	// Strength level
	Uint16 level = 10000;

	// Make effects
	int effect_left = setConstantForce(duration, level, HE::LEFT);
	int effect_right = setConstantForce(duration, level, HE::RIGHT);

	std::cout << "Centering...";

	// Force update as there is no event
	SDL_JoystickUpdate();
	int pos = readWheelPosition();

	// Find the centre as best we can
	while ( pos > 20 || pos < -20 )
	{	
		// Force update so we can read position
		SDL_JoystickUpdate();
		pos = readWheelPosition();

		// If we are to the right of centre
		if (pos > 0)
		{
			runEffect(HE::CONSTANT_LEFT, 1);

			// For large movements let effect run but not to the end to avoid vibration
			if (pos > 500) SDL_Delay(duration - 2); else waitForNoMovement();
		}
		// If we are left of centre
		else if (pos < 0)
		{
			runEffect(HE::CONSTANT_RIGHT, 1);

			// For large movements let effect run but not to the end to avoid vibration
			if (pos < -500) SDL_Delay(duration - 2); else waitForNoMovement();
		}

		// If close to middle, slow down
		if (pos < 500 && pos > -500 ) SDL_Delay(100);
	}

	// Let it settle
	waitForNoMovement();

	// Output position 
	SDL_JoystickUpdate();
	std::cout << "done (pos=" << readWheelPosition() << ")" << std::endl;
}

// Helper function to send effect to wheel controller
int Wheel::uploadExecuteEffect()
{

	// Upload the effect
	int effect_id = SDL_HapticNewEffect(haptic, &effect);

	// error?
	if (effect_id < 0)
	{
		std::string msg = SDL_GetError();
		toConsole("Error: " + msg + "\n");
	}

	// Test the effect
	effect_id = SDL_HapticRunEffect(haptic, effect_id, 1);
	
	// Error?
	if (effect_id < 0)
	{
		std::string msg = SDL_GetError();
		toConsole("Error: " + msg + "\n");
	}

	// Return it (negative is failure)
	return effect_id;
}

// For testing purposes only
void Wheel::hapticTest()
{

	int effect_id;
	
	// TODO force first device to be used
	wheel = SDL_JoystickOpen(0);
	haptic = SDL_HapticOpenFromJoystick(wheel);

	if (haptic == NULL) return ; // Most likely joystick isn't haptic
	
	// See if it can do sine waves
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SINE) == 0) 
	{
		SDL_HapticClose(haptic); // No sine effect
		return ;
	}
	

	//// test 0
	spring();
	toConsole("Trying Haptic Spring...\n");
	effect_id = uploadExecuteEffect();
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);

	SDL_HapticSetGain(haptic, 100);
	centre();
	SDL_Delay(5000);

	// test 1
	toConsole("Trying Constant Force Right...\n");
	effect_id = runEffect(HE::CONSTANT_RIGHT, 100);
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);

	// test 2
	toConsole("Trying Constant Force Left...\n");
	effect_id = runEffect(HE::CONSTANT_LEFT, 100);
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000); 

	// test 3
	hapticSine();
	toConsole("Trying Haptic Sine...\n");
	effect_id = uploadExecuteEffect();
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);

	// test 4
	hapticTriangle();
	toConsole("Trying Haptic Triangle...\n");
	effect_id = uploadExecuteEffect();
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);



	// test 5
	hapticSawToothUp();
	toConsole("Trying Haptic Sawtooth Up...\n");
	effect_id = uploadExecuteEffect();
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);

	// test 6
	// Initialize simple rumble
	toConsole("Trying Haptic Rumble...\n");
	effect_id = SDL_HapticRumbleInit(haptic);	
	// Play effect at 50% strength for 2 seconds
	if (effect_id == 0) SDL_HapticRumblePlay(haptic, 0.5, 2000);
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(2000);
	
	// Destroy this effect
	SDL_HapticDestroyEffect(haptic, effect_id);
	
	// Close the device
	SDL_HapticClose(haptic);
}

// Init SDL and find joysticks / wheels with the right abilities
void Wheel::init()
{
	

	//Initialize SDL
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0)
	{
		std::string e = SDL_GetError();
		toConsole("SDL could not initialize! SDL Error:\n"  + e + "\n");
		error = true;
	}
	else
	{
		//Check for joysticks
		int num_wheels = SDL_NumJoysticks();
		
		if (num_wheels < 1)
		{
			toConsole("Error: No joysticks / wheels connected!\n");
		}
		else
		{
			// We have wheels
			int devicePointer = 0;

			// test each wheel upto MAX_WHEEL number
			for (int i = 0; i < num_wheels && i < MAX_WHEELS; ++i)
			{
				SDL_JoystickEventState(SDL_ENABLE);
				wheel = SDL_JoystickOpen(i);

				// Open wheel
				if (wheel != NULL)
				{
					// Display info 
					std::string name = SDL_JoystickName(wheel);
					std::string num = std::to_string(SDL_JoystickNumAxes(wheel));
					haptic = setHaptic(i); ; // SDL_HapticOpen(i);
					
					
					
					
					int nEffects = SDL_HapticNumEffects(haptic);
					toConsole("Found <" + name + "> with axis [" + num + "]  Device# [" + std::to_string(i) + "] effects [" + std::to_string(nEffects) + "]\n");

					// Haptic wheel?
					if (haptic == NULL)
					{
						deviceIndex[devicePointer] = NON_HAPTIC;
						toConsole("has no haptic ability\n");
					}
					else
					{
						deviceIndex[devicePointer] = HAPTIC;
						toConsole("has haptic ability\n");

						// Try Auto centre (G27 doesn't have auto centre - relies on Logitech driver)
						if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_AUTOCENTER))
						{
							if (SDL_HapticSetAutocenter(haptic, 50) == 0)
							{
								toConsole("wheel was centred with force 50\n");
							}
							else
							{
								toConsole("Error: wheel centreing failed\n");
							}
						}

						// display abilities
						displayWheelAbilities();
					}
				}
				else
				{
					// Wheel could not be opened - ODD? Something was there!
					deviceIndex[devicePointer] = ERROR;
				}

				// Next device
				++devicePointer;

			} // each wheel

		} // end we have wheels

	} // end SDL init tests
	
	initEffect();

} // end init