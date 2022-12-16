#include "InputSender.h"

#include <cmath>
#include <iostream>

int main()
{
	InputSender sender;
	while (true) {
		sender.SendWheelMove(-5);
		Sleep(100);
	}
	return 0;
}