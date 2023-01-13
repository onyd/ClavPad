#include "OptitrackData.hpp"
#include "optitrack_minimal_sc.hpp"

OptitrackData::OptitrackData(size_t buffer_size)
{
    m_packet_buffer = new char[buffer_size];
}

OptitrackData::~OptitrackData()
{
    delete[] m_packet_buffer;
}

void OptitrackData::update() {
    glm::vec3 origin;
    glm::quat quaternion;
    std::vector<glm::vec3> rigid_body_markers;
    std::vector<glm::vec3> markers_list;
    receive_packet(origin, quaternion, rigid_body_markers, markers_list, m_packet_buffer, K_SOCK_MAX_PACKET_SIZE);
    
    rigid_body.origin = origin;
    rigid_body.orientation = quaternion;
    rigid_body.markers = rigid_body_markers;
    markers = markers_list;
}

