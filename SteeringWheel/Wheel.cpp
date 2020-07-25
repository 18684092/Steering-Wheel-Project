#include "Wheel.h"

/*
Author: Andy Perrett
Email: andy@wired-wrong.co.uk

Version 0.1

*/

Wheel::Wheel(const std::string name, bool debug) : debug(debug), deviceNumber(DEVICE_ERROR), hasHaptic(false)
{
	leftLock = SDL_MAX_SINT16;
	rightLock = SDL_MIN_SINT16;
	centre = 0;
	jitter = 0;
	hapticGain = EFFECT_ERROR;




	// Initialise effect
	resetEffect();

	//Initialize SDL
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0)
	{
		deviceNumber = DEVICE_ERROR;
		log("Could not initialise SDL Joystick or SDL Haptic system. (" + std::string(SDL_GetError()) + ")");
	}
	else
	{
		log("SDL Subsystem initialised");

		//Check for joysticks
		int numWheels = SDL_NumJoysticks();

		if (numWheels < 1)
		{
			deviceNumber = DEVICE_ERROR;
		}
		else
		{
			log("Searching for Joysticks / Wheels ...");

			// Cycle through Joysticks
			for (int i = 0; i < numWheels; ++i)
			{
				joy = SDL_JoystickOpen(i);
				if (joy != nullptr)
				{
					std::string jName = SDL_JoystickName(joy);
					int n = jName.find(name);
					log("Joy ID: " + std::to_string(i) + std::string(n != std::string::npos ? " *" : "  ") + " " + jName);
					if (n != std::string::npos)
					{
						deviceNumber = i;
						SDL_JoystickClose(joy);
						break;
					}
				}
			}

			// Report which Joystick to use
			if (deviceNumber >= 0)
			{
				log("Using Joystick with ID: " + std::to_string(deviceNumber));

				// Open joystick
				joy = SDL_JoystickOpen(deviceNumber);

				if (deviceNumber > DEVICE_ERROR) testHapticAbilitiy();
				log("Waiting for device to settle");
				wait(7000); // TODO Driver may be moving wheel - perhaps test if moving?

				if (debug)
				{
					if (hasSine()) log("Has Sine Wave Effect");
					if (hasConstant()) log("Has Constant Effect");
					if (hasLeftRight()) log("Has LeftRight Effect");
					if (hasTriangle()) log("Has Triangle Wave Effect");
					if (hasSawUp()) log("Has Sawtooth Up Effect");
					if (hasSawDown()) log("Has Sawtooth Down Effect");
					if (hasRamp()) log("Has Ramp Effect");
					if (hasSpring()) log("Has Spring Effect");
					if (hasDamper()) log("Has Damper Effect");
					if (hasInertia()) log("Has Inertia Effect");
					if (hasFriction()) log("Has Friction Effect");
					if (hasCustom()) log("Has Custom Effect");
					if (canSetGain()) log("Has Set Gain Effect");
					if (hasAutoCentre()) log("Has Auto Centre Effect");
					if (canGetStatus()) log("Can Get Status");
					if (canPause()) log("Can Pause Effect");
				}
			}
			else
			{
				log("No Joystick or wheel can be used.");
			}
		}

	}

	// Set gain to max (it may be scalled by SDL_HAPTIC_GAIN_MAX)
	setMaxGain(100);
	setGain(100);
}

// Any and all effects that are uploaded are deleted
void Wheel::destroyAllEffects()
{
	for (unsigned int i = MIN_EFFECT_NUMBER; i <= MAX_EFFECT_NUMBER; i++)
	{
		destroyEffect(i);
	}
}

// Destructor - cleanup
Wheel::~Wheel()
{
	log("Wheel destructor");

	if (haptic != NULL)
	{
		SDL_HapticClose(haptic);
		haptic = nullptr;
	}

	if (deviceNumber != DEVICE_ERROR)
	{
		destroyAllEffects();
		SDL_JoystickClose(joy);
		joy = nullptr;
	}

	// TODO Mutliple joysticks will cause an issue here
	SDL_QuitSubSystem(SDL_INIT_HAPTIC);
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	wait(1000); // Seem to run into problems without waiting
	SDL_Quit();
}

// Scale a level
Uint16 Wheel::scaleLevel(Uint16 lvl)
{
	return (Uint16)(lvl * FORCE_SCALE);
}

// Taken from:
// https://stackoverflow.com/questions/16077299/how-to-print-current-time-with-milliseconds-using-c-c11
// Enrico Pintus
// Andy Perrett modified slightly
std::string Wheel::getTimeStr()
{
	using std::chrono::system_clock;
	auto currentTime = std::chrono::system_clock::now();
	char buffer[80];
	auto transformed = currentTime.time_since_epoch().count() / 10000; // was 1000000
	auto millis = transformed % 1000;
	std::time_t tt;
	tt = system_clock::to_time_t(currentTime);
	auto timeinfo = localtime(&tt);
	strftime(buffer, 80, "%F %H:%M:%S", timeinfo);
	sprintf_s(buffer, "%s (%03d mS)", buffer, (int)millis);
	return std::string(buffer);
}

// Log to screen or file
// TODO make log file
void Wheel::log(std::string msg, int place)
{
	if (!debug) return;

	if (place == SCREEN)
	{
		std::cout << getTimeStr() << " " << msg << std::endl;
	}
}

// Read x axis of wheel
Sint16 Wheel::getPosition()
{
	SDL_JoystickUpdate();
	int position = SDL_JoystickGetAxis(joy, 0);
	//log("Position: " + std::to_string(p));
	return position;
}

Sint16 Wheel::calculateAngle(Sint16 position)
{
	Uint16 range = std::abs(leftLock) + std::abs(rightLock);
	return (Sint16)((position + OFFSET) / (float)(range / DEGREES));
}

// Calculate position from angle
Sint16 Wheel::calculatePosition(float angle)
{
	Uint16 range = std::abs(leftLock) + std::abs(rightLock);
	return (Sint16)(angle * (float)(range / DEGREES));
}

Sint16 Wheel::getAngle()
{
	Uint16 range = std::abs(leftLock) + std::abs(rightLock);
	return (Sint16)((getPosition() + OFFSET) / (float)(range / DEGREES));
}

