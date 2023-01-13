#pragma once

#include "OptitrackData.hpp"
#include "KeyboardAdapter.h"
#include "InputSender.h"
#include "InputParser.hpp"

enum class State {
    IDLE,
    KEYBOARD,
    CURSOR_MOVE
};

class ClavPad {
public:
    // Constructor
    ClavPad();
    ~ClavPad();

    void run();

private:
    void parseInput(); // TODO
    void sendAction(); // TODO

    OptitrackData m_data;

    KeyboardAdapter m_keyboard_adapter;

    InputParser m_input_parser;
    InputSender m_input_sender;

    State m_state;
};


