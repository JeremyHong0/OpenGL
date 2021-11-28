/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.h
Purpose: This file creates simple perspective camera and camera movement.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>


// Default camera values
const float SPEED = 2.5f;
const float ZOOM = 45.0f;

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
        CAM_PITCH
    };

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 world_up_;
    float yaw_;
    float pitch_;
    float movement_speed_;
    float zoom_;

    Camera(glm::vec3 position = glm::vec3(0.f), glm::vec3 front = glm::vec3(0.f, 0.f, 1.f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
    : position_(position), front_(front), up_(up), world_up_(up), yaw_(0.f), pitch_(0.f), movement_speed_(SPEED), zoom_(ZOOM)
    {
        right_ = glm::normalize(glm::cross(front_, world_up_));
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position_, position_ + front_, up_);
    }

    glm::vec3 GetPosition()
    {
        return position_;
    }

    void process_keyboard(Camera_Movement direction, double deltaTime);

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front_ = glm::normalize(front);
        right_ = glm::normalize(glm::cross(front, world_up_));
        up_ = glm::normalize(glm::cross(right_, front));
    }
};

inline void Camera::process_keyboard(Camera_Movement direction, double deltaTime)
{
    float velocity = movement_speed_ * (float)deltaTime;
    if (direction == Camera_Movement::CAM_FORWARD)
        position_ += front_ * velocity;
    if (direction == Camera_Movement::CAM_BACKWARD)
        position_ -= front_ * velocity;
    if (direction == Camera_Movement::CAM_LEFT)
        position_ -= right_ * velocity;
    if (direction == Camera_Movement::CAM_RIGHT)
        position_ += right_ * velocity;
    if (direction == Camera_Movement::CAM_YAW_LEFT)
        yaw_ += 1.f;
    if (direction == Camera_Movement::CAM_YAW_RIGHT)
        yaw_ -= 1.f;
    updateCameraVectors();
}
#endif