bool Wheel::stopEffect(int effect)
{
	if (!checkEffectNumber(effect))
	{
		log("Error: Cant stop - Bad effect number");
		return false;
	}

	if (effectsMap[effect] == EFFECT_ERROR)
	{
		log("Error: Effect (" + effectsName[effect] + ") not uploaded");
		return false;
	}

	int result = SDL_HapticStopEffect(haptic, effectsMap[effect]);
	if (result != 0)
	{
		log("Error: Could not stop (" + effectsName[effect] + ") - " + SDL_GetError());
		return false;
	}
	log("Effect (" + effectsName[effect] + ") stopped");
	return true;
}

bool Wheel::isEffectRunning(int effect)
{
	if (!checkEffectNumber(effect))
	{
		log("Error: Bad effect number (isEffectRunning)");
		return false;
	}

	if (effectsMap[effect] == EFFECT_ERROR)
	{
		log("Error: Effect (" + effectsName[effect] + ") not uploaded");
		return false;
	}

	int result = SDL_HapticGetEffectStatus(haptic, effectsMap[effect]);
	if (result == 1) return true;
	return false;
}

// Tests to see if Joystick / wheel has haptic abilities
// Sets "hasHaptic" to true or false
void Wheel::testHapticAbilitiy()
{
	// *** NOTE *** SDL_HapticOpen(int device_index)
	// returns an error if there are 2 joysticks
	// one is non haptic and haptic one is device id 1
	// this is because SDL_haptic.c line 116 has bug
	// if ((device_index < 0) || (device_index >= SDL_numhaptics))
	// ie, deviceID = 1 matches numhaptics = 1
	haptic = SDL_HapticOpenFromJoystick(joy);
	if (haptic != NULL) hasHaptic = true;
}

// Test for Sine wave haptic ability
bool Wheel::hasSine()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SINE) return true;
	}
	return false;
}

// Test for Constant force haptic ability
bool Wheel::hasConstant()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_CONSTANT) return true;
	}
	return false;
}

// Test for LeftRight force haptic ability
bool Wheel::hasLeftRight()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_LEFTRIGHT) return true;
	}
	return false;
}

// Test for Triangle force haptic ability
bool Wheel::hasTriangle()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_TRIANGLE) return true;
	}
	return false;
}

// Test for SawtoothUp force haptic ability
bool Wheel::hasSawUp()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SAWTOOTHUP) return true;
	}
	return false;
}

// Test for SawtoothDown force haptic ability
bool Wheel::hasSawDown()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SAWTOOTHDOWN) return true;
	}
	return false;
}

// Test for Ramp force haptic ability
bool Wheel::hasRamp()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_RAMP) return true;
	}
	return false;
}

// Test for Spring force haptic ability
bool Wheel::hasSpring()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_SPRING) return true;
	}
	return false;
}

// Test for Spring force haptic ability
bool Wheel::hasDamper()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_DAMPER) return true;
	}
	return false;
}

// Test for Spring force haptic ability
bool Wheel::hasInertia()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_INERTIA) return true;
	}
	return false;
}

// Test for Friction force haptic ability
bool Wheel::hasFriction()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_FRICTION) return true;
	}
	return false;
}

// Test for Custom force haptic ability
bool Wheel::hasCustom()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_CUSTOM) return true;
	}
	return false;
}

// Test for ability to set gain force haptic ability
bool Wheel::canSetGain()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_GAIN) return true;
	}
	return false;
}

// Test for Spring force haptic ability
bool Wheel::hasAutoCentre()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_AUTOCENTER) return true;
	}
	return false;
}

// Test for status haptic ability
bool Wheel::canGetStatus()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_STATUS) return true;
	}
	return false;
}

// Test for for ability to pause haptic ability
bool Wheel::canPause()
{
	if (hasHaptic)
	{
		if (SDL_HapticQuery(haptic) & SDL_HAPTIC_PAUSE) return true;
	}
	return false;
}

// Test for Rumble force haptic ability
bool Wheel::hasRumble()
{
	if (hasHaptic)
	{
		if (SDL_HapticRumbleSupported(haptic)) return true;
	}
	return false;
}

bool Wheel::validDevice()
{
	if (deviceNumber != DEVICE_ERROR) return true;
	return false;
}

bool Wheel::validHaptic()
{
	return hasHaptic;
}

int Wheel::numEffectsPlaying()
{
	if (hasHaptic) return SDL_HapticNumEffectsPlaying(haptic);
	return EFFECT_ERROR;
}

// Reset effect memory
void Wheel::resetEffect()
{
	memset(&effect, 0, sizeof(SDL_HapticEffect));
}

// Duration must be between range
bool Wheel::checkDuration(Uint32 mS)
{
	if (mS < MIN_DURATION || mS > SDL_MAX_UINT32)
	{
		log("Error: duration in mS Out of Bounds");
		return false;
	}
	return true;
}

// Delay must be between range
bool Wheel::checkDelay(Uint32 dly)
{
	if (dly < MIN_DELAY || dly > SDL_MAX_UINT32)
	{
		log("Error: delay in mS Out of Bounds");
		return false;
	}
	return true;
}

// Attack length must be between range
bool Wheel::checkAttackLength(Uint32 aLen)
{
	if (aLen < MIN_ATTACK_LENGTH || aLen > SDL_MAX_UINT32)
	{
		log("Error: attack length in mS Out of Bounds");
		return false;
	}
	return true;
}

// Fade length must be between range
bool Wheel::checkFadeLength(Uint32 fLen)
{
	if (fLen < MIN_FADE_LENGTH || fLen > SDL_MAX_UINT32)
	{
		log("Error: fade length in mS Out of Bounds");
		return false;
	}
	return true;
}

// Check fade and attack time compared to overall duration
bool Wheel::checkEnvelopeTime(Uint32 mS, Uint32 aLen, Uint32 fLen)
{
	Uint32 tLen = aLen + fLen;
	if (tLen > mS)
	{
		log("Error: Attack + Fade length is greater than total duration");
		return false;
	}
	return true;
}

// Iterations must be between range
bool Wheel::checkIterations(Uint32 iterations)
{
	if (iterations < 1 || iterations > SDL_MAX_UINT32)
	{
		log("Error: iterations in mS Out of Bounds");
		return false;
	}
	return true;
}

// Level must be between range
bool Wheel::checkLevel(Uint16 level)
{
	if (level < MIN_LEVEL || level > SDL_MAX_UINT16)
	{
		log("Error: level Out of Bounds");
		return false;
	}
	return true;
}

