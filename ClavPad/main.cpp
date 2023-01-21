#include <cmath>
#include <iostream>

#include "InputSender.h"
#include "ClavPad.hpp"

                 
int main()
{
	ClavPad clav(true);
	clav.calibrate();

	clav.run();
	clav.printActivity();
}