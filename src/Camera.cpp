#include "Camera.h"

glm::vec2 Camera::mouse_pos;

void Camera::setMousePos(float xPos, float yPos)
{
    mouse_pos.x = xPos - 1680 / 2.f;
    mouse_pos.y = -yPos + 1050 / 2.f;
}
