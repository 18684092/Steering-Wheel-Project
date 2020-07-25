#pragma once

#pragma warning(disable : 4996) // TODO supress localtime warnings (not thread safe) getTimeStr()

/*
Author: Andy Perrett
Email: andy@wired-wrong.co.uk

Version 0.1

*/

#include <string>
#include <iostream>
#include <SDL.h>
#include <map>
#include <ctime> // used in getTimeStr()
#include <chrono> // used in getTimeStr()
#include <ratio>
#include <cstdlib> // random number
//#include <SDL_stdinc.h> // setMaxGain()
#include <sstream> // getMaxGain()


/*
   Consider using setMaxGain() and setGain() for scaling
   since different wheels have different forces
*/


//const float force_scale = 0.2; // for SIM STEER
const float FORCE_SCALE = 1.0f; // for G27 - leave at one and use setGain() setMaxGain()

constexpr Sint16 DEGREES = 900;
constexpr auto OFFSET = 0;
constexpr auto JITTER_MARGIN = 5;
constexpr auto STATIONARY_TESTS = 2;
constexpr auto RATED_HAPTIC_FORCE = 1.6; // in Newton metres

// G27 10mS move counts for each effect level
constexpr auto M1 = 0, M2 = 0, M3 = 0, M4 = 0, M5 = 0;
constexpr auto M6 = 0, M7 = 34, M8 = 63, M9 = 85, M10 = 126;
constexpr auto M11 = 166, M12 = 197, M13 = 219, M14 = 223, M15 = 225;
constexpr auto M16 = 226, M17 = 235, M18 = 247, M19 = 256, M20 = 269;
constexpr auto M21 = 261, M22 = 284, M23 = 264, M24 = 271, M25 = 305;
constexpr auto M26 = 306, M27 = 307, M28 = 308, M29 = 324, M30 = 359;
constexpr auto M31 = 381, M32 = 382;

// Effects
constexpr unsigned int LEFT = 0;
constexpr unsigned int RIGHT = 1;
constexpr unsigned int SINE = 2;
constexpr unsigned int TRIANGLE = 3;
constexpr unsigned int SAWUP = 4;
constexpr unsigned int SAWDOWN = 5;
constexpr unsigned int SPRING = 6;
constexpr unsigned int DAMPER = 7;
constexpr unsigned int INERTIA = 8;
constexpr unsigned int FRICTION = 9;
constexpr unsigned int RAMP_LEFT = 10;
constexpr unsigned int RAMP_RIGHT = 11;
constexpr unsigned int MAX_EFFECT_NUMBER = 11;

constexpr unsigned int UP = 3;
constexpr unsigned int DOWN = 4;

// Speed (Arbitary speeds)
constexpr auto SLOW = 8000;
constexpr auto NORMAL = 12000;
constexpr auto FAST = 22000;
constexpr auto FULL = 32000;
constexpr auto MAX = SDL_MAX_SINT16;

// Levels (Shorthand for lazy people)
constexpr auto L1 = 1000, L2 = 2000, L3 = 3000, L4 = 4000, L5 = 5000;
constexpr auto L6 = 6000, L7 = 7000, L8 = 8000, L9 = 9000, L10 = 10000;
constexpr auto L11 = 11000, L12 = 12000, L13 = 13000, L14 = 14000, L15 = 15000;
constexpr auto L16 = 16000, L17 = 17000, L18 = 18000, L19 = 19000, L20 = 20000;
constexpr auto L21 = 21000, L22 = 22000, L23 = 23000, L24 = 24000, L25 = 25000;
constexpr auto L26 = 26000, L27 = 27000, L28 = 28000, L29 = 29000, L30 = 30000;
constexpr auto L31 = 31000, L32 = 32000;

constexpr auto DEFAULT_ATTACK_LVL = 0;
constexpr auto DEFAULT_FADE_LVL = 0;
constexpr auto MIN_START_LEVEL = SDL_MIN_SINT16;
constexpr auto MAX_START_LEVEL = SDL_MAX_SINT16;
constexpr auto MIN_END_LEVEL = SDL_MIN_SINT16;
constexpr auto MAX_END_LEVEL = SDL_MAX_SINT16;

// Lengths of time
constexpr auto DEFAULT_ATTACK_TIME = 0;
constexpr auto DEFAULT_FADE_TIME = 0;
constexpr auto DEFAULT_DELAY = 0;
constexpr auto FOREVER = SDL_HAPTIC_INFINITY;

// Errors
constexpr auto EFFECT_ERROR = -1;
constexpr auto DEVICE_ERROR = -1;

