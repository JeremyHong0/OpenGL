/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: DeferredScene.cpp
Purpose: This file generate scene to load/render models using deferred shading
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Jan 8, 2022
End Header ---------------------------------------------------------*/
#include "DeferredScene.h"

#define STB_IMAGE_IMPLEMENTATION

#include <memory>
#include <queue>
#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "OBJManager.h"
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
static const int kernelSize = 64;
static const int noiseSize = 4;
float lastX;
float lastY;
bool firstMouse = true;

//void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

DeferredScene::~DeferredScene()
{
    camera_ = nullptr;
    mainShader = nullptr;
    drawNormalShader = nullptr;
    UVTypes.clear();
    UVPipeline.clear();
    UVEntity.clear();
}

DeferredScene::DeferredScene(int windowWidth, int windowHeight) :
    Scene(windowWidth, windowHeight), angleOfRotation(0.0f),
    gBuffer(GBuffer(windowWidth, windowHeight)), ShadowMap_(ShadowMap(2048, 2048)),
    PointLightShadowMap_(1024,1024)
{
    initMembers();
    drawBuffer = 0;
    screen_width = static_cast<float>(windowWidth);
    screen_height = static_cast<float>(windowHeight);
    lastX = screen_width / 2.f;
    lastY = screen_height / 2.f;
}

void DeferredScene::initMembers()
{
    angleOfRotation = 0.0f;
    orbitRadius = 2.5f;
    camera_ = nullptr;
    mainShader = nullptr;
    drawNormalShader = nullptr;
    bShowVNormal = false;
    bShowFNormal = false;
    bReloadShader = false;
    bReCalcUVs = false;
    bRotate = true;
    bCalcUVatGPU = true;
    bCopyDepth = true;
    normalSize = 0.2f;
    lightNum = 5;
    AlbedoTexture_ = 0;
    AmbientTexture_ = 0;
    fogMaxDist = 20.f;
    fogMinDist = 0.1f;
    fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
    UVTypes = { "Cylindrical", "Spherical", "Cube", "Planar", "None" };
    currUVType = "Cylindrical";
    UVPipeline = { "CPU", "GPU" };
    currUVPipeline = "GPU";
    UVEntity = { "Position", "Normal" };
    currUVEntity = "Position";
}

void DeferredScene::initKernel()
{
    for (int i = 0; i < kernelSize; i++) {
        float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        glm::vec3 k(r1 * 2.0f - 1.0f, r2 * 2.0f - 1.0f, r3);
        k = glm::normalize(k);
        float scale = float(i) / float(kernelSize);
        scale = 0.1f;
        k *= scale;

        kernel.push_back(k.x);
        kernel.push_back(k.y);
        kernel.push_back(k.z);
    }

    std::vector<glm::vec3> noise;
    for (int i = 0; i < kernelSize; i++) {
        float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        glm::vec3 n(r1 * 2.0f - 1.0f, r2 * 2.0f - 1.0f, 0);
        n = glm::normalize(n);
        noise.push_back(n);
    }

    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, noiseSize, noiseSize, 0, GL_RGBA, GL_FLOAT, &noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    noiseScale = glm::vec2(window_width_ / noiseSize, window_height_ / noiseSize);
}

