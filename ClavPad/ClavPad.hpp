#pragma once

#include <chrono>
#include <vector>

#include "OptitrackData.hpp"
#include "KeyboardAdapter.h"
#include "InputSender.h"

enum class State 
{
    IDLE = 0,
    KEYBOARD,
    CURSOR_MOVE,
    CURSOR_CLICK,
    MOUSE
};

class ClavPad {
public:
    // Constructor
    ClavPad();
    ~ClavPad();

    void calibrate();

    void run();

    void printActivity() const;

private:
    void parseInput(); 
    bool kbhit() const;

    void updateActivity(State old_state);
    bool gotoCursorMoveState(const glm::vec3& p1, const glm::vec3& p2);
    bool gotoCursorClickState(const glm::vec3& p1, const glm::vec3& p2);
    bool gotoKeyboardState();

    OptitrackData m_data;

    KeyboardAdapter* m_keyboard_adapter = nullptr;

    InputSender m_input_sender;

    State m_state = State::IDLE;
    int m_state_frame_count = 0;

    // parameters
    float m_finger_distance_threshold = 0.0;
    float m_cursor_state_height_threshold = 0.0;

    // time profiling
    std::vector<std::pair<State, std::chrono::seconds>> m_activity;
    std::chrono::time_point<std::chrono::system_clock> m_last_point;
};


