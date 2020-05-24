# Steering-Wheel-Project
Haptic Steering Wheel

NOTE: *** G27 Logitech driver - set to 900 degrees of rotation. No spring or centring. Everything to 100% ***

Uses SDL2 libary - see http://lazyfoo.net/tutorials/SDL/01_hello_SDL/index.php for install

Uses MS Visual Studio 2019

The Haptic features of SDL are not very well documented.
Examples that you will find around the net focus upon specific games and normally just
test the "rumble" ability of many controllers.

Testing so far has been carried out upon Logitech G27 Force Feedback Racing Wheel

Notes: 
G27 uses SDL_HAPTIC_CARTESIAN and not SDL_HAPTIC_POLAR for direction
No Autocentre. The Logitech driver provides this.

This program will list the Wheels abilities (effects that can be applied) and the operating system makes a difference.
Linux very few effects can be applied
Windows 10 - many effects are available
