#include <cmath>
#include <iostream>

#include "InputSender.h"
#include "ClavPad.hpp"

                 
int main()
{
	ClavPad clav(false);
	clav.calibrate();

	clav.run();
	clav.saveActivity("C:\\Local_documents\\clavpad-project\\experiments\\activity.json");
}