// Level must be between range
bool Wheel::checkAttackLevel(Uint16 aLvl)
{
	if (aLvl < MIN_ATTACK_LEVEL || aLvl > SDL_MAX_UINT16)
	{
		log("Error: attack level Out of Bounds");
		return false;
	}
	return true;
}

// Level must be between range
bool Wheel::checkFadeLevel(Uint16 fLvl)
{
	if (fLvl < MIN_FADE_LEVEL || fLvl > SDL_MAX_UINT16)
	{
		log("Error: fade level Out of Bounds");
		return false;
	}
	return true;
}

// Saturation must be between range
bool Wheel::checkSatLevel(Uint16 sat)
{
	if (sat < MIN_SAT_LEVEL || sat > SDL_MAX_UINT16)
	{
		log("Error: Saturation level Out of Bounds");
		return false;
	}
	return true;
}

// Deadband must be between range
bool Wheel::checkDeadband(Uint16 dead)
{
	if (dead < MIN_DEADBAND || dead > SDL_MAX_UINT16)
	{
		log("Error: Deadband Out of Bounds");
		return false;
	}
	return true;
}

// Centre must be between range
bool Wheel::checkCentre(Sint16 centre)
{
	if (centre < MIN_CENTRE || centre > SDL_MAX_SINT16)
	{
		log("Error: Centre position Out of Bounds");
		return false;
	}
	return true;
}

// Coefficient must be between range
bool Wheel::checkCoEfLevel(Sint16 coef)
{
	if (coef < MIN_COEF_LEVEL || coef > SDL_MAX_SINT16)
	{
		log("Error: Coefficient level Out of Bounds");
		return false;
	}
	return true;
}

// Start must be between range
bool Wheel::checkStartLevel(Sint16 start)
{
	if (start < MIN_START_LEVEL || start > MAX_START_LEVEL)
	{
		log("Error: Start level Out of Bounds");
		return false;
	}
	return true;
}

// End must be between range
bool Wheel::checkEndLevel(Sint16 end)
{
	if (end < MIN_END_LEVEL || end > MAX_END_LEVEL)
	{
		log("Error: End level Out of Bounds");
		return false;
	}
	return true;
}

// Effect number must be within range
bool Wheel::checkEffectNumber(unsigned int effect)
{
	if (effect < MIN_EFFECT_NUMBER || effect > MAX_EFFECT_NUMBER)
	{
		log("Error: Effect ID Out of Bounds");
		return false;
	}
	return true;
}

// Is there a haptic device?
bool Wheel::checkHaptic()
{
	if (haptic == nullptr)
	{
		log("Error: haptic not set");
		return false;
	}
	return true;
}

// Gain must be between range
bool Wheel::checkGain(int gain)
{
	if (gain < MIN_GAIN || gain > MAX_GAIN)
	{
		log("Error: Gain Out of Bounds");
		return false;
	}
	return true;
}


// Will scale linearly using setMaxGain() as the maximum.
int Wheel::setGain(int gain)
{
	log("Setting gain to: " + std::to_string(gain));

	if (!checkHaptic() || !checkGain(gain))
	{
		log("Error: Gain not set");
		return EFFECT_ERROR;
	}
	int result = SDL_HapticSetGain(haptic, gain);
	if (result != 0)
	{
		log("Error: (setGain) " + std::string(SDL_GetError()));
		return EFFECT_ERROR;
	}

	hapticGain = gain;

	return 0;
}

// Report gain
int Wheel::getGain()
{
	log("Getting gain");

	if (hapticGain == EFFECT_ERROR)
	{
		log("Error: Gain not set");
		return EFFECT_ERROR;
	}

	log("Haptic gain: " + std::to_string(hapticGain));

	return hapticGain;
}


// Report SDL_HAPTIC_GAIN_MAX environmental variable
int Wheel::getMaxGain()
{
	log("Getting max gain");
	const char* maxGain = getenv("SDL_HAPTIC_GAIN_MAX");
	if (maxGain == NULL)
	{
		log("Error: SDL_HAPTIC_GAIN_MAX not set");
		return -1;
	}

	int envMaxGain;
	std::stringstream maxGainStr;
	maxGainStr << maxGain;

	log("SDL_HAPTIC_GAIN_MAX = " + maxGainStr.str());
	maxGainStr >> envMaxGain;

	return envMaxGain;
}


// https://stackoverflow.com/questions/17258029/c-setenv-undefined-identifier-in-visual-studio
// Bill Weinman
// Set environmental variable name with value
int Wheel::setenv(const char* name, const char* value, int overwrite)
{
	int errcode = 0;
	if (!overwrite) {
		size_t envsize = 0;
		errcode = getenv_s(&envsize, NULL, 0, name);
		if (errcode || envsize) return errcode;
	}
	return _putenv_s(name, value);
}

// Set max gain SDL_HAPTIC_GAIN_MAX.
bool Wheel::setMaxGain(int gain)
{
	log("Setting MAX gain to: " + std::to_string(gain));

	if (!checkHaptic() || !checkGain(gain))
	{
		log("Error: MAX Gain not set");
		return false;
	}

	std::string s = std::to_string(gain);
	MAXIMUM_GAIN = s.c_str();
	int result = setenv("SDL_HAPTIC_GAIN_MAX", MAXIMUM_GAIN, 1);

	if (result != 0)
	{
		log("Error: Cant set environment variable SDL_HAPTIC_GAIN_MAX");
		return false;
	}

	return true;
}

// Destroy current effect if exists
void Wheel::destroyEffect(unsigned int effect)
{
	if (effectsMap[effect] != EFFECT_ERROR && checkEffectNumber(effect))
	{
		log("Destroying effect: " + effectsName[effect] + " with effect ID: " + std::to_string(effectsMap[effect]));
		SDL_HapticDestroyEffect(haptic, effectsMap[effect]);
		effectsMap[effect] = EFFECT_ERROR;
		return;
	}

	if (!checkEffectNumber(effect)) log("Error: Cant destroy effect (" + effectsName[effect] + ")");
}

// Upload effect to haptic controller
int Wheel::uploadEffect()
{
	log("Uploading effect");

	// Upload the effect
	return SDL_HapticNewEffect(haptic, &effect);
}

