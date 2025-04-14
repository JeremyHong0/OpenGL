#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Default camera values
const float SPEED = 2.5f;
const float ZOOM = 45.0f;

struct CameraDireciton {
    GLenum face;
    glm::vec3 target;
    glm::vec3 up;
};

class Camera
{
public:
    enum class Camera_Movement
    {
        CAM_FORWARD,
        CAM_BACKWARD,
        CAM_LEFT,
        CAM_RIGHT,
        CAM_YAW_LEFT,
        CAM_YAW_RIGHT,
        CAM_PITCH,
        CAM_UP,
        CAM_DOWN
    };

    glm::vec3 m_Position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 world_up_;
    float yaw_;
    float pitch_;
    float movement_speed_;
    float zoom_;
    float mouseSensitivity;

    Camera(glm::vec3 position = glm::vec3(0.f),  glm::vec3 front = glm::vec3(0.f, 0.f, 1.f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
        : m_Position_(position), front_(front), up_(up), world_up_(up), yaw_(0.f), pitch_(0.f), movement_speed_(SPEED),
          zoom_(ZOOM), mouseSensitivity(0.1f)
    {
        right_ = normalize(cross(front_, world_up_));
        lastX = 1600 / 2.f;
        lastY = 1200 / 2.f;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const
    {
        return lookAt(m_Position_, m_Position_ + front_, up_);
    }

    glm::vec3 GetPosition() const
    {
        return m_Position_;
    }

    void process_keyboard(Camera_Movement direction, double deltaTime);

    void ProcessMouseMovement(GLboolean constrainPitch = true);

    static void setMousePos(float xPos, float yPos);
    
    static glm::vec2 mouse_pos;

private:

    float lastX;
    float lastY;
    bool firstMouse = true;

    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front_ = normalize(front);
        right_ = normalize(cross(front, world_up_));
        up_ = normalize(cross(right_, front));
    }

};

inline void Camera::process_keyboard(Camera_Movement direction, double deltaTime)
{
    const auto velocity = movement_speed_ * static_cast<float>(deltaTime);
    if (direction == Camera_Movement::CAM_FORWARD)
        m_Position_ += front_ * velocity;
    if (direction == Camera_Movement::CAM_BACKWARD)
        m_Position_ -= front_ * velocity;
    if (direction == Camera_Movement::CAM_LEFT)
        m_Position_ -= right_ * velocity;
    if (direction == Camera_Movement::CAM_RIGHT)
        m_Position_ += right_ * velocity;
    if (direction == Camera_Movement::CAM_YAW_LEFT)
        yaw_ += 1.f;
    if (direction == Camera_Movement::CAM_YAW_RIGHT)
        yaw_ -= 1.f;
    if (direction == Camera_Movement::CAM_UP)
        m_Position_ += up_ * velocity;
    if (direction == Camera_Movement::CAM_DOWN)
        m_Position_ -= up_ * velocity;
    updateCameraVectors();
}

inline void Camera::ProcessMouseMovement(GLboolean constrainPitch)
{
    float xpos = mouse_pos.x;
    float ypos = mouse_pos.y;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw_ -= xoffset;
    pitch_ -= yoffset;

    if (pitch_ > 89.0f)
        pitch_ = 89.0f;
    if (pitch_ < -89.0f)
        pitch_ = -89.0f;

    updateCameraVectors();
}

#endif
