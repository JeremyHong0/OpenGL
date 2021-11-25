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
const float YAW = 0.0f;
const float PITCH = 0.0f;
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

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    float yaw_;
    float pitch_;
    float movement_speed;
    float zoom;

    Camera(glm::vec3 position = glm::vec3(0.f), glm::vec3 front = glm::vec3(0.f,0.f,1.f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) :
       position(position), front(front), up(up), movement_speed(SPEED), zoom(ZOOM)
    {
        world_up = up;
        yaw_ = yaw;
        pitch_ = pitch;
        right = glm::normalize(glm::cross(front, world_up));
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
        front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED), zoom(ZOOM)
    {
        position = glm::vec3(posX, posY, posZ);
        world_up = glm::vec3(upX, upY, upZ);
        yaw_ = yaw;
        pitch_ = pitch;
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    glm::vec3 GetPosition()
    {
        return position;
    }

    void process_keyboard(Camera_Movement direction, double deltaTime);

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }
};

inline void Camera::process_keyboard(Camera_Movement direction, double deltaTime)
{
    float velocity = movement_speed * (float)deltaTime;
    if (direction == Camera_Movement::CAM_FORWARD)
        position += front * velocity;
    if (direction == Camera_Movement::CAM_BACKWARD)
        position -= front * velocity;
    if (direction == Camera_Movement::CAM_LEFT)
        position -= right * velocity;
    if (direction == Camera_Movement::CAM_RIGHT)
        position += right * velocity;
    if (direction == Camera_Movement::CAM_YAW_LEFT)
        yaw_ += 1.f;
    if (direction == Camera_Movement::CAM_YAW_RIGHT)
        yaw_ -= 1.f;
    updateCameraVectors();

}
#endif
