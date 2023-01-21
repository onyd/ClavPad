#pragma once

#include "RigidBody.hpp"
#include "glm/glm.hpp"
#include "optitrack_minimal_sc.hpp"

#include <vector>

#define			K_SOCK_MAX_PACKET_SIZE	20000


class OptitrackData {
    // Struct to get raw data from the optitrack and store it in suited collections
public:
    OptitrackData(size_t buffer_size = K_SOCK_MAX_PACKET_SIZE);
    ~OptitrackData();

    // structure containing info on the four vec3s delimiting the keyboard and its origin
    RigidBody rigid_body;  

    // vector of the vec3s of the different markers which we will stick to the fingers   
    std::vector<glm::vec3> markers;

    // get raw data from the optitrack
    void update();
private:
    char* m_packet_buffer;
}; 

