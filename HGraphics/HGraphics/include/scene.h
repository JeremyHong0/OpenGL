/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: scene.h
Purpose: This file is header for scene parent class.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>  // include glad to get all the required OpenGL headers
#include <GLFW/glfw3.h>

#define _GET_GL_ERROR   { GLenum err = glGetError(); std::cout << "[OpenGL Error] " << glewGetErrorString(err) << std::endl; }

class Scene
{
public:
    Scene();
    Scene(int windowWidth, int windowHeight);
    virtual ~Scene();

    // Public methods

    // Init: called once when the scene is initialized
    virtual int Init();

    // LoadAllShaders: This is the placeholder for loading the shader files
    virtual void LoadAllShaders();

    // Display : encapsulates per-frame behavior of the scene
    virtual int Display();

    // preRender : called to setup stuff prior to rendering the frame
    virtual int preRender();

    // Render : per frame rendering of the scene
    virtual int Render();

    // postRender : Any updates to calculate after current frame
    virtual int postRender();

    // cleanup before destruction
    virtual void CleanUp();

    // ImGUI stuff
    virtual void SetupImGUI(GLFWwindow* pWwindow);
    virtual void RenderImGUI();

    virtual void ProcessInput(GLFWwindow* pWwindow, double dt);

protected:
    int window_height_, window_width_;

    // Common functionality for all scene
};


#endif //SCENE_H