int DeferredScene::Init(GLFWwindow* pWwindow)
{
    //mainShader = std::make_unique<Shader>();
    drawNormalShader = std::make_unique<Shader>();
    shadowShader = std::make_unique<Shader>();
    geometryShader = std::make_unique<Shader>();
    stencilShader = std::make_unique<Shader>();
    lightPassShader = std::make_unique<Shader>();
    pointLightShader = std::make_unique<Shader>();
    finalPassShader = std::make_unique<Shader>();
    ssaoShader = std::make_unique<Shader>();
    skyboxShader = std::make_unique<Shader>();

    /*mainShader->loadShader("shader/FSQShading.vert",
        "shader/FSQShading.frag");*/
    drawNormalShader->loadShader("../assets/shader/normalShader.vert",
        "../assets/shader/normalShader.frag");
    shadowShader->loadShader("../assets/shader/shadow.vert",
        "../assets/shader/shadow.frag");
    geometryShader->loadShader("../assets/shader/geometry.vert",
        "../assets/shader/geometry.frag");
    stencilShader->loadShader("../assets/shader/light.vert",
        "../assets/shader/shadow.frag");
    lightPassShader->loadShader("../assets/shader/light.vert",
        "../assets/shader/light.frag");
    pointLightShader->loadShader("../assets/shader/pointLightShadow.vert",
        "../assets/shader/pointLightShadow.frag");
    finalPassShader->loadShader("../assets/shader/finalPass.vert",
        "../assets/shader/finalPass.frag");
    ssaoShader->loadShader("../assets/shader/ssao.vert",
        "../assets/shader/ssao.frag");
    skyboxShader->loadShader("../assets/shader/skybox.vert",
        "../assets/shader/skybox.frag");

    AlbedoTexture_ = OBJ_MANAGER->getTexture("albedoTexture");
    NormTexture_ = OBJ_MANAGER->getTexture("normTexture");

    currentModelName = OBJ_MANAGER->loaded_models[0];

    std::vector<std::string> faces
    {
        "../assets/textures/right.jpg",
        "../assets/textures/left.jpg",
        "../assets/textures/top.jpg",
        "../assets/textures/bottom.jpg",
        "../assets/textures/front.jpg",
        "../assets/textures/back.jpg"
    };

    //skyboxTexture = OBJ_MANAGER->load_cubemap(faces);
    skyboxTexture = OBJ_MANAGER->loadCubemap(faces);

    camera_ = std::make_unique<Camera>(glm::vec3(4.8f, 6.6f, 7.1f));

    for (int i = 0; i < 1; i++) {
        
        PointLight pl;
        pl.color = glm::vec3(1);
        pl.position = glm::vec3(0, 20, 0);
        pl.attenuation = glm::vec3(1, 0.01f, 0.001f); //radius, 0.001 = 500, 0.01 = 159
        pl.radius = (-pl.attenuation.y + sqrtf(pow(pl.attenuation.y, 2) - (4 * pl.attenuation.z * (pl.attenuation.x - 256)))) / (2 * pl.attenuation.z);
        Lights_.push_back(pl);
    }

    directions[0] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
    directions[1] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
    directions[2] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) };
    directions[3] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };
    directions[4] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
    directions[5] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) };

    initKernel();

    return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

