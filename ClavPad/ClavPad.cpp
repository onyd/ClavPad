#include "ClavPad.hpp"

#include <iostream>
#include <conio.h>

#include "optitrack_minimal_sc.hpp"
#include "glm/gtx/string_cast.hpp" 

#define VALIDATION_FRAME_COUNT 30
#define EPS 0.001

#define CALIBRATION_DURATION_SECOND 2
#define FRAME_RATE 120 

static const char* state_names[] =
{
    "idle",
    "keyboard",
    "cursor",
    "mouse"
};

ClavPad::ClavPad()
{
    open_udp_multicast_socket(k_default_port, k_default_mcast_group, e_true, "10.10.0.8");
    m_last_point = std::chrono::system_clock::now();
}

ClavPad::~ClavPad()
{
    if (g_mcast_address != NULL)
        free((void*)g_mcast_address);
    delete m_keyboard_adapter;
}

std::pair<float, float> getPrincipalLength(const RigidBody& rigid_body)
{
    if (rigid_body.markers.size() >= 3) {
        glm::vec3 ab = rigid_body.markers[1] - rigid_body.markers[0];
        glm::vec3 ac = rigid_body.markers[2] - rigid_body.markers[0];
        glm::vec3 bc = rigid_body.markers[2] - rigid_body.markers[1];

        float l1 = glm::length(ab);
        float l2 = glm::length(ac);
        float l3 = glm::length(bc);

        if (l1 > l2 && l1 > l3)
            return { l2, l3 };

        if (l2 > l1 && l2 > l3)
            return { l1, l3 };

        if (l3 > l1 && l3 > l2)
            return { l1, l2 };
    } else
        throw std::runtime_error("Can't compute size from the Rigid Body.");
}

void ClavPad::run()
{
    m_data.update();

    // Read optitrack data
    while (true) {
        m_keyboard_adapter->setOrigin(m_data.rigid_body.origin);
        m_keyboard_adapter->setOrientation(m_data.rigid_body.orientation);

        parseInput();

        if (m_state == State::CURSOR_MOVE) { // We're in cursor mode so send corresponding cursor pos
            // Convert into keyboard space
            if (m_data.markers.size() == 2) {
                auto p = m_keyboard_adapter->toScreen(m_data.markers.front());
                if (p.has_value())
                    m_input_sender.SendMousePos(p->first, p->second);
            }
        }

        if (GetKeyState(VK_ESCAPE)) // Stop running when ESC is pressed
            break;

        // Update optitrack data
        m_data.update();
    }

    updateActivity(m_state);
}

void ClavPad::calibrate() 
{
    m_data.update();

    // Initialize keyboard size
    auto [width, height] = getPrincipalLength(m_data.rigid_body);
    if (width < height)
        std::swap(width, height);

    if (m_keyboard_adapter != nullptr)
        delete m_keyboard_adapter;
    m_keyboard_adapter = new KeyboardAdapter(glm::vec3(), glm::conjugate(m_data.reference_orientation), m_data.rigid_body.orientation, width, height);

    int total_nb_values = FRAME_RATE * CALIBRATION_DURATION_SECOND;

    float distance_fingers = glm::distance(m_data.markers[0], m_data.markers[1]);

    m_finger_distance_threshold = distance_fingers;

    for (int i = 1; i < total_nb_values; i++) {
        m_data.update();
        float distance_fingers = glm::distance(m_data.markers[0], m_data.markers[1]);

        m_finger_distance_threshold += distance_fingers;
    }
    m_finger_distance_threshold /= total_nb_values;
    m_finger_distance_threshold += EPS;

    m_cursor_state_height_threshold = 0.02;
}

void ClavPad::printActivity() const
{
    for (const auto& [state, duration] : m_activity) {
        std::cout << state_names[(int)state] << ": " << std::to_string(duration.count()) << " s\n";
    }
}

void ClavPad::updateActivity(State old_state)
{
    auto current_time = std::chrono::system_clock::now();
    m_activity.push_back(std::make_pair(old_state, std::chrono::duration_cast<std::chrono::seconds>(current_time - m_last_point)));
    m_last_point = current_time;
}

bool ClavPad::gotoCursorMoveState(const glm::vec3& p1, const glm::vec3& p2)
{
    if (m_state == State::CURSOR_MOVE)
        return true;

    float distance_fingers = glm::distance(p1, p2);
    if (distance_fingers < m_finger_distance_threshold) // CURSOR mode transition criterion
        m_state_frame_count++;
    else
        m_state_frame_count = 0;

    if (m_state_frame_count == VALIDATION_FRAME_COUNT) {
        updateActivity(m_state);
        m_state = State::CURSOR_MOVE;
        std::cout << "\n Cursor mode activated.\n" << std::endl;
        return true;
    }
    
    return false;
}

bool ClavPad::gotoCursorClickState(const glm::vec3& p1, const glm::vec3& p2)
{
}

bool ClavPad::gotoKeyboardState()
{
    if (m_state == State::KEYBOARD)
        return true;

    if (kbhit()) { // KEYBOARD mode transition criterion
        m_state_frame_count = 0;
        updateActivity(m_state);
        m_state = State::KEYBOARD;
        std::cout << "\n Keyboard mode activated.\n" << std::endl;
        return true;
    }

    return false;
}

void ClavPad::parseInput()
{
    if (m_data.markers.size() < 2) { // Not enough markers, goto IDLE
        updateActivity(m_state);
        m_state = State::IDLE;
        return;
    }

    if (!gotoCursorMoveState(m_data.markers[0], m_data.markers[1])) 
        gotoKeyboardState();
}

bool ClavPad::kbhit() const
{
    for (char button = 0; button < 256; button++) {
        if (GetKeyState(button) & 0x8000) 
            return true;
    }
    return false;
}