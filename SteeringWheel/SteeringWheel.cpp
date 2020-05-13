
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

	wheel.centre();

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

								current = wheel.readWheelPosition();

								if (duration >= timeBetweenReadings)
								{
									count++;
									//std::cout << count << " F: " << power << " P: " << current << " L: " << last << " T: " << duration << "mS" << " D: " << (last - current) << " Time: " << now << std::endl;
									std::cout << count << "," << power << "," << current << "," << last << "," << duration << "," << (last - current) << "," << now << std::endl;

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
						if (e.jbutton.button == 0)
						{
							quit = true;
						}
						if (e.jbutton.button == 7)
						{
							wheel.centre();
						}
						if (e.jbutton.button == 1)
						{
							int el = wheel.setConstantForce(1000, power, HE::LEFT);
							int effect_id = wheel.runEffect(HE::CONSTANT_LEFT, 1);
							start = std::clock();
						}
						if (e.jbutton.button == 2)
						{
							int el = wheel.setConstantForce(1000, power, HE::RIGHT);
							int effect_id = wheel.runEffect(HE::CONSTANT_RIGHT, 1);
							start = std::clock();
						}
						if (e.jbutton.button == 3)
						{
							power += 1000;
							count = 0;
							
						}
						break;
				}


			}
			//SDL_Delay(30);
		}



	return 0;
}



