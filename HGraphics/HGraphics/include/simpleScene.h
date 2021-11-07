/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: simpleScene.h
Purpose: This file is header for simple scene.
Language: c++
Platform: VS2019 / Window
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef SIMPLE_SCENE_H
#define SIMPLE_SCENE_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "LightManager.h"
#include "Camera.h"
#include "mesh.h"
#include "OBJManager.h"
#include "scene.h"
#include "shader.hpp"


class SimpleScene : public Scene
{
public:
    SimpleScene() = default;
    SimpleScene(int windowWidth, int windowHeight);
    virtual ~SimpleScene();

    int Init() override;

    int Render() override;
    int postRender() override;

    void SetupImGUI(GLFWwindow* pWwindow) override;
    void RenderImGUI() override;

    void ProcessInput(GLFWwindow* pWwindow, double dt) override;

    int lightNum;

private:
    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Shader> drawNormalShader;
    std::unique_ptr<Shader> lightSphereShader;
    std::unique_ptr<Light> light_;

    std::vector<Light> lights_;

    std::unique_ptr<Camera> camera_;

    // member functions
    void initMembers();
    GLfloat angleOfRotation;

    unsigned int uboMatirices;

    //matrix for model rendering
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 scale = glm::vec3(0.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat3 normalMatrix = glm::mat3(0.0f);
    glm::vec3 globalAmbient = glm::vec3(0.f, 0.f, 0.1f);

    //matrix for drawing normal
    glm::mat4 drawNormModel = glm::mat4(1.0f);
    glm::vec3 drawNormScale = glm::vec3(0.0f);
    glm::mat4 drawNormView = glm::mat4(1.0f);
    glm::mat4 drawNormProjection = glm::mat4(1.0f);

    glm::vec4 lightPos = glm::vec4(1.f);
    glm::vec3 obj_color = glm::vec3(0.f);
    glm::vec3 Ka = glm::vec3(0.f,0.f,0.01f);
    glm::vec3 Kd = glm::vec3(1.f);
    glm::vec3 Ks = glm::vec3(1.f);

    float screen_width, screen_height;

    float orbitRadius;

    int scenario = 0;
    int selectedLightNum = 0;
    float normalSize;
    bool bShowVNormal;
    bool bShowFNormal;
    bool bReloadShader;
    bool bReCalcUVs;
    bool bRotate;
    bool bCalcUVatGPU;
    bool bCalcUVPos;
    bool bShowUV = false;

    OBJManager obj_manager_;

    std::vector<std::string> loadedModelName;
    std::vector<std::string> loadedShader;
    std::string currentModelName;
    std::string currentVShader;
    std::string currentFShader;
    std::vector<std::string> LightTypes;
    std::string currentLightType;
    std::string currentLightNum;
    std::vector<std::string> currentLightType_;
    std::vector<std::string> LightNums;

    glm::vec3 LightPositions[16];
    std::vector<glm::vec3> La_;
    std::vector<glm::vec3> Ld_;
    std::vector<glm::vec3> Ls_;
    std::vector<int> lightType;
    std::vector<float> spotInner;
    std::vector<float> spotOuter;
    std::vector<float> spotFalloff;

    unsigned int diffMap;
    unsigned int specMap;
    unsigned int gridMap;

    float fogMaxDist;
    float fogMinDist;
    glm::vec3 fogColor;
    float attConst[4] = {1.f, 0.22f,0.2f};

    std::vector<std::string> UVTypes;
    std::string currUVType;
    std::vector<std::string> UVPipeline;
    std::string currUVPipeline;
    std::vector<std::string> UVEntity;
    std::string currUVEntity;
};


#endif //SIMPLE_SCENE_H