bool Wheel::checkParamsConstant(Uint32 mS, Uint16 lvl)
{
	// Sanity Checks
	if (!checkHaptic() || !hasConstant()) return false;
	if (!checkDuration(mS) || !checkLevel(lvl)) return false;
	return true;
}

// Create force left
bool Wheel::setLeft(Uint32 mS, Uint16 lvl)
{
	if (!checkParamsConstant(mS, lvl))
	{
		destroyEffect(LEFT);
		return false;
	}
	return setConstantForce(mS, lvl, LEFT, DEFAULT_DELAY, DEFAULT_ATTACK_TIME, DEFAULT_ATTACK_LVL, DEFAULT_FADE_TIME, DEFAULT_FADE_LVL);
}

// Create force right
bool Wheel::setRight(Uint32 mS, Uint16 lvl)
{
	if (!checkParamsConstant(mS, lvl))
	{
		destroyEffect(RIGHT);
		return false;
	}
	return setConstantForce(mS, lvl, RIGHT, DEFAULT_DELAY, DEFAULT_ATTACK_TIME, DEFAULT_ATTACK_LVL, DEFAULT_FADE_TIME, DEFAULT_FADE_LVL);
}

// Check effect envelope parameters
bool Wheel::checkParamsEnvelope(Uint32 mS, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl)
{
	if (!checkDelay(dly) || !checkAttackLength(aLen)) return false;
	if (!checkFadeLength(fLen) || !checkAttackLevel(aLvl)) return false;
	if (!checkFadeLevel(fLvl) || !checkEnvelopeTime(mS, aLen, fLen)) return false;
	return true;
}

// Create force left
bool Wheel::setLeftWithEnv(Uint32 mS, Uint16 lvl, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl)
{
	bool ok = true;
	if (!checkParamsConstant(mS, lvl)) ok = false;
	if (!checkParamsEnvelope(mS, dly, aLen, aLvl, fLen, fLvl)) ok = false;
	if (!ok)
	{
		destroyEffect(LEFT);
		return false;
	}
	return setConstantForce(mS, lvl, LEFT, dly, aLen, aLvl, fLen, fLvl);
}

// Create force right
bool Wheel::setRightWithEnv(Uint32 mS, Uint16 lvl, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl)
{
	bool ok = true;
	if (!checkParamsConstant(mS, lvl)) ok = false;
	if (!checkParamsEnvelope(mS, dly, aLen, aLvl, fLen, fLvl)) ok = false;
	if (!ok)
	{
		destroyEffect(RIGHT);
		return false;
	}
	return setConstantForce(mS, lvl, RIGHT, dly, aLen, aLvl, fLen, fLvl);
}

/* Define SDL_HAPTIC_CONSTANT force
	Returns effect id (negative is and error)
	Checks input parameters for bounds
	Outputs to console errors if found */
bool Wheel::setConstantForce(Uint32 mS, Uint16 lvl, int dir, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl)
{
	destroyEffect(dir);

	log("Setting up Constant Force Effect");

	if (!hasConstant())
	{
		log("Error: Does not have constant ability");
		return false;
	}

	resetEffect();

	// SDL_HAPTIC_CONSTANT;
	effect.type = SDL_HAPTIC_CONSTANT;
	effect.constant.direction.type = DIRECTION_TYPE;
	effect.constant.direction.dir[0] = (dir == LEFT ? 1 : -1);
	effect.constant.direction.dir[1] = 0;
	effect.constant.direction.dir[2] = 0;
	effect.constant.length = mS;
	effect.constant.delay = dly;
	effect.constant.level = scaleLevel(lvl);
	effect.constant.attack_length = aLen;
	effect.constant.attack_level = scaleLevel(aLvl);
	effect.constant.fade_length = fLen;
	effect.constant.fade_level = scaleLevel(fLvl);

	int effect_id = uploadEffect();

	// error?
	if (effect_id < 0)
	{
		effectsMap[dir] = EFFECT_ERROR;
		log("Error: (setConstantForce) " + std::string(SDL_GetError()));
		return false;
	}

	// Update map with effect ID
	effectsMap[dir] = effect_id;

	return true;

} // end setConstantForce

// Check for valid direction
bool Wheel::checkDirection(unsigned int dir)
{
	if (dir != LEFT && dir != RIGHT && dir != UP && dir != DOWN)
	{
		log("Error: Bad direction");
		return false;
	}
	return true;
}

// Check for valid type
bool Wheel::checkPeriodType(unsigned int type)
{
	if (type != SINE && type != TRIANGLE && type != SAWDOWN && type != SAWUP)
	{
		log("Error: Period type not known");
		return false;
	}
	return true;
}

// Check for valid type
bool Wheel::checkConditionType(unsigned int type)
{
	if (type != SPRING && type != DAMPER && type != INERTIA && type != FRICTION)
	{
		log("Error: Condition type not known");
		return false;
	}
	return true;
}

// Check for valid type
bool Wheel::checkRampType(unsigned int type)
{
	if (type != RAMP_RIGHT && type != RAMP_LEFT)
	{
		log("Error: Ramp type not known");
		return false;
	}
	return true;
}

bool Wheel::checkParamsPeriod(Uint32 mS, Uint16 lvl, Uint32 period, int dir, unsigned int type)
{
	// Sanity Checks
	bool ok = true;
	switch (type)
	{
	case SINE:
		if (!hasSine()) ok = false;
		break;
	case TRIANGLE:
		if (!hasTriangle()) ok = false;
		break;
	case SAWUP:
		if (!hasSawUp()) ok = false;
		break;
	case SAWDOWN:
		if (!hasSawDown()) ok = false;
		break;
	default:
		ok = false;
	}

	if (!ok) log("Error: Haptic ability not available (acheckParamsPeriod)");

	if (!checkHaptic() || !ok) return false;
	if (!checkDuration(mS) || !checkLevel(lvl)) return false;
	if (!checkDirection(dir) || !checkPeriodType(type)) return false;
	return true;
}

bool Wheel::setSine(Uint32 mS, Uint32 period, Uint16 lvl, int dir)
{
	bool ok = true;
	if (!checkParamsPeriod(mS, lvl, period, dir, SINE)) ok = false;
	if (!ok)
	{
		destroyEffect(SINE);
		return false;
	}
	return setPeriod(SINE, mS, period, lvl / 2, 0, lvl, dir, 0, 0, 0, 0, 0);
}

