#include "ClavPad.hpp"

#include <iostream>

#include "optitrack_minimal_sc.hpp"
#include "glm/gtx/string_cast.hpp" 

ClavPad::ClavPad()
{
    open_udp_multicast_socket(k_default_port, k_default_mcast_group, e_true, "10.10.0.5");
}

ClavPad::~ClavPad()
{
    if (g_mcast_address != NULL)
        free((void*)g_mcast_address);
}

std::pair<float, float> getPrincipalLength(const RigidBody& rigid_body)
{
    if (rigid_body.markers.size() >= 3) {
        glm::vec3 ab = rigid_body.markers[1] - rigid_body.markers[0];
        glm::vec3 ac = rigid_body.markers[2] - rigid_body.markers[0];
        glm::vec3 bc = rigid_body.markers[2] - rigid_body.markers[1];

        float l1 = ab.length();
        float l2 = ac.length();
        float l3 = bc.length();

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

    // Initialize keyboard size
    auto [width, height] = getPrincipalLength(m_data.rigid_body);
    if (width < height)
        std::swap(width, height);

    m_keyboard_adapter.setWidth(width);
    m_keyboard_adapter.setHeight(height);

    // Read optitrack data
    while (true) {

        std::cout << "Origin of rigid body : " << std::endl;
        std::cout << glm::to_string(m_data.rigid_body.origin) << std::endl;
        m_keyboard_adapter.setOrigin(m_data.rigid_body.origin);

        std::cout << "Quaternion of rigid body : " << std::endl;
        std::cout << glm::to_string(m_data.rigid_body.orientation) << std::endl;
        m_keyboard_adapter.setOrientation(m_data.rigid_body.orientation);

        std::cout << "Positions of markers of rigid body : " << std::endl;
        for (int i = 0; i < m_data.rigid_body.markers.size(); i++) {
            std::cout << glm::to_string(m_data.rigid_body.markers[i]) << std::endl;
        }

        std::cout << "Positions of markers : " << std::endl;
        for (int i = 0; i < m_data.markers.size(); i++) {
            std::cout << glm::to_string(m_data.markers[i]) << std::endl;
        }

        // Convert into keyboard space
        auto p = m_keyboard_adapter.toScreen(m_data.markers.front());
        if (p.has_value())
            m_input_sender.SendMousePos(p->first, p->second);
           
        // Update optitrack data
        m_data.update();
    }
}

