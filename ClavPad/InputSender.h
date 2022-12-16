#pragma once

#include <windows.h>
#include <winuser.h>
#include "wtypes.h"

class InputSender
{
public:
	InputSender();

	void SendMousePos(int x, int y);
	void SendMouseMove(int dx, int dy);

	void SendClick();

	void SendWheelMove(int d);

private:
	INPUT m_input;
	int m_width, m_height;
	
	void Setup(INPUT* input);
};