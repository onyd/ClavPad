#include "ClavPad.hpp"

#include <iostream>
#include <fstream>
#include <conio.h>

#include "optitrack_minimal_sc.hpp"
#include "glm/gtx/string_cast.hpp" 

#define VALIDATION_FRAME_COUNT 30
#define IDLE_FRAME_COUNT 1000
#define EPS 0.001

#define CALIBRATION_DURATION_SECOND 2
#define FRAME_RATE 120 

static const char* state_names[] =
{
    "idle",
    "keyboard",
    "cursor_move",
    "cursor_click",
    "mouse_move",
    "mouse_click"
};

ClavPad::ClavPad(bool mouse_mode) : m_mouse_mode(mouse_mode)
{
    if (!mouse_mode)
        open_udp_multicast_socket(k_default_port, k_default_mcast_group, e_true, "10.10.0.8");
    m_last_activity_start = std::chrono::system_clock::now();
    m_idle_start = std::chrono::system_clock::now();
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
    update();

    // Read optitrack data
    while (!m_should_stop) {
        if (!m_mouse_mode) {
            m_keyboard_adapter->setOrigin(m_data.rigid_body.origin);
            m_keyboard_adapter->setOrientation(m_data.rigid_body.orientation);
        }

        parseInput();

        if (!m_mouse_mode && m_state == State::CURSOR_MOVE) { // We're in cursor mode so send corresponding cursor pos
            // Convert into keyboard space
            if (m_data.markers.size() == 2) {
                auto p = m_keyboard_adapter->toScreen(m_data.markers.front());
                if (p.has_value())
                    m_input_sender.SendMousePos(p->first, p->second);
            }
        } 
        if (!mouse_event && m_state == State::CURSOR_CLICK) {
            m_input_sender.SendClick();
            updateState(State::IDLE);
        }

        // Update optitrack data
        update();
    }

    updateState(m_state);
}

void ClavPad::update()
{
    if (!m_mouse_mode)
        m_data.update();
}

void ClavPad::calibrate() 
{
    if (m_mouse_mode) // No need of calibration
        return;

    std::cout << "Begin calibration" << std::endl;

    update();

    // Initialize keyboard size
    auto [width, height] = getPrincipalLength(m_data.rigid_body);
    if (width < height)
        std::swap(width, height);

    if (m_keyboard_adapter != nullptr)
        delete m_keyboard_adapter;
    m_keyboard_adapter = new KeyboardAdapter(glm::vec3(), glm::conjugate(m_data.rigid_body.orientation), m_data.rigid_body.orientation, width, height);


    float pre_last_heights = m_data.markers.front().z;
    update();
    m_last_heights = m_data.markers.front().z;
    m_last_vertical_velocity = (m_last_heights - pre_last_heights) * FRAME_RATE;
    m_vertical_accelerations.push_back(0.0);

    int total_nb_values = FRAME_RATE * CALIBRATION_DURATION_SECOND;

    float distance_fingers = glm::distance(m_data.markers[0], m_data.markers[1]);
    m_finger_distance_threshold = distance_fingers;

    float min_mean_acc = 0.0;
    for (int i = 1; i < total_nb_values; i++) {
        update();

        // Finger distance calibration
        float distance_fingers = glm::distance(m_data.markers[0], m_data.markers[1]);
        m_finger_distance_threshold += distance_fingers;

        // Click acceleration calibration
        float current_vertical_velocity = (m_data.markers.front().z - m_last_heights) * FRAME_RATE;
        float current_acc = (current_vertical_velocity - m_last_vertical_velocity) * FRAME_RATE;
        std::cout << current_acc << "\n";

        m_last_vertical_velocity = current_vertical_velocity;
        m_vertical_accelerations.push_back(current_acc);
        if (m_vertical_accelerations.size() > 10) {
            m_vertical_accelerations.pop_front();

            float mean_acc = 0;
            for (const auto& acc : m_vertical_accelerations)
                mean_acc += acc;
            mean_acc /= m_vertical_accelerations.size();
            min_mean_acc = min(min_mean_acc, mean_acc);
        }
    }

    m_finger_distance_threshold /= total_nb_values;
    m_finger_distance_threshold += EPS;

    m_acc_threshold = 0.9 * min_mean_acc;

    std::cout << "End calibration" << std::endl;
}

