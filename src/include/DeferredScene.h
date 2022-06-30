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
#include <GBuffer.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Camera.h"
#include "PointLightShadowMap.h"
#include "scene.h"
#include "shader.hpp"
#include "ShadowMap.h"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 attenuation;
    float radius;
};

class DeferredScene : public Scene
{
public:
    DeferredScene() = delete;
    DeferredScene(int windowWidth, int windowHeight);
    ~DeferredScene() override;

    int Init(GLFWwindow* pWwindow) override;

    int Render() override;
    int postRender() override;

    void SetupImGUI(GLFWwindow* pWwindow) override;
    int RenderImGUI() override;

    void ProcessInput(GLFWwindow* pWwindow, double dt) override;

    int lightNum;

private:
    void initMembers();

    void initKernel();
    void loadCubemap();
    void geometryPass();
    void shadowPass();
    void plShadowPass(PointLight pl);
    void stencilPass(PointLight pl);
    void pointLightPass(PointLight pl);
    void ssaoPass();
    void blurPass();
    void compositePass();
    void skyboxPass();



    GLuint noiseTex, skyboxTexture;
    std::vector<float> kernel;
    glm::vec2 noiseScale;

    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Shader> drawNormalShader;
    std::unique_ptr<Shader> shadowShader;
    std::unique_ptr<Shader> geometryShader;
    std::unique_ptr<Shader> stencilShader;
    std::unique_ptr<Shader> lightPassShader;
    std::unique_ptr<Shader> pointLightShader;
    std::unique_ptr<Shader> finalPassShader;
    std::unique_ptr<Shader> ssaoShader;
    std::unique_ptr<Shader> skyboxShader;

    std::unique_ptr<Camera> camera_;
    CameraDireciton directions[6];

    GLfloat angleOfRotation;

    unsigned int uboMatirices;

    //matrix for model rendering
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 scale = glm::vec3(0.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat3 normalMatrix = glm::mat3(0.0f);
    glm::vec3 globalAmbient = glm::vec3(0.f, 0.f, 0.1f);

    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;

    //matrix for drawing normal
    glm::mat4 drawNormModel = glm::mat4(1.0f);
    glm::vec3 drawNormScale = glm::vec3(0.0f);
    glm::mat4 drawNormView = glm::mat4(1.0f);
    glm::mat4 drawNormProjection = glm::mat4(1.0f);

    glm::vec4 light_pos_ = glm::vec4(1.f, 1.f,1.f,0.f);
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
    bool bCopyDepth;

    std::string currentModelName;
    unsigned AlbedoTexture_;
    unsigned AmbientTexture_;
    unsigned NormTexture_;

    float fogMaxDist;
    float fogMinDist;
    glm::vec3 fogColor;

    std::vector<std::string> UVTypes;
    std::string currUVType;
    std::vector<std::string> UVPipeline;
    std::string currUVPipeline;
    std::vector<std::string> UVEntity;
    std::string currUVEntity;

    std::vector<PointLight> Lights_;

    GBuffer gBuffer;
    ShadowMap ShadowMap_;
    PointLightShadowMap PointLightShadowMap_;
    GLuint gPosition, gNormal, gAlbedo, gDepth;
    unsigned int rboDepth;
    int drawBuffer;
    unsigned int attachments[4];

    enum eLightTypes
    {
        Point = 0,
        Direction,
        Spot
    };
};


#endif //SIMPLE_SCENE_H