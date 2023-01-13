#include "InputSender.h"

#include <cmath>
#include <iostream>

#include "ClavPad.hpp"

int main()
{
	/*InputSender sender;
	while (true) {
		sender.SendWheelMove(-5);
		Sleep(100);
	}
	return 0;*/

	ClavPad clav;
	clav.run();
}