// Effect min and max
constexpr auto MIN_GAIN = 0;
constexpr auto MAX_GAIN = 100;
constexpr auto MIN_DURATION = 50;
constexpr auto MIN_DELAY = 0;
constexpr auto MIN_ATTACK_LENGTH = 0;
constexpr auto MIN_FADE_LENGTH = 0;
constexpr auto MIN_LEVEL = 0;
constexpr auto MIN_ATTACK_LEVEL = 0;
constexpr auto MIN_FADE_LEVEL = 0;
constexpr auto MIN_SAT_LEVEL = 0;
constexpr auto MIN_COEF_LEVEL = -32768;
constexpr auto MIN_DEADBAND = 0;
constexpr auto MIN_CENTRE = -32768;
constexpr unsigned int MIN_EFFECT_NUMBER = 0;

constexpr auto DIRECTION_TYPE = SDL_HAPTIC_CARTESIAN; // Only Catesian supported

// stuff for log
constexpr auto SCREEN = 1;
constexpr auto TEXT_FILE = 2;
constexpr auto LOG_FILE = "G27_log.txt";

class Wheel
{
private:
	bool debug;
	int deviceNumber;
	bool hasHaptic;
	Sint16 leftLock, rightLock, centre;
	Sint16 jitter;
	char const* MAXIMUM_GAIN; // place holder for env variable
	int hapticGain;

	// init profiles with calibrated levels
	int effectLevelsLeft[33] = { 0, M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M11, M12, M13, M14, M15, M16, M17, M18, M19, M20
	, M21, M22, M23, M24, M25, M26, M27, M28, M29, M30, M31, M32 };
	int effectLevelsRight[33] = { 0, M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M11, M12, M13, M14, M15, M16, M17, M18, M19, M20
	, M21, M22, M23, M24, M25, M26, M27, M28, M29, M30, M31, M32 };

	SDL_Joystick* joy = nullptr;
	SDL_Haptic* haptic = nullptr;
	SDL_HapticEffect effect;

	// Sets hasHaptic variable
	void testHapticAbilitiy();

	bool checkDuration(Uint32 mS);
	bool checkDelay(Uint32 dly);
	bool checkAttackLength(Uint32 aLen);
	bool checkFadeLength(Uint32 fLen);
	bool checkEnvelopeTime(Uint32 mS, Uint32 aLen, Uint32 fLen);
	bool checkIterations(Uint32 iterations);
	bool checkLevel(Uint16 level);
	bool checkFadeLevel(Uint16 fLvl);
	bool checkSatLevel(Uint16 sat);
	bool checkCoEfLevel(Sint16 coef);
	bool checkDeadband(Uint16 dead);
	bool checkCentre(Sint16 centre);
	bool checkAttackLevel(Uint16 aLvl);
	bool checkEffectNumber(unsigned int effect);
	bool checkHaptic();
	bool checkParamsConstant(Uint32 mS, Uint16 lvl);
	bool checkParamsEnvelope(Uint32 mS, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl);
	bool checkParamsPeriod(Uint32 mS, Uint16 lvl, Uint32 period, int dir, unsigned int type);
	bool checkParamsCondition(unsigned int type, Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre);

	bool checkStartLevel(Sint16 start);
	bool checkEndLevel(Sint16 end);
	bool checkRampType(unsigned int type);

	bool checkDirection(unsigned int dir);
	bool checkPeriodType(unsigned int type);
	bool checkConditionType(unsigned int type);
	bool checkGain(int gain);
	void destroyEffect(unsigned int effect);
	void destroyAllEffects();
	Sint16 findLeftLock();
	Sint16 findRightLock();
	Sint16 findJitter();
	void setDir(int dir, int& left_right, int& up_down);
	void setPeriodType(int type, int& sdl_type);
	void setConditionType(int type, int& sdl_type);

	bool setConstantForce(Uint32 mS, Uint16 lvl, int dir, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl);
	bool setPeriod(unsigned int type, Uint32 mS, Uint32 period, Sint16 offset, Uint16 phase, Uint16 lvl, int dir, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl);
	bool setCondition(unsigned int type, Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead, Sint16 centre);
	bool setRampForce(Uint32 mS, int dir, Uint32 dly, Sint16 start, Sint16 end, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl, int type);

	int setenv(const char* name, const char* value, int overwrite);
	int uploadEffect();

	Uint16 scaleLevel(Uint16 lvl);
	void profileD(int dir);