bool Wheel::setTriangle(Uint32 mS, Uint32 period, Uint16 lvl, int dir)
{
	bool ok = true;
	if (!checkParamsPeriod(mS, lvl, period, dir, TRIANGLE)) ok = false;
	if (!ok)
	{
		destroyEffect(TRIANGLE);
		return false;
	}
	return setPeriod(TRIANGLE, mS, period, lvl / 2, 0, lvl, dir, 0, 0, 0, 0, 0);
}

bool Wheel::setSawUp(Uint32 mS, Uint32 period, Uint16 lvl, int dir)
{
	bool ok = true;
	if (!checkParamsPeriod(mS, lvl, period, dir, SAWUP)) ok = false;
	if (!ok)
	{
		destroyEffect(SAWUP);
		return false;
	}
	return setPeriod(SAWUP, mS, period, 0, 0, lvl, dir, 0, 0, 0, 0, 0);
}

bool Wheel::setSawDown(Uint32 mS, Uint32 period, Uint16 lvl, int dir)
{
	bool ok = true;
	if (!checkParamsPeriod(mS, lvl, period, dir, SAWDOWN)) ok = false;
	if (!ok)
	{
		destroyEffect(SAWDOWN);
		return false;
	}
	return setPeriod(SAWDOWN, mS, period, 0, 0, lvl, dir, 0, 0, 0, 0, 0);
}

void Wheel::setDir(int dir, int& left_right, int& up_down)
{
	switch (dir)
	{
	case LEFT:
		left_right = 1;
		up_down = 0;
		break;
	case RIGHT:
		left_right = -1;
		up_down = 0;
		break;
	case UP:
		left_right = 0;
		up_down = 1;
		break;
	case DOWN:
		left_right = 0;
		up_down = -1;
		break;
	default:
		left_right = 0;
		up_down = 0;
	}
}

// Convert Andy type into SDL period type 
void Wheel::setPeriodType(int type, int& sdl_type)
{
	switch (type)
	{
	case SINE:
		sdl_type = SDL_HAPTIC_SINE;
		break;
	case TRIANGLE:
		sdl_type = SDL_HAPTIC_TRIANGLE;
		break;
	case SAWUP:
		sdl_type = SDL_HAPTIC_SAWTOOTHUP;
		break;
	case SAWDOWN:
		sdl_type = SDL_HAPTIC_SAWTOOTHUP;
		break;
	default:
		sdl_type = SDL_HAPTIC_SINE;
	}
}

// Convert Andy types into SDL condition type
void Wheel::setConditionType(int type, int& sdl_type)
{
	switch (type)
	{
	case SPRING:
		sdl_type = SDL_HAPTIC_SPRING;
		break;
	case DAMPER:
		sdl_type = SDL_HAPTIC_DAMPER;
		break;
	case INERTIA:
		sdl_type = SDL_HAPTIC_INERTIA;
		break;
	case FRICTION:
		sdl_type = SDL_HAPTIC_FRICTION;
		break;
	default:
		sdl_type = SDL_HAPTIC_SPRING;
	}
}

/* Define SDL_HAPTIC_SINE SAW and TRIANGLE force
	Returns effect id (negative is and error)
	Checks input parameters for bounds
	Outputs to console errors if found */
bool Wheel::setPeriod(unsigned int type, Uint32 mS, Uint32 period, Sint16 offset, Uint16 phase, Uint16 lvl, int dir, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl)
{
	destroyEffect(type);

	log("Setting up " + std::string(effectsName[type]) + " Effect");

	int left_right, up_down, sdl_type;
	setDir(dir, left_right, up_down);
	setPeriodType(type, sdl_type);

	resetEffect();

	// TODO cludge - offset needs a value for LEFT or RIGHT to work
	// but UP and DOWN don't
	if (dir == UP || dir == DOWN) offset = 0;

	// SDL_HAPTIC_TRIANGLE SDL_HAPTIC_SINE SDL_HAPTIC_SAWTOOTHUP
	effect.type = sdl_type;
	effect.periodic.direction.type = DIRECTION_TYPE;
	effect.periodic.direction.dir[0] = left_right;
	effect.periodic.direction.dir[1] = up_down;
	effect.periodic.direction.dir[2] = 0;
	effect.periodic.length = mS;
	effect.periodic.delay = dly;
	effect.periodic.period = period; //  ms
	effect.periodic.magnitude = scaleLevel(lvl); // lvl of 32767 strength
	effect.periodic.offset = scaleLevel(offset);
	effect.periodic.phase = phase;
	effect.periodic.attack_length = aLen;
	effect.periodic.attack_level = scaleLevel(aLvl);
	effect.periodic.fade_length = fLen;
	effect.periodic.fade_level = scaleLevel(fLvl);

	int effect_id = uploadEffect();

	// error?
	if (effect_id < 0)
	{
		effectsMap[type] = EFFECT_ERROR;
		log("Error: (setPeriod) " + std::string(SDL_GetError()));
		return false;
	}

	// Update map with effect ID
	effectsMap[type] = effect_id;

	return true;

} // end setPeriod

bool Wheel::checkParamsCondition(unsigned int type, Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre)
{
	// Sanity Checks
	bool ok = true;
	switch (type)
	{
	case SPRING:
		if (!hasSpring()) ok = false;
		break;
	case DAMPER:
		if (!hasDamper()) ok = false;
		break;
	case INERTIA:
		if (!hasInertia()) ok = false;
		break;
	case FRICTION:
		if (!hasFriction()) ok = false;
		break;
	default:
		ok = false;
	}

	if (!ok) log("Error: Condition ability not available");

	if (!checkHaptic() || !ok) return false;
	if (!checkDuration(mS) || !checkDelay(dly)) return false;
	if (!checkSatLevel(rSat) || !checkConditionType(type)) return false;
	if (!checkSatLevel(lSat) || !checkCoEfLevel(lCo) || !checkCoEfLevel(rCo)) return false;
	if (!checkDeadband(dead) || !checkCentre(centre)) return false;
	return true;
}

bool Wheel::setSpring(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre)
{
	bool ok = true;
	if (!checkParamsCondition(SPRING, mS, dly, rSat, lSat, rCo, lCo, dead, centre)) ok = false;
	if (!ok)
	{
		destroyEffect(SPRING);
		return false;
	}
	return setCondition(SPRING, mS, dly, rSat, lSat, rCo, lCo, dead, centre);
}