int DeferredScene::Render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    view = camera_->GetViewMatrix();
    projection = glm::perspective(glm::radians(camera_->zoom_), (float)screen_width / (float)screen_height, 0.1f,
        100.0f);

    glm::vec3 lightPos(0.0f, 10.0f, 0.0f);

    float near_plane = 1.0f, far_plane = 7.5f;
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(Lights_[0].position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    geometryPass();

    shadowPass();

    gBuffer.bindDraw();
    gBuffer.setDrawLight();
    glClear(GL_COLOR_BUFFER_BIT);
    gBuffer.unbindDraw();

    for(auto& light : Lights_)
    {
        glViewport(0, 0, 1024, 1024);
        plShadowPass(light);
        glViewport(0, 0, window_width_, window_height_);
        glEnable(GL_STENCIL_TEST);
        stencilPass(light);
        pointLightPass(light);
        glDisable(GL_STENCIL_TEST);
    }
    ssaoPass();

    compositePass();

    skyboxPass();
    /*if(bCopyDepth)
        glBlitFramebuffer(0, 0, (GLint)screen_width, (GLint)screen_height, 0, 0, (GLint)screen_width, (GLint)screen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

    if (bShowVNormal)
    {
        for (auto& i : OBJ_MANAGER->loaded_models)
        {
            drawNormalShader->use();
            drawNormalShader->SetUniform("model", drawNormModel);
            drawNormalShader->SetUniform("view", view);
            drawNormalShader->SetUniform("projection", projection);
            drawNormalShader->SetUniform("color", glm::vec3(0.32f, 0.57f, 0.86f));
            OBJ_MANAGER->GetMesh(i)->render(1);
        }
    }
    if (bShowFNormal)
    {
        for (auto& i : OBJ_MANAGER->loaded_models)
        {
            drawNormalShader->use();
            drawNormalShader->SetUniform("model", drawNormModel);
            drawNormalShader->SetUniform("view", view);
            drawNormalShader->SetUniform("projection", projection);
            drawNormalShader->SetUniform("color", glm::vec3(0.2f, 0.49f, 0.0f));
            OBJ_MANAGER->GetMesh(i)->render(2);
        }
    }


    GLenum err = glGetError();
    if (err != 0) std::cout << err << std::endl;
    return 0;
}

int DeferredScene::postRender()
{
    if (bRotate)
        angleOfRotation += 0.01f;
    return 0;
}

void DeferredScene::SetupImGUI(GLFWwindow* pWwindow)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(pWwindow, true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

int DeferredScene::RenderImGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Controls");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);

    //Model config
    if (ImGui::CollapsingHeader("Model"))
    {
        static const char* current_item = currentModelName.c_str();
        if (ImGui::BeginCombo("Loaded Model", current_item))
        {
            for (const auto& model : OBJ_MANAGER->loaded_models)
            {
                bool is_selected = currentModelName == model;
                if (ImGui::Selectable(model.c_str(), is_selected))
                {
                    currentModelName = model;
                    current_item = model.c_str();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }


        ImGui::Checkbox("Draw Vertex Normal", &bShowVNormal);
        ImGui::Checkbox("Draw Face Normal", &bShowFNormal);
    }
    ImGui::Checkbox("Copy Depth", &bCopyDepth);
    ImGui::End();


    ImGui::Begin("FBOs");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Image((ImTextureID)(intptr_t)gBuffer.position, ImVec2(400,250), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image((ImTextureID)(intptr_t)gBuffer.normal, ImVec2(400, 250), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image((ImTextureID)(intptr_t)gBuffer.color, ImVec2(400, 250), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image((ImTextureID)(intptr_t)gBuffer.light, ImVec2(400, 250), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    return Scene::RenderImGUI();
}

void DeferredScene::geometryPass()
{
    geometryShader->use();
    gBuffer.bindDraw();
    gBuffer.setDrawBuffers();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, AlbedoTexture_);
    geometryShader->SetUniform("texture_diff", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormTexture_);
    geometryShader->SetUniform("texture_normal", 1);

    model = glm::mat4(1.f);
    model = glm::scale(glm::vec3(10.f)) * glm::rotate(glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f));

    geometryShader->SetUniform("model", model);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    drawNormModel = model;

    geometryShader->SetUniform("view", view);
    geometryShader->SetUniform("projection", projection);
    geometryShader->SetUniform("isWithTexture", 0);
    geometryShader->SetUniform("diffColor", glm::vec3(0, 0, 1));

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& i : OBJ_MANAGER->loaded_models)
        OBJ_MANAGER->GetMesh(i)->render();

    model = glm::mat4(1.f);
    model = glm::translate(glm::vec3(0, -0.5f, 0)) * glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
        * glm::scale(glm::vec3(5, 5, 1));
    geometryShader->SetUniform("model", model);
    geometryShader->SetUniform("isWithTexture", 1);
    //OBJ_MANAGER->GetMesh("plane")->render();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void DeferredScene::shadowPass()
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    shadowShader->use();
    ShadowMap_.bindDraw();
    shadowShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    for (auto& i : OBJ_MANAGER->loaded_models)
    {
        OBJ_MANAGER->GetMesh(i)->render();
    }
    OBJ_MANAGER->GetMesh("plane")->render();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    ShadowMap_.unbindDraw();
    glViewport(0, 0, (int)screen_width, (int)screen_height);
}

void DeferredScene::plShadowPass(PointLight pl)
{
    stencilShader->use();
    PointLightShadowMap_.bindDraw();
    glDrawBuffer(GL_NONE);

    stencilShader->SetUniform("mView", view);
    stencilShader->SetUniform("projection", projection);
    stencilShader->SetUniform("worldPos", pl.position);
    stencilShader->SetUniform("radius", pl.radius);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClear(GL_DEPTH_BUFFER_BIT);
    OBJ_MANAGER->GetMesh("plane")->render();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    PointLightShadowMap_.unbindDraw();

    pointLightShader->use();
    PointLightShadowMap_.bindDraw();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    pointLightShader->SetUniform("worldPos", pl.position);

    for (int i = 0; i < 6; i++) {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, directions[i].face, PointLightShadowMap_.cubeMap, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        pointLightShader->SetUniform("mvp", projection * glm::lookAt(pl.position, pl.position + directions[i].target, directions[i].up));
        for (auto& i : OBJ_MANAGER->loaded_models)
        {
            OBJ_MANAGER->GetMesh(i)->render();
        }
    }

    PointLightShadowMap_.unbindDraw();
}

void DeferredScene::stencilPass(PointLight pl)
{
    stencilShader->use();
    gBuffer.bindDraw();
    gBuffer.setDrawNone();

    stencilShader->SetUniform("mView", view);
    stencilShader->SetUniform("projection", projection);
    stencilShader->SetUniform("worldPos", pl.position);
    stencilShader->SetUniform("radius", pl.radius);

    glEnable(GL_DEPTH_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    glClear(GL_STENCIL_BUFFER_BIT);

    //sphere.render();

    OBJ_MANAGER->GetMesh("plane")->render();

    glDisable(GL_DEPTH_TEST);

    gBuffer.unbindDraw();
}

void DeferredScene::pointLightPass(PointLight pl)
{
    lightPassShader->use();
    gBuffer.bindDraw();
    gBuffer.setDrawLight();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.color);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, PointLightShadowMap_.cubeMap);

    lightPassShader->SetUniform("positionMap", 0);
    lightPassShader->SetUniform("normalMap", 1);
    lightPassShader->SetUniform("colorMap", 2);
    lightPassShader->SetUniform("shadowMap", 3);

    lightPassShader->SetUniform("inverseMView", glm::inverse(view));
    lightPassShader->SetUniform("mView", view);
    lightPassShader->SetUniform("projection", projection);
    lightPassShader->SetUniform("worldPos", pl.position);
    lightPassShader->SetUniform("radius", pl.radius);
    lightPassShader->SetUniform("lPos", pl.position);
    lightPassShader->SetUniform("lightColor", pl.color);
    //lightPassShader->SetUniform("lightAttenuation", pl.attenuation);
    lightPassShader->SetUniform("screenSize", glm::vec2(window_width_, window_height_));

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE);

    //sphere.render();
    OBJ_MANAGER->GetMesh("plane")->render();

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    gBuffer.unbindDraw();
}

void DeferredScene::ssaoPass()
{
    ssaoShader->use();
    gBuffer.bindDraw();
    gBuffer.setDrawEffect();

    ssaoShader->SetUniform("projection", projection);
    ssaoShader->SetUniform("kernelSize", kernelSize);
    ssaoShader->SetUniform("noiseScale", noiseScale);
    ssaoShader->SetUniform("kernel", kernelSize, &kernel[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);

    ssaoShader->SetUniform("positionMap", 0);
    ssaoShader->SetUniform("normalMap", 1);
    ssaoShader->SetUniform("noiseMap", 2);

    renderQuad();

    gBuffer.unbindDraw();
}

void DeferredScene::blurPass()
{
}

void DeferredScene::compositePass()
{
    finalPassShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    finalPassShader->SetUniform("inverseMView", glm::inverse(view));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.color);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer.light);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gBuffer.effect1);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, ShadowMap_.depth);

    finalPassShader->SetUniform("positionMap", 0);
    finalPassShader->SetUniform("normalMap", 1);
    finalPassShader->SetUniform("colorMap", 2);
    finalPassShader->SetUniform("lightMap", 3);
    finalPassShader->SetUniform("ssaoMap", 4);
    finalPassShader->SetUniform("shadowMap", 5);

    finalPassShader->SetUniform("l", glm::vec3(view * light_pos_));
    finalPassShader->SetUniform("shadowMapMVP", lightProjection * lightSpaceMatrix);
    finalPassShader->SetUniform("shadowMapWidth", 2048);
    finalPassShader->SetUniform("shadowMapHeight", 2048);
    //finalPassShader->SetUniform("type", type);

    //glEnable(GL_FRAMEBUFFER_SRGB);

    glClear(GL_COLOR_BUFFER_BIT);

    renderQuad();
    //glDisable(GL_FRAMEBUFFER_SRGB);
}

void DeferredScene::skyboxPass()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getFBO());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, window_width_, window_height_, 0, 0, window_width_, window_height_, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    skyboxShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

    skyboxShader->SetUniform("skybox", 0);

    skyboxShader->SetUniform("inverseVP", glm::inverse(projection * glm::mat4(glm::mat3(view))));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    renderQuad();
    glDisable(GL_DEPTH_TEST);
}

void DeferredScene::ProcessInput(GLFWwindow* pWwindow, double dt)
{
    camera_->ProcessMouseMovement();

    if (glfwGetKey(pWwindow, GLFW_KEY_W) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_FORWARD, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_S) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_BACKWARD, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_A) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_LEFT, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_D) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_RIGHT, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_Q) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_YAW_LEFT, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_E) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_YAW_RIGHT, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_F) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_UP, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_V) == GLFW_PRESS)
        camera_->process_keyboard(Camera::Camera_Movement::CAM_DOWN, dt);
    if (glfwGetKey(pWwindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(pWwindow, true);
}


