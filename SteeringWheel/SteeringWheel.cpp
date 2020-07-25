/*
Author: Andy Perrett
Email: andy@wired-wrong.co.uk

Version 0.1

*/

#include "Wheel.h"
#include <ctime>
#include <iostream>

// levels
constexpr auto LEVEL8 = 8000;
constexpr auto LEVEL10 = 10000;
constexpr auto LEVEL32 = 32000;
constexpr auto LEVEL20 = 20000;
constexpr auto LEVEL15 = 15000;


// Name the Wheel we want to use (G27 or Steering) 
constexpr auto NAME = "G27";
constexpr auto TIMEOUT = 120000;


int main(int argc, char** argv)
{
    std::cout << "Plug in haptic wheel within 2 minutes..." << std::endl;

    // Wait for haptic wheel to be plugged in
    bool found = false;
    clock_t start = clock();
    while (!found)
    {
        // Create a wheel object
        Wheel* test = new Wheel(NAME);
        if (test->validDevice() && test->validHaptic()) found = true;
        //test->wait(1000);
        delete test;
        clock_t now = clock();
        if (now - start >= TIMEOUT && !found) break;
    }

    // We have a device
    if (found)
    {
        Wheel* wheel = new Wheel(NAME, true);
        if (wheel->validDevice() && wheel->validHaptic())
        {
            //wheel->getGain();
            //wheel->setGain(50);
            //wheel->getGain();
            //wheel->getMaxGain();
            //wheel->wait(3000);
            wheel->calibrate();

            //wheel->wait(2000);

            //wheel->setMaxGain(100);
            //wheel->setGain(100);
            //wheel->profile();
            /*
            wheel->gotoAngle(340);
            wheel->setRampLeft(1000, L32, 0);
            wheel->setRampRight(1000, 0, L32);
            wheel->setSine(FOREVER, 120, 20000, DOWN);
            wheel->runEffect(RAMP_LEFT);
            wheel->wait(1500);
            wheel->runEffect(RAMP_RIGHT);
            //wheel->runEffect(SINE);
            wheel->wait(2000); // 2 seconds
            wheel->gotoAngle(90);
            wheel->wait(2000);
            wheel->gotoAngleSlow(-120);
            */
            //wheel->setLeft(5000, 8000);
            //wheel->setDamper(FOREVER, 0, 20000 , 20000, -10000, -10000, 100, 100);
            //wheel->setSpring(FOREVER, 0, 32000, 32000, 32000, 32000, 500, 0);
            //wheel->runEffect(LEFT);
            //wheel->wait(3000);
            //wheel->setInertia(FOREVER, 0, 32000, 32000, 32000, 32000);
            //wheel->setInertia(FOREVER, 0, 10000, 10000, 10000, 10000);
            //wheel->setFriction(FOREVER, 0, 10000, 10000, 20000, 20000);
            //wheel->setSine(FOREVER, 120, 20000, DOWN);
            //wheel->setTriangle(FOREVER, 150, 8000, DOWN);
            //wheel->setSawUp(FOREVER, 150, 8000, DOWN);
            //wheel->runEffect(SPRING);
            //wheel->runEffect(SPRING);
            //wheel->runEffect(DAMPER);
            //wheel->runEffect(SPRING);
            //wheel->wait(1000);

            //wheel->runEffect(SINE);
            //wheel->gotoAngle(0);
            //wheel->setLeft(500, L32);
            //wheel->runEffect(LEFT, 1);
            //wheel->wait(250);
            //wheel->getDistance(10);
            //wheel->setGain(100);
            //wheel->wait(5000);


            //wheel->setMaxGain(55);
            //wheel->getMaxGain();
           // wheel->setGain(100);
            //
            /*
            wheel->runEffect(DAMPER, 1);
            wheel->gotoAngle(38);
            wheel-> wait(2000);
            wheel->gotoAngle(-90);
            wheel->wait(2000);
            wheel->gotoAngle(90);
            wheel->wait(2000);
            wheel->gotoAngleSlow(-120);
            wheel->wait(2000);
            wheel->gotoAngleFast(0);
            wheel->wait(2000);
            wheel->gotoAngleFast(-120);
            wheel->wait(2000);
            wheel->gotoAngleFast(90);
            wheel->wait(2000);
            wheel->gotoAngleSlow(0);
            wheel->wait(2000);
            wheel->gotoAngleSlow(1);
            wheel->wait(2000);
            wheel->gotoAngle(2);
            wheel->wait(2000);
            wheel->gotoAngleFast(3);
            wheel->wait(2000);
            wheel->gotoAngleFast(0);
            wheel->wait(1000);
            */

            wheel->setDamper(FOREVER, 0, 32000, 32000, 10000, 10000, 0, wheel->getCentre());
            wheel->runEffect(DAMPER);
            wheel->setSpring(FOREVER, 0, 25000, 25000, 25000, 25000, 0, wheel->getCentre());
            wheel->runEffect(SPRING);
            wheel->setRight(FOREVER, 32000);
            wheel->setSine(FOREVER, 50, 20000, DOWN);
            wheel->setRampRight(1000, 32000, 10000);
            int last = 0;
            int now = 0;
            bool backstop = false;
            bool done = false;
            while (!done)
            {
                now = wheel->getPosition();

                if (last != now)
                {
                    std::cout << "Position: " << now << std::endl;
                    //std::cout << "Angle   : " << wheel->getAngle() << std::endl;

                    if (now < wheel->calculatePosition(-30) && !backstop)
                    {
                        wheel->runEffect(RAMP_RIGHT);
                        //->runEffect(SINE);
                        backstop = true;
                    }
                    if (now > wheel->calculatePosition(-30) && backstop)
                    {
                        backstop = false;
                        wheel->stopEffect(RAMP_RIGHT);
                        //wheel->stopEffect(SINE);

                    }

                }
                last = now;


            }


            wheel->wait(20000);

            delete wheel;
        }
    }
    // Timedout - no usable device
    else std::cout << "TIMEOUT : No device " << std::endl;

    return 0;
}

