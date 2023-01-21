#pragma once

#include <chrono>
#include <vector>
#include <deque>
#include <string>

#include "OptitrackData.hpp"
#include "KeyboardAdapter.h"
#include "InputSender.h"

enum class State 
{
    IDLE = 0,
    KEYBOARD,
    CURSOR_MOVE,
    CURSOR_CLICK,
    MOUSE_MOVE,
    MOUSE_CLICK
};

class ClavPad {
public:
    // Constructor
    ClavPad(bool mouse_mode = false);
    ~ClavPad();

    void calibrate();

    void run();

    void printActivity() const;
    void saveActivity(const std::string& path);

private:
    void update();

    void parseInput(); 
    bool handleKeyBoardInput();

    bool updateState(State new_state);

    OptitrackData m_data;
    bool m_mouse_mode;
    KeyboardAdapter* m_keyboard_adapter = nullptr;
    InputSender m_input_sender;

    State m_state = State::IDLE;
    int m_state_frame_count = 0;
    std::chrono::time_point<std::chrono::system_clock> m_idle_start;
    bool m_should_stop = false;

    std::deque<float> m_vertical_accelerations;
    float m_last_vertical_velocity = 0.0;
    float m_last_heights = 0.0;

    POINT m_last_mouse_pos = { 0, 0 };

    // parameters
    float m_finger_distance_threshold = 0.0;
    float m_acc_threshold = 0.0;

    // time profiling
    std::vector<std::pair<State, std::chrono::milliseconds>> m_activity;
    std::chrono::time_point<std::chrono::system_clock> m_last_activity_start;
};


