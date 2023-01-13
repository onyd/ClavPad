#include "KeyboardAdapter.h"

KeyboardAdapter::KeyboardAdapter()
{
}

KeyboardAdapter::KeyboardAdapter(const glm::vec3& origin, const glm::quat& orientation, float width, float height)
    : m_origin(origin), m_orientation(orientation)
{
    RECT desktop;

    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);

    m_screen_width = desktop.right;
    m_screen_height = desktop.bottom;

    m_scale_factor = glm::vec2(m_screen_width / width, m_screen_height / height);
}

glm::vec3 KeyboardAdapter::toLocal(const glm::vec3& point)
{
    return glm::conjugate(m_orientation) * (point - m_origin);
}

std::optional<Point> KeyboardAdapter::toScreen(const glm::vec3& point)
{
    glm::vec3 p = toLocal(point);
    glm::vec2 projection = glm::vec2(p.x, p.y) * m_scale_factor + glm::vec2(m_screen_width, m_screen_height) / 2.0f;

    // Not in boundaries
    if (!(0 < projection.x && projection.x < m_screen_width))
        return std::nullopt;
    if (!(0 < projection.y && projection.y < m_screen_height))
        return std::nullopt;

    return std::make_optional<Point>(Point{ (int)projection.x, (int)projection.y });
}

void KeyboardAdapter::setOrigin(const glm::vec3& origin)
{
    m_origin = origin;
}

void KeyboardAdapter::setOrientation(const glm::quat& orientation)
{
    m_orientation = orientation;
}

void KeyboardAdapter::setWidth(float width)
{
    m_scale_factor.x = m_screen_width / width;
}

void KeyboardAdapter::setHeight(float height)
{
    m_scale_factor.y = m_screen_height / height;
}