bool Wheel::setDamper(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre)
{
	bool ok = true;
	if (!checkParamsCondition(DAMPER, mS, dly, rSat, lSat, rCo, lCo, dead, centre)) ok = false;
	if (!ok)
	{
		destroyEffect(DAMPER);
		return false;
	}
	return setCondition(DAMPER, mS, dly, rSat, lSat, rCo, lCo, dead, centre);
}

bool Wheel::setInertia(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre)
{
	bool ok = true;
	if (!checkParamsCondition(INERTIA, mS, dly, rSat, lSat, rCo, lCo, dead, centre)) ok = false;
	if (!ok)
	{
		destroyEffect(INERTIA);
		return false;
	}
	return setCondition(INERTIA, mS, dly, rSat, lSat, rCo, lCo, dead, centre);
}

bool Wheel::setFriction(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre)
{
	bool ok = true;
	if (!checkParamsCondition(FRICTION, mS, dly, rSat, lSat, rCo, lCo, dead, centre)) ok = false;
	if (!ok)
	{
		destroyEffect(FRICTION);
		return false;
	}
	return setCondition(FRICTION, mS, dly, rSat, lSat, rCo, lCo, dead, centre);
}

/* Define SDL_HAPTIC_FRICTION INERTIA and DAMPER force
	Returns effect id (negative is and error)
	Checks input parameters for bounds
	Outputs to console errors if found
	*** Leo Bodnar's SimSteering2 doesn't support DAMPER/FRICTION and INERTIA
	*** It does SPRING
	*/
bool Wheel::setCondition(unsigned int type, Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre)
{
	destroyEffect(type);

	log("Setting up " + std::string(effectsName[type]) + " Effect");

	int sdl_type;
	setConditionType(type, sdl_type);

	// SDL_HAPTIC_DAMPER FRICTION INERTIA and SPRING
	effect.type = sdl_type;
	effect.condition.direction.type = DIRECTION_TYPE;
	effect.condition.direction.dir[0] = 0;
	effect.condition.direction.dir[1] = 0;
	effect.condition.direction.dir[2] = 0;
	effect.condition.length = mS;
	effect.condition.delay = dly;
	// Same for all 3 axis
	for (int axis = 0; axis < 3; axis++)
	{
		effect.condition.right_sat[axis] = scaleLevel(rSat);
		effect.condition.left_sat[axis] = scaleLevel(lSat);
		effect.condition.right_coeff[axis] = scaleLevel(rCo);
		effect.condition.left_coeff[axis] = scaleLevel(lCo);
		effect.condition.deadband[axis] = dead;
		effect.condition.center[axis] = centre;
	}

	int effect_id = uploadEffect();

	// error?
	if (effect_id < 0)
	{
		effectsMap[type] = EFFECT_ERROR;
		log("Error: (setPeriod) " + std::string(SDL_GetError()));
		return false;
	}

	// Update map with effect ID
	effectsMap[type] = effect_id;

	return true;

} // end setPeriod

// Set Ramp left effect
bool Wheel::setRampLeft(Uint32 mS, Sint16 start, Sint16 end)
{
	if (mS == FOREVER)
	{
		log("Erro: Ramp duration can not be FOREVER");
		return false;
	}

	if (!checkDuration(mS)) return false;
	if (!checkStartLevel(start) || !checkEndLevel(end)) return false;

	return setRampForce(mS, LEFT, 0, start, end, 0, 0, 0, 0, RAMP_LEFT);
}

// Set Ramp right effect
bool Wheel::setRampRight(Uint32 mS, Sint16 start, Sint16 end)
{
	if (mS == FOREVER)
	{
		log("Erro: Ramp duration can not be FOREVER");
		return false;
	}

	if (!checkDuration(mS)) return false;
	if (!checkStartLevel(start) || !checkEndLevel(end)) return false;

	return setRampForce(mS, RIGHT, 0, start, end, 0, 0, 0, 0, RAMP_RIGHT);
}

// Set Ramp Effect
bool Wheel::setRampForce(Uint32 mS, int dir, Uint32 dly, Sint16 start, Sint16 end, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl, int type)
{
	if (!checkRampType(type)) return false;

	destroyEffect(type);

	log("Setting up Ramp Effect");

	if (!hasRamp())
	{
		log("Error: Does not have ramp ability");
		return false;
	}

	int left_right, up_down;
	setDir(dir, left_right, up_down);

	resetEffect();

	// SDL_HAPTIC_CONSTANT;
	effect.type = SDL_HAPTIC_RAMP;
	effect.ramp.direction.type = DIRECTION_TYPE;
	effect.ramp.direction.dir[0] = left_right;
	effect.ramp.direction.dir[1] = up_down;
	effect.ramp.direction.dir[2] = 0;
	effect.ramp.length = mS;
	effect.ramp.delay = dly;
	effect.ramp.start = scaleLevel(start);
	effect.ramp.end = scaleLevel(end);
	effect.ramp.attack_length = aLen;
	effect.ramp.attack_level = scaleLevel(aLvl);
	effect.ramp.fade_length = fLen;
	effect.ramp.fade_level = scaleLevel(fLvl);

	int effect_id = uploadEffect();

	// error?
	if (effect_id < 0)
	{
		effectsMap[type] = EFFECT_ERROR;
		log("Error: (setRampForce) " + std::string(SDL_GetError()));
		return false;
	}

	// Update map with effect ID
	effectsMap[type] = effect_id;

	return true;

} // end setConstantForce


// Wait / pause / delay for number of milli seconds
void Wheel::wait(Uint32 mS)
{
	log("Waiting for " + std::to_string(mS) + " milli Seconds");
	SDL_Delay(mS);
}

// Wait / pause / delay for number of milli seconds
void Wheel::waitNoLog(Uint32 mS)
{
	SDL_Delay(mS);
}

// Run Haptic Effect
// Returns effect_id
// The wheel runs effect
bool Wheel::runEffect(unsigned int effect, Uint32 iterations)
{
	// Sanity Checks
	if (!checkHaptic() || !checkIterations(iterations) || !checkEffectNumber(effect)) return false;

	log("RunEffect Number: " + std::to_string(effect) + " (" + effectsName[effect] + ")");

	if (effectsMap[effect] == EFFECT_ERROR)
	{
		log("Error: (" + effectsName[effect] + ") cant be run");
		return false;
	}

	int r = SDL_HapticRunEffect(haptic, effectsMap[effect], iterations);
	if (r < 0) log("Error: " + std::string(SDL_GetError()));
	return (r == 0 ? true : false);
}

