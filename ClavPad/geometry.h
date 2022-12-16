#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Plane
{
public:
	Plane(const glm::vec3& p, const glm::vec3& normal);
	Plane(const glm::vec3& p, const glm::quat& orientation);

private:
	glm::vec3 m_p;
	glm::vec3 m_normal;
};