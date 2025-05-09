/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: main.cpp
Purpose: This file creates application for window.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DeferredScene.h"
#include "scene.h"
#include "simpleScene.h"
#include "CrashHandler.h"
#include "Camera.h"

Scene* simple_scene;
Scene* deferredScene;
Scene* current_scene;
GLFWwindow* window;
const int windowWidth = 1680;
const int windowHeight = 1050;

double deltaTime = 0.0;
double lastFrame = 0.0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    Camera::mouse_pos.x = 0;
    Camera::setMousePos((float)xposIn, (float)yposIn);
}

int main()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, "HGraphics", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    CrashHandler::catchStackOverflow();
    SetUnhandledExceptionFilter(CrashHandler::WriteDump);

    deferredScene = new DeferredScene(windowWidth, windowHeight);
    simple_scene = new SimpleScene(windowWidth, windowHeight);
    simple_scene->LoadAllModels();

    current_scene = simple_scene;

    current_scene->Init(window);
    current_scene->SetupImGUI(window);

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        current_scene->Display();
        current_scene->ProcessInput(window, deltaTime);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}

