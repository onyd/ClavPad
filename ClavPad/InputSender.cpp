#include "InputSender.h"

InputSender::InputSender()
{
    RECT desktop;

    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    
    m_width = desktop.right;
    m_height = desktop.bottom;
}

void InputSender::Setup(INPUT* m_input)
{
    m_input->type = INPUT_MOUSE;
    m_input->mi.dx = 0;
    m_input->mi.dy = 0;
    m_input->mi.mouseData = 0;
    m_input->mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    m_input->mi.time = 0;
    m_input->mi.dwExtraInfo = 0;
}

void InputSender::SendMousePos(int x, int y)
{
    Setup(&m_input);
    m_input.mi.dx = (x * (0xFFFF / m_width));
    m_input.mi.dy = (y * (0xFFFF / m_height));
    m_input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);

    SendInput(1, &m_input, sizeof(INPUT));
}

void InputSender::SendMouseMove(int dx, int dy)
{
    Setup(&m_input);
    m_input.mi.dx = dx;
    m_input.mi.dy = dy;
    m_input.mi.dwFlags = MOUSEEVENTF_MOVE;

    SendInput(1, &m_input, sizeof(INPUT));
}

void InputSender::SendClick()
{
    Setup(&m_input);
    m_input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
    SendInput(1, &m_input, sizeof(INPUT));

    Sleep(10);

    m_input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
    SendInput(1, &m_input, sizeof(INPUT));
}

void InputSender::SendWheelMove(int d)
{
    Setup(&m_input);
    m_input.mi.mouseData = d;
    m_input.mi.dwFlags = MOUSEEVENTF_WHEEL;

    SendInput(1, &m_input, sizeof(INPUT));
}

