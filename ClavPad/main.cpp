#include <cmath>
#include <iostream>

#include "InputSender.h"
#include "ClavPad.hpp"


int main()
{
	ClavPad clav;
	clav.calibrate();

	clav.run();
	clav.printActivity();
}