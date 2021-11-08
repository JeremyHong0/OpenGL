/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: scene.cpp
Purpose: This file is parent class for scenes.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include "scene.h"

Scene::Scene()
{
    _windowWidth = 1280;
    _windowHeight = 720;
}

Scene::Scene(int windowWidth, int windowHeight)
{
    _windowHeight = windowHeight;
    _windowWidth = windowWidth;
}

Scene::~Scene()
{
    CleanUp();
}

int Scene::Init()
{
    return -1;
}

void Scene::LoadAllShaders()
{
}

int Scene::Display()
{
    preRender();

    Render();

    RenderImGUI();

    postRender();

    return -1;
}

int Scene::preRender()
{
    return 0;
}

int Scene::Render()
{
    return 0;
}

int Scene::postRender()
{
    return 0;
}

void Scene::CleanUp()
{
}

void Scene::SetupImGUI(GLFWwindow* pWwindow)
{
}

void Scene::RenderImGUI()
{
}

void Scene::ProcessInput(GLFWwindow* pWwindow, double dt)
{
}
