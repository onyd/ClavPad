#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "wtypes.h"

#include <optional>

typedef std::pair<int, int> Point;

class KeyboardAdapter
{
public:
	KeyboardAdapter(const glm::vec3& origin, 
					const glm::quat& reference_orientation,
					const glm::quat& orientation,
					float width, float height);
	 
	glm::vec3 toLocal(const glm::vec3& point);
	std::optional<Point> toScreen(const glm::vec3& point);

	void setOrigin(const glm::vec3& origin);
	void setOrientation(const glm::quat& orientation);
	void setWidth(float width);
	void setHeight(float height);

private:
	glm::vec3 m_origin;
	glm::quat m_reference_orientation;
	glm::quat m_orientation;
	glm::vec2 m_scale_factor;
	long m_screen_width, m_screen_height;
};