	// Store effect ID with effect uploaded to controller
	std::map<unsigned int, int> effectsMap = {
	{ LEFT		, EFFECT_ERROR },
	{ RIGHT		, EFFECT_ERROR },
	{ SINE		, EFFECT_ERROR },
	{ TRIANGLE	, EFFECT_ERROR },
	{ SAWUP		, EFFECT_ERROR },
	{ SAWDOWN	, EFFECT_ERROR },
	{ SPRING	, EFFECT_ERROR },
	{ DAMPER	, EFFECT_ERROR },
	{ INERTIA	, EFFECT_ERROR },
	{ FRICTION	, EFFECT_ERROR },
	{ RAMP_LEFT	, EFFECT_ERROR },
	{ RAMP_RIGHT , EFFECT_ERROR }
	};

	// Store effect ID with its effect name
	std::map<unsigned int, std::string> effectsName = {
	{ LEFT		, "Constant Force Left" },
	{ RIGHT		, "Constant Force Right" },
	{ SINE		, "Sine Wave" },
	{ TRIANGLE	, "Triangle Wave" },
	{ SAWUP		, "Sawtooth Up" },
	{ SAWDOWN	, "Sawtooth Down" },
	{ SPRING	, "Spring Condition" },
	{ DAMPER	, "Damper Condition" },
	{ INERTIA	, "Inertia Condition" },
	{ FRICTION	, "Friction Condition" },
	{ RAMP_LEFT , "Ramp Left" },
	{ RAMP_RIGHT , "Ramp Right" }
	};

public:
	// Constructor / Destructor
	Wheel(const std::string name, bool debug = false);
	~Wheel();

	// Haptic Abilities (bits 0-15)
	bool hasSine();
	bool hasConstant();
	bool hasLeftRight();
	bool hasTriangle();
	bool hasSawUp();
	bool hasSawDown();
	bool hasRamp();
	bool hasSpring();
	bool hasDamper();
	bool hasInertia();
	bool hasFriction();
	bool hasCustom();
	bool canSetGain();
	bool hasAutoCentre();
	bool canGetStatus();
	bool canPause();

	bool hasRumble();

	bool validDevice();
	bool validHaptic();
	int numEffectsPlaying();
	void resetEffect();
	bool setLeft(Uint32 mS, Uint16 lvl);
	bool setRight(Uint32 mS, Uint16 lvl);
	bool setLeftWithEnv(Uint32 mS, Uint16 lvl, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl);
	bool setRightWithEnv(Uint32 mS, Uint16 lvl, Uint32 dly, Uint32 aLen, Uint16 aLvl, Uint32 fLen, Uint16 fLvl);
	bool setSine(Uint32 mS, Uint32 period, Uint16 lvl, int dir);
	bool setTriangle(Uint32 mS, Uint32 period, Uint16 lvl, int dir);
	bool setSawUp(Uint32 mS, Uint32 period, Uint16 lvl, int dir);
	bool setSawDown(Uint32 mS, Uint32 period, Uint16 lvl, int dir);
	bool setSpring(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead = 0, Sint16 centre = 0);
	bool setDamper(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead = 0, Sint16 centre = 0);
	bool setInertia(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead = 0, Sint16 centre = 0);
	bool setFriction(Uint32 mS, Uint32 dly, Uint16 rSat, Uint16 lSat, Sint16 rCo, Sint16 lCo, Uint16 dead = 0, Sint16 centre = 0);
	bool setRampLeft(Uint32 mS, Sint16 start, Sint16 end);
	bool setRampRight(Uint32 mS, Sint16 start, Sint16 end);

	Sint16 getPosition();
	Sint16 getAngle();
	Sint16 calculateAngle(Sint16 position);
	Sint16 calculatePosition(float angle);
	bool stopEffect(int effect);
	bool isEffectRunning(int effect);

	bool calibrate();
	bool gotoAngle(Sint16 angle, Uint16 level = NORMAL);
	bool gotoAngleSlow(Sint16 angle);
	bool gotoAngleFast(Sint16 angle);
	bool gotoAngleFullSpeed(Sint16 angle);

	void wait(Uint32 mS);
	void waitNoLog(Uint32 mS);
	bool runEffect(unsigned int effect, Uint32 iterations = 1);
	Sint16 getJitter();
	Sint16 getLeftLock();
	Sint16 getRightLock();
	Sint16 getCentre();
	int setGain(int gain);
	int getGain();
	bool setMaxGain(int maxGain);
	int getMaxGain();

	void log(std::string msg, int place = SCREEN);

	std::string getTimeStr();
	Sint16 getDistance(Uint32 time = 10);

	bool isStationary();

	double convertLevelToForce(Uint16 lvl);
	Uint16 convertForceToLevel(float force);

	void profile();

	Uint16 getClosestEffectLevel(int distance, int dir = LEFT);


};

