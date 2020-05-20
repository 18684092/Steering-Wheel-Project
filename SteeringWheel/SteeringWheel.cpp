
// SDL includes
#include <SDL.h>
#include <iostream>
#include <ctime>
#include "Wheel.h"

#define DEAD_LEFT 0
#define DEAD_RIGHT 0

int main(int argc, char* args[])
{
	// Last position
	int last = 0;
	int current = 0;

	int power = 0;
	int count = 0;
	bool profiling = false;

	int effect_id;

	// Time
	std::clock_t start = 0;
	clock_t duration = 0;
	clock_t timeBetweenReadings = 10; // in milli seconds

	//Create a wheel instance
	Wheel wheel;

	// Init SDL and find haptic abilities
	wheel.init();
	
	// Run tests on wheel 0
	//wheel.hapticTest();

	// Find min and max travel 
	int el = wheel.setConstantForce(4000, 10000, HE::RIGHT);
	effect_id = wheel.runEffect(HE::CONSTANT_RIGHT, 1);
	wheel.findMaxWheelPosition();

	el = wheel.setConstantForce(4000, 10000, HE::LEFT);
	effect_id = wheel.runEffect(HE::CONSTANT_LEFT, 1);
	SDL_Delay(4000);

	el = wheel.setConstantForce(4000, 10000, HE::LEFT);
	effect_id = wheel.runEffect(HE::CONSTANT_LEFT, 1);
	
	wheel.findMinWheelPosition();


	// Centre the wheel
	wheel.centre();

	wheel.profiler();


	SDL_JoystickUpdate();
	last = wheel.readWheelPosition();

	//Main loop flag
	bool quit = false;

	//Event handler
	SDL_Event e;
	start = std::clock();
	//While application is running
	while (!quit)
	{
			
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			// Time now
			clock_t now = clock();
			
			////////////
			// Events //
			////////////
			switch (e.type)
			{
				// Keyboard
				case SDL_KEYDOWN:
					// News a window opening for this event to work
					break;

				// QUIT
				case SDL_QUIT:
					quit = true;
					break;

				// Motion x axis has been triggered
				case SDL_JOYAXISMOTION:
					if ((e.jaxis.value < DEAD_LEFT) || (e.jaxis.value > DEAD_RIGHT))
					{
						if (e.jaxis.axis == 0)
						{
		
							// Duration in seconds
							duration = (now - start);

							// Get wheels position
							//current = wheel.readWheelPosition();
							current = e.jaxis.value;

							// Test duration
							if (duration >= timeBetweenReadings)
							{
								count++;
								std::cout << count << " F: " << power << " P: " << current << " L: " << last << " T: " << duration << "mS" << " D: " << (last - current) << " Time: " << now << std::endl;
								//std::cout << count << "," << power << "," << current << "," << last << "," << duration << "," << std::abs(last - current) << "," << now << std::endl;

								// Reset Clock
								start = std::clock();

								// Store position
								last = current;
							}
						}

					}
					break;

				// Wheel buttons
				case SDL_JOYBUTTONDOWN:  
					std::cout << "Button: " << (int)e.jbutton.button << " of " << (int)wheel.getNumberOfButtons() << " Position: " << wheel.readWheelPosition() << std::endl;
						
					// These buttons are on G27 gear stick and are red
					if (e.jbutton.button == 0)
					{
						quit = true;
					}
					if (e.jbutton.button == 1)
					{
						int el = wheel.setConstantForce(1000, power, HE::LEFT);
						effect_id = wheel.runEffect(HE::CONSTANT_LEFT, 1);
						profiling = true;
						start = std::clock();							
					}
					if (e.jbutton.button == 2)
					{
						int el = wheel.setConstantForce(1000, power, HE::RIGHT);
						effect_id = wheel.runEffect(HE::CONSTANT_RIGHT, 1);
						profiling = true;
						start = std::clock();
					}
					if (e.jbutton.button == 3)
					{
						power += 1000;
						count = 0;
							
					}

					// These buttons are on G27 Steering wheel
					if (e.jbutton.button == 7)
					{
						wheel.centre();
					}

					if (e.jbutton.button == 20)
					{
						wheel.profiler();
					}
					break;
			}


		}
		//SDL_Delay(30);
	}



	return 0;
}



