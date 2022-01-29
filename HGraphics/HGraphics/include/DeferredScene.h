/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: DeferredShading.h
Purpose: This file is header for deferred shading scene.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Jan 8, 2022
End Header ---------------------------------------------------------*/
#ifndef DEFERRED_SCENE_H
#define DEFERRED_SCENE_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Camera.h"
#include "scene.h"
#include "shader.hpp"


class DeferredScene : public Scene
{
public:
    DeferredScene() = default;
    DeferredScene(int windowWidth, int windowHeight);
    virtual ~DeferredScene();

    int Init() override;

    int Render() override;
    int postRender() override;

    void SetupImGUI(GLFWwindow* pWwindow) override;
    int RenderImGUI() override;

    void ProcessInput(GLFWwindow* pWwindow, double dt) override;

    int lightNum;

private:
    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Shader> drawNormalShader;
    std::unique_ptr<Shader> lightSphereShader;
    std::unique_ptr<Shader> defferedShader;

    std::unique_ptr<Camera> camera_;

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

    glm::vec4 light_pos_ = glm::vec4(1.f);
    glm::vec3 obj_color = glm::vec3(0.4f);
    glm::vec3 Ka = glm::vec3(0.f, 0.f, 0.01f);
    glm::vec3 Kd = glm::vec3(1.f);
    glm::vec3 Ks = glm::vec3(1.f);

    float screen_width, screen_height;

    float orbitRadius;

    float normalSize;
    bool bShowVNormal;
    bool bShowFNormal;
    bool bReloadShader;
    bool bReCalcUVs;
    bool bRotate;
    bool bCalcUVatGPU;
    bool bCalcUVPos;

    float att_const_[4] = { 1.f, 0.22f,0.2f };
    glm::vec3 light_positions_[16];
    std::vector<glm::vec3> la_;
    std::vector<glm::vec3> ld_;
    std::vector<glm::vec3> ls_;
    std::vector<int> light_type_;
    std::vector<float> spot_inner_;
    std::vector<float> spot_outer_;
    std::vector<float> spot_falloff_;
    int total_light_num_;

    std::vector<std::string> loadedShader;
    std::string currentModelName;
    std::string currentVShader;
    std::string currentFShader;
    std::vector<std::string> current_light_type_;
    std::string current_light_num_;
    int selected_light_num_ = 0;

    unsigned int diffMap;
    unsigned int specMap;
    unsigned diff_texture_;
    unsigned spec_texture_;

    float fogMaxDist;
    float fogMinDist;
    glm::vec3 fogColor;

    std::vector<std::string> UVTypes;
    std::string currUVType;
    std::vector<std::string> UVPipeline;
    std::string currUVPipeline;
    std::vector<std::string> UVEntity;
    std::string currUVEntity;

    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gUV, gDepth;
    unsigned int rboDepth;
    int drawBuffer;

    enum eLightTypes
    {
        Point = 0,
        Direction,
        Spot
    };
};


#endif //SIMPLE_SCENE_H