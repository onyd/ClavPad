#ifndef RigidBody_HPP
#define RigidBody_HPP
    
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <vector>

class RigidBody {
    public:
        // center of the rigid body (center of the keyboard)
        glm::vec3 origin;

        // Quaternion
        glm::quat orientation;
        
        // positions of the markers forming the boundary of the keyboard
        std::vector<glm::vec3> markers;

        float length;

        float width;

        // Constructor
        RigidBody() {};

        RigidBody(glm::vec3 origin, glm::quat orientation, std::vector<glm::vec3> markers){
            origin = origin;
            orientation = orientation;
            markers = markers;
        }

        RigidBody(const RigidBody &rigiBody) {
            origin = rigiBody.origin;
            orientation = rigiBody.orientation;
            markers = rigiBody.markers;
        }

        void ComputeKeyBoardDimensions();
};

#endif
