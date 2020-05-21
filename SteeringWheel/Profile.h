#pragma once

namespace HE
{


	// Object holds profiling details
	class Profile
	{
	public:
		int direction; // 1 means right, -1 means left
		int reading; // reading number
		double delta; // rate of change
		int from; // postion
		int to; // position
		int power; // level of effect
		int distance; // how far moves
	};

}

