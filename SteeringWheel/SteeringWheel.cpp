
// SDL includes
#include <SDL.h>
#include <iostream>
#include "Wheel.h"


//Frees media and shuts down SDL
void close();

//Game Controller 1 handler
SDL_Joystick* wheel = NULL;

void close()
{
	//Close game controller
	SDL_JoystickClose(wheel);
	wheel = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Create a wheel instance
	Wheel wheela;

	// Init SDL and find haptic abilities
	wheela.init();
	
	// Run tests on wheel 0
	wheela.hapticTest();



	//if (!init())
	//{
	//	std::cout << "Failed to initialize!" << std::endl;
	//}
	//else
	//{
	//	//Main loop flag
	//	bool quit = false;

	//	//Event handler
	//	SDL_Event e;
	//	wheel = SDL_JoystickOpen(0);
	//	//While application is running
	//	while (!quit)
	//	{
	//		//Handle events on queue
	//		while (SDL_PollEvent(&e) != 0)
	//		{
	//			//User requests quit
	//			if (e.type == SDL_QUIT)
	//			{
	//				quit = true;
	//			}
	//			//Joystick button press
	//			else if (e.type == SDL_JOYBUTTONDOWN)
	//			{
	//				int r = test_haptic();
	//			}

	//		}
	//	}
	//	SDL_Delay(30);
	//}

	//Free resources and close SDL
	close();

	return 0;
}