Sint16 Wheel::findLeftLock()
{
	Sint16 ll = SDL_MAX_SINT16;
	Uint16 level = L10;

	if (isEffectRunning(DAMPER) || isEffectRunning(INERTIA) || isEffectRunning(FRICTION) || isEffectRunning(SPRING)) level = 32000;

	if (!setLeft(6000, level))
	{
		log("Error: failed to set LEFT effect");
		return false;
	}

	if (!runEffect(LEFT))
	{
		log("Error: failed to run LEFT effect");
		return false;
	}

	// Should get to end within 3 seconds
	wait(4500);
	clock_t now = clock();

	// take samples for 1 second ish
	while ((clock() - now) < 1000)
	{
		Sint16 pos = getPosition();
		if (pos == 0)
		{
			log("Error: Cant get position");
			return false;
		}
		if (pos < ll) ll = pos;
	}

	log("Left lock: " + std::to_string(ll));

	stopEffect(LEFT);

	return ll;
}

Sint16 Wheel::findRightLock()
{
	Sint16 rl = SDL_MIN_SINT16;
	Uint16 level = L10;

	if (isEffectRunning(DAMPER) || isEffectRunning(INERTIA) || isEffectRunning(FRICTION) || isEffectRunning(SPRING)) level = 32000;

	if (!setRight(6000, level))
	{
		log("Error: failed to set RIGHT effect");
		return false;
	}

	if (!runEffect(RIGHT))
	{
		log("Error: failed to run RIGHT effect");
		return false;
	}

	// Should get to end within 3 seconds
	wait(4500);
	clock_t now = clock();

	// take samples for 1 second ish
	while ((clock() - now) < 1000)
	{
		Sint16 pos = getPosition();
		if (pos == 0)
		{
			log("Error: Cant get position");
			return false;
		}
		if (pos > rl) rl = pos;
	}

	log("Right lock: " + std::to_string(rl));

	stopEffect(RIGHT);

	return rl;
}

// TODO overcome Damper and Spring etc
// TODO return false if failed. What constitutes a failure here?
bool Wheel::calibrate()
{
	bool ok = true;

	// Get near centre
	Uint16 level = L20;

	if (isEffectRunning(DAMPER) || isEffectRunning(INERTIA) || isEffectRunning(FRICTION) || isEffectRunning(SPRING)) level = L32;

	gotoAngle(0, level);

	// find left lock
	leftLock = findLeftLock();

	gotoAngle(0, level);
	waitNoLog(1000);

	// find right lock
	rightLock = findRightLock();

	// find centre
	centre = ((leftLock + rightLock) / 2) + OFFSET;
	log("Centre point: " + std::to_string(centre));

	waitNoLog(1000);

	// get jitter
	findJitter();

	gotoAngle(0, level);

	return true;
}


bool Wheel::gotoAngleSlow(Sint16 angle)
{
	return gotoAngle(angle, SLOW);
}

bool Wheel::gotoAngleFast(Sint16 angle)
{
	return gotoAngle(angle, FAST);
}

bool Wheel::gotoAngleFullSpeed(Sint16 angle)
{
	return gotoAngle(angle, FULL);
}

bool Wheel::gotoAngle(Sint16 angle, Uint16 level)
{
	log("Going to angle: " + std::to_string(angle));

	// Sanity checks
	if (getAngle() == angle)
	{
		log("Wanted: " + std::to_string(angle) + " Got to angle: " + std::to_string(getAngle()));
		return true;
	}
	if (std::abs(angle) > DEGREES / 2)
	{
		log("Error: Bad angle");
		return false;
	}

	stopEffect(DAMPER);
	stopEffect(FRICTION);
	stopEffect(INERTIA);
	stopEffect(SPRING);
	setDamper(FOREVER, 0, FULL, FULL, FULL, FULL);

	int near = 10;
	if (level < 10000) near = 5;
	if (level > 15000) near = 15;


	// Start moving in correct direction
	int direction = LEFT;
	setLeft(FOREVER, level);
	if (angle > getAngle())
	{
		direction = RIGHT;
		setRight(FOREVER, level);
		runEffect(RIGHT);
	}
	else
	{
		runEffect(LEFT);
	}

	// Are we there yet?
	bool there = false;
	int now = 0;
	while (!there)
	{
		now = getAngle();
		if (direction == LEFT)
		{
			if (now <= angle + near && !isEffectRunning(DAMPER)) runEffect(DAMPER);
			if (now <= angle) there = true;
		}
		else
		{
			if (now >= angle - near && !isEffectRunning(DAMPER)) runEffect(DAMPER);
			if (now >= angle) there = true;
		}
		//waitNoLog(10);
	}
	if (direction == LEFT) stopEffect(LEFT); else stopEffect(RIGHT);
	waitNoLog(200);
	stopEffect(DAMPER);

	log("Wanted: " + std::to_string(angle) + " Got to angle: " + std::to_string(getAngle()));
	if (angle != getAngle()) return false;
	return true;
}

Sint16 Wheel::findJitter()
{
	log("Finding jitter...");
	int min, max;
	int max_jitter = 0;
	int j;

	srand((unsigned int)time(0));

	// Get 10 random angles
	for (int k = 0; k < 10; ++k)
	{
		// random angle
		int rn = (rand() % ((DEGREES - 2) / 4) + 1);
		int rd = (rand() % 2) + 1;
		if (rd == 2) rn = -rn;

		gotoAngle(rn);

		// Settle
		waitNoLog(500);

		// Reset
		min = SDL_MAX_SINT16;
		max = SDL_MIN_SINT16;
		j = 0;

		// get 100 readings
		for (int i = 0; i < 100; ++i)
		{
			int pos = std::abs(getPosition());
			if (pos < min) min = pos;
			if (pos > max) max = pos;
			waitNoLog(10);
		}

		// Max so far
		j = max - min;
		if (j > max_jitter) max_jitter = j;

		log("Intermediate Jitter: " + std::to_string(j));

		waitNoLog(100);
	}

	// found jitter
	if (max_jitter > jitter) jitter = max_jitter;
	log("Jitter: " + std::to_string(max_jitter));
	return jitter;
}