void ClavPad::printActivity() const
{
    for (const auto& [state, duration] : m_activity) {
        std::cout << state_names[(int)state] << ": " << std::to_string(duration.count()) << " ms\n";
    }
}

void ClavPad::saveActivity(const std::string& path)
{
    std::ofstream out_file(path);
    out_file << "[\n";

    auto it = m_activity.begin();
    out_file << "{" << "\"" << state_names[(int)it->first] << "\" : " << std::to_string(it->second.count()) << "}";
    ++it;
    while (it != m_activity.end()) {
        const auto& [state, duration] = *it++;
        out_file << ",\n{" << "\"" << state_names[(int)state] << "\" : " << std::to_string(duration.count()) << "}";
    }
    out_file << "\n]";

    out_file.close();
}


bool ClavPad::updateState(State new_state)
{
    m_idle_start = std::chrono::system_clock::now();

    if (m_state != new_state) {
        auto current_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_last_activity_start);

        m_activity.push_back(std::make_pair(m_state, duration));
        m_last_activity_start = current_time;
        m_state = new_state;
        std::cout << "\n" << state_names[(int)m_state] << " mode activated.\n" << std::endl;
        return true;
    }

    return false;
}

void ClavPad::parseInput()
{
    POINT current_mouse_pos;
    GetCursorPos(&current_mouse_pos);
    bool has_mouse_moved = current_mouse_pos.x != m_last_mouse_pos.x || current_mouse_pos.y != m_last_mouse_pos.y;
    m_last_mouse_pos = current_mouse_pos;

    // Should we go in KEYBOARD mode
    if (handleKeyBoardInput()) {
        updateState(State::KEYBOARD);
        return;
    }

    // Should we go in IDLE mode
    if (!m_mouse_mode && m_data.markers.size() < 2) { // Not enough markers, goto IDLE
        updateState(State::IDLE);
        return;
    }

    if (!m_mouse_mode) {
        // Should we go in CURSOR_CLICK mode
        if (m_state == State::CURSOR_MOVE || m_state == State::MOUSE_MOVE) {
            float current_vertical_velocity = (m_data.markers.front().z - m_last_heights) * FRAME_RATE;
            float current_acc = (current_vertical_velocity - m_vertical_accelerations.back()) * FRAME_RATE;

            // Update velocity and acceleration
            m_last_vertical_velocity = current_vertical_velocity;
            m_vertical_accelerations.push_back(current_acc);

            if (m_vertical_accelerations.size() > 10) {
                m_vertical_accelerations.pop_front();

                float mean_acc = 0;
                for (const auto& acc : m_vertical_accelerations)
                    mean_acc += acc;
                mean_acc /= m_vertical_accelerations.size();

                if (mean_acc < m_acc_threshold) {
                    updateState(State::CURSOR_CLICK);
                    return;
                }
            }
        }

        // Should we go in CURSOR_MOVE mode
        float distance_fingers = glm::distance(m_data.markers[0], m_data.markers[1]);
        if (distance_fingers < m_finger_distance_threshold
            && ++m_state_frame_count >= VALIDATION_FRAME_COUNT) {
            updateState(State::CURSOR_MOVE);
            return;
        }
        else { // 'Open' finger => reset counter
            m_state_frame_count = 0;
        }
    }

    // Should we go in MOUSE_CLICK mode
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        updateState(State::MOUSE_CLICK);
        return;
    }

    // Should we go in MOUSE_MOVE mode
    if (has_mouse_moved) {
        updateState(State::MOUSE_MOVE);
        return;
    }

    auto idle_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_idle_start);
    if (idle_duration.count() > 100)
        updateState(State::IDLE); // No mode match => go to IDLE
}

// Check if we should stop with ESC and return if key has been pressed
bool ClavPad::handleKeyBoardInput()
{
    BYTE keys[256];
    if (GetKeyState(VK_ESCAPE) & 0x8000)
        m_should_stop = true;

    if (!GetKeyboardState(keys))
        return false;

    for (int i = 0; i < 256; i++) {
        if (i != VK_LBUTTON && keys[i] & 0x80)
            return true;
    }
    return false;
}