Sint16 Wheel::getJitter()
{
	log("Jitter: " + std::to_string(jitter));
	return jitter;
}

Sint16 Wheel::getLeftLock()
{
	log("Left Lock: " + std::to_string(leftLock));
	return leftLock;
}

Sint16 Wheel::getRightLock()
{
	log("Right Lock: " + std::to_string(rightLock));
	return rightLock;
}

Sint16 Wheel::getCentre()
{
	log("Centre: " + std::to_string(centre));
	return centre;
}

// Get distance travelled in time mS
Sint16 Wheel::getDistance(Uint32 time)
{
	using namespace std::chrono;
	int pos2;
	int pos1 = getPosition();
	duration<double, std::milli> timeSpan;
	high_resolution_clock::time_point startPoint = high_resolution_clock::now();

	bool done = false;
	while (!done)
	{
		high_resolution_clock::time_point endPoint = high_resolution_clock::now();
		timeSpan = endPoint - startPoint;
		if (timeSpan.count() >= time)
		{
			pos2 = getPosition();
			done = true;
		}
	}

	Sint16 dist = pos2 - pos1;
	log("Distance travelled in: " + std::to_string(timeSpan.count()) + " mS was " + std::to_string(dist) + " units");

	return dist;
}

// Is the wheel stationary?
bool Wheel::isStationary()
{

	for (int i = 0; i <= STATIONARY_TESTS; ++i)
	{
		int p1 = getPosition();
		wait(10);
		int p2 = getPosition();
		if (!(std::abs(p2 - p1) < std::abs(jitter + JITTER_MARGIN))) return false;
	}

	log("Wheel is stationary within: " + std::to_string(jitter + JITTER_MARGIN) + " count");

	return true;

}

// Convert level into force value taking scaling into account
double Wheel::convertLevelToForce(Uint16 lvl)
{
	double max = getMaxGain();
	if (max == EFFECT_ERROR) max = 100.0;

	double gain = getGain();
	if (gain == EFFECT_ERROR) gain = 100.0;

	if (gain == 0 || max == 0)
	{
		log("Error: Force is: 0.0 Nm");
		return 0.0f;
	}

	double level = lvl * (max / 100.0) * (gain / 100.0);
	double ratio = level / MAX;
	log("Level: " + std::to_string(lvl) + " converts to force: " + std::to_string(ratio * RATED_HAPTIC_FORCE) + " Nm");
	return (double)(ratio * RATED_HAPTIC_FORCE);
}

// Convert force into level taking scaling into account
Uint16 Wheel::convertForceToLevel(float force)
{
	if (RATED_HAPTIC_FORCE <= 0)
	{
		log("Error: RATED_HAPTIC_FORCE is too small. Zero force returned.");
		return 0;
	}

	double ratio = force / RATED_HAPTIC_FORCE;
	if (ratio > 1)
	{
		log("Error: Force supplied is greater than wheel's haptic ability. MAX force will be used.");
		return MAX;
	}

	double level = MAX * ratio;

	// apply scaling
	double max = getMaxGain();
	if (max == EFFECT_ERROR) max = 100.0;

	double gain = getGain();
	if (gain == EFFECT_ERROR) gain = 100.0;

	if (gain == 0 || max == 0)
	{
		log("Error: Level is 0 due to gain and maxGain settings");
		return 0;
	}

	level = level * (1 / (gain / 100)) * (1 / (max / 100));
	if (level > MAX)
	{
		level = MAX;
		log("Error: After scaling level: " + std::to_string(level) + " was bigger than MAX. Setting to MAX");
	}
	if (level <= 0)
	{
		level = 0;
		log("Error: After scaling level was smaller than 0. Setting to 0");
	}

	log("Force: " + std::to_string(force) + " Nm converts to level: " + std::to_string((Uint16)level));
	return (Uint16)level;
}

// Profile effect levels
void Wheel::profile()
{
	log("Profiling effect levels...");

	profileD(RIGHT);
	profileD(LEFT);

	// Show results
	for (int lvl = 0; lvl < 33; ++lvl)
	{
		log("Profile level 10mS move count: " + std::to_string(lvl) + " Right: " + std::to_string(effectLevelsRight[lvl]) + " Left: " + std::to_string(effectLevelsLeft[lvl]));
	}

	gotoAngle(0);
}

// get effect level profile of dir
void Wheel::profileD(int dir)
{
	// Loop thru all effect levels
	for (int lvl = 0; lvl < 33; ++lvl)
	{
		// Reset position
		gotoAngle(0);

		// Setup force
		setRight(FOREVER, (lvl * 1000));
		setLeft(FOREVER, (lvl * 1000));
		if (dir == LEFT) runEffect(LEFT); else runEffect(RIGHT);

		// Wait for force to build up momentum
		wait(150);

		// Take 20 readings of how far travelled in 10 mS
		int average = 0;
		for (int i = 0; i < 20; ++i) average += std::abs(getDistance(10));

		// Store result
		int result = average / 20;
		if (dir == LEFT) effectLevelsLeft[lvl] = result; else effectLevelsRight[lvl] = result;
		log("Profile level 10mS move count: " + std::to_string(lvl) + " = " + std::to_string(result));

		stopEffect(dir == LEFT ? LEFT : RIGHT);

		// Apply break
		if (lvl > 10)
		{
			runEffect(dir == LEFT ? RIGHT : LEFT);
			if (lvl > 13) wait(30); else wait(20);
			stopEffect(dir == LEFT ? RIGHT : LEFT);
		}

		while (!isStationary()) wait(50);
	}
}

// Get the distance travelled over 10mS and find the nearest level that would achieve the same
Uint16 Wheel::getClosestEffectLevel(int distance, int dir)
{
	Uint16 l = 0;
	Uint16 closest = SDL_MAX_UINT16;
	for (Uint16 lvl = 0; lvl < 33; ++lvl)
	{
		Uint16 level = dir == LEFT ? effectLevelsLeft[lvl] : effectLevelsRight[lvl];
		if (std::abs(distance) >= level) l = lvl;
	}

	log("Closes effect level for distance: " + std::to_string(distance) + " is " + std::to_string(l));

	return l * 1000;
}