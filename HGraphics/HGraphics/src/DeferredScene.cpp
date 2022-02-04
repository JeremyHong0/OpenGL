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

DeferredScene::~DeferredScene()
{
    camera_ = nullptr;
    mainShader = nullptr;
    drawNormalShader = nullptr;
    UVTypes.clear();
    UVPipeline.clear();
    UVEntity.clear();
    loadedShader.clear();
    current_light_type_.clear();
    light_type_.clear();
    la_.clear();
    ld_.clear();
    ls_.clear();
    spot_inner_.clear();
    spot_falloff_.clear();
    spot_outer_.clear();
}

DeferredScene::DeferredScene(int windowWidth, int windowHeight) :
    Scene(windowWidth, windowHeight), angleOfRotation(0.0f), la_(16, glm::vec3(1.f)), ld_(16, glm::vec3(1.f)),
    ls_(16, glm::vec3(1.f)), spot_inner_(16, 15.f), spot_outer_(16, 20.f), spot_falloff_(16, 1.f),
    current_light_type_(16, "Point"), light_type_(16, 0)
{
    initMembers();
    drawBuffer = 0;
    screen_width = static_cast<float>(windowWidth);
    screen_height = static_cast<float>(windowHeight);
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
    normalSize = 0.2f;
    lightNum = 5;
    diff_texture_ = 0;
    spec_texture_ = 0;
    diffMap = 0;
    specMap = 0;
    fogMaxDist = 20.f;
    fogMinDist = 0.1f;
    total_light_num_ = 5;
    fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
    UVTypes = { "Cylindrical", "Spherical", "Cube", "Planar", "None" };
    currUVType = "Cylindrical";
    current_light_num_ = "Light#1";
    UVPipeline = { "CPU", "GPU" };
    currUVPipeline = "GPU";
    UVEntity = { "Position", "Normal" };
    currUVEntity = "Position";
}


int DeferredScene::Init()
{
    mainShader = std::make_unique<Shader>();
    drawNormalShader = std::make_unique<Shader>();
    lightSphereShader = std::make_unique<Shader>();
    defferedShader = std::make_unique<Shader>();

    mainShader->loadShader("shader/FSQShading.vert",
        "shader/FSQShading.frag");
    drawNormalShader->loadShader("shader/normalShader.vert",
        "shader/normalShader.frag");
    lightSphereShader->loadShader("shader/lightSphere.vert",
        "shader/lightSphere.frag");
    defferedShader->loadShader("shader/deferredShading.vert",
        "shader/deferredShading.frag");

    currentModelName = OBJ_MANAGER->loaded_models[0];

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)(screen_width), (GLsizei)(screen_height), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)(screen_width), (GLsizei)(screen_height), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gUV);
    glBindTexture(GL_TEXTURE_2D, gUV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)(screen_width), (GLsizei)(screen_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gUV, 0);

    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)(screen_width), (GLsizei)(screen_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gDepth, 0);

    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

    glDrawBuffers(4, attachments);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)(screen_width), (GLsizei)(screen_height));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 1.f, -6.f));
    mainShader->use();
    mainShader->SetUniform("gPosition", 0);
    mainShader->SetUniform("gNormal", 1);
    mainShader->SetUniform("gUV", 2);
    mainShader->SetUniform("gDepth", 3);

    return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
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
    glEnable(GL_DEPTH_TEST);
    glClearColor(fogColor.x, fogColor.y, fogColor.z, 1.0f);

    view = camera_->GetViewMatrix();
    projection = glm::perspective(glm::radians(camera_->zoom_), (float)screen_width / (float)screen_height, 0.1f,
        100.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    defferedShader->use();

    model = glm::mat4(1.f);
    model = glm::rotate(glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f));

    defferedShader->SetUniform("model", model);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    drawNormModel = model;

    defferedShader->SetUniform("view", view);
    defferedShader->SetUniform("projection", projection);
    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diff_texture_);
    defferedShader->SetUniform("diffuseTexture", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, spec_texture_);
    defferedShader->SetUniform("specTexture", 1);*/
    OBJ_MANAGER->GetMesh(currentModelName)->render();

    model = glm::mat4(1.f);
    model = glm::translate(glm::vec3(0, -0.5f, 0)) * glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
    * glm::scale(glm::vec3(5, 5, 1));
    defferedShader->SetUniform("model", model);
    OBJ_MANAGER->GetMesh("quad")->render();

    model = glm::mat4(1.f);
    model = glm::translate(glm::vec3(0.f, 0.3f, 0.f));
    defferedShader->SetUniform("model", model);
    OBJ_MANAGER->GetLineMesh("orbitLine")->render();

    lightSphereShader->use();
    lightSphereShader->SetUniform("view", view);
    lightSphereShader->SetUniform("projection", projection);
    for (auto i = 0; i < lightNum; ++i)
    {
        model = glm::mat4(1.f);
        model = glm::rotate(angleOfRotation, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::translate(glm::vec3(cosf(glm::radians(360.f / static_cast<float>(lightNum) * i)) * orbitRadius, 0.3f,
                sinf(glm::radians(360.f / static_cast<float>(lightNum) * i)) * orbitRadius))
            * glm::scale(glm::vec3(0.08f));
        normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        lightSphereShader->SetUniform("model", model);
        lightSphereShader->SetUniform("objectColor", ld_[i]);

        OBJ_MANAGER->GetMesh("orbitSphere")->render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mainShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gUV);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gDepth);


    mainShader->SetUniform("viewPos", camera_->GetPosition());
    mainShader->SetUniform("lightCount", lightNum);
    mainShader->SetUniform("drawBuffer", drawBuffer);

    for (int i = 0; i < lightNum; ++i)
    {
        std::stringstream Light;

        Light << "Lights[" << i << "].position";

        model = glm::mat4(1.f);
        light_pos_ = glm::vec4(1.f);
        model = glm::rotate(angleOfRotation, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::translate(glm::vec3(cosf(glm::radians(360.f / static_cast<float>(lightNum) * i)) * orbitRadius, 0.3f,
                sinf(glm::radians(360.f / static_cast<float>(lightNum) * i)) * orbitRadius));
        light_positions_[i] = model[3];
        if (current_light_type_[i] == "Point")
        {
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].lightType", light_type_[i]);
            mainShader->SetUniform(Light.str(), light_positions_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].ambient", la_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].diffuse", ld_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].specular", ls_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].constant", att_const_[0]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].linear", att_const_[1]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].quadratic", att_const_[2]);
        }

        // directionLight
        else if (current_light_type_[i] == "Direction")
        {
            mainShader->SetUniform(Light.str(), light_positions_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].lightType", light_type_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].direction", light_positions_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].ambient", la_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].diffuse", ld_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].specular", ls_[i]);
        }

        // spotLight
        else if (current_light_type_[i] == "Spot")
        {
            mainShader->SetUniform(Light.str(), light_positions_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].lightType", light_type_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].direction", -light_positions_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].ambient", la_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].diffuse", ld_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].specular", ls_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].falloff", spot_falloff_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].constant", att_const_[0]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].linear", att_const_[1]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].quadratic", att_const_[2]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].inner_angle", glm::cos(glm::radians(spot_inner_[i])));
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].outer_angle", glm::cos(glm::radians(spot_outer_[i])));
        }
    }
    renderQuad();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, (GLint)screen_width, (GLint)screen_height, 0, 0, (GLint)screen_width, (GLint)screen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (bShowVNormal)
    {
        drawNormalShader->use();
        drawNormalShader->SetUniform("model", drawNormModel);
        drawNormalShader->SetUniform("view", view);
        drawNormalShader->SetUniform("projection", projection);
        drawNormalShader->SetUniform("color", glm::vec3(0.32f, 0.57f, 0.86f));
        OBJ_MANAGER->GetMesh(currentModelName)->render(1);
    }
    if (bShowFNormal)
    {
        drawNormalShader->use();
        drawNormalShader->SetUniform("model", drawNormModel);
        drawNormalShader->SetUniform("view", view);
        drawNormalShader->SetUniform("projection", projection);
        drawNormalShader->SetUniform("color", glm::vec3(0.2f, 0.49f, 0.0f));
        OBJ_MANAGER->GetMesh(currentModelName)->render(2);
    }
    if (bReloadShader)
    {
        defferedShader->reloadShader("shader/defferedShading.vert",
            "shader/defferedShading.frag");
        defferedShader->use();
        mainShader->reloadShader(currentVShader.c_str(),
            currentFShader.c_str());
        mainShader->use();
        bReloadShader = false;
    }
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
    if (ImGui::Button("Position"))
    {
        drawBuffer = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Normal"))
    {
        drawBuffer = 2;
    }
    ImGui::SameLine();
    if (ImGui::Button("UV"))
    {
        drawBuffer = 3;
    }
    ImGui::SameLine();
    if (ImGui::Button("Depth"))
    {
        drawBuffer = 4;
    }
    ImGui::SameLine();
    if (ImGui::Button("Final"))
    {
        drawBuffer = 0;
    }
    ImGui::End();

    //Light config
    ImGui::Begin("Light Controls");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    if (ImGui::CollapsingHeader("Light"))
    {
        ImGui::SliderInt("Light Count", &lightNum, 1, 16);

        ImGui::Text("Lighting Scenarios");
        if (ImGui::Button("Scenario 1"))
        {
            lightNum = 6;
            la_[0] = glm::vec3(1.f);
            ld_[0] = glm::vec3(1.f);
            ls_[0] = glm::vec3(1.f);
            current_light_type_[0] = "Point";
            light_type_[0] = Point;

            la_[1] = glm::vec3(1.f);
            ld_[1] = glm::vec3(1.f);
            ls_[1] = glm::vec3(1.f);
            current_light_type_[1] = "Point";
            light_type_[1] = Point;

            la_[2] = glm::vec3(1.f);
            ld_[2] = glm::vec3(1.f);
            ls_[2] = glm::vec3(1.f);
            current_light_type_[2] = "Point";
            light_type_[2] = Point;

            la_[3] = glm::vec3(1.f);
            ld_[3] = glm::vec3(1.f);
            ls_[3] = glm::vec3(1.f);
            current_light_type_[3] = "Point";
            light_type_[3] = Point;

            la_[4] = glm::vec3(1.f);
            ld_[4] = glm::vec3(1.f);
            ls_[4] = glm::vec3(1.f);
            current_light_type_[4] = "Point";
            light_type_[4] = Point;

            la_[5] = glm::vec3(1.f);
            ld_[5] = glm::vec3(1.f);
            ls_[5] = glm::vec3(1.f);
            current_light_type_[5] = "Point";
            light_type_[5] = Point;
        }
        ImGui::SameLine();
        if (ImGui::Button("Scenario 2"))
        {
            lightNum = 7;
            la_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            ld_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            ls_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            current_light_type_[0] = "Spot";
            light_type_[0] = Spot;

            la_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            ld_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            ls_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            current_light_type_[1] = "Spot";
            light_type_[1] = Spot;

            la_[2] = glm::vec3(0.956f, 0.056f, 0.056f);
            ld_[2] = glm::vec3(0.956f, 0.056f, 0.056f);
            ls_[2] = glm::vec3(0.956f, 0.056f, 0.056f);
            current_light_type_[2] = "Spot";
            light_type_[2] = Spot;

            la_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            ld_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            ls_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            current_light_type_[3] = "Spot";
            light_type_[3] = Spot;

            la_[4] = glm::vec3(0.113f, 0.887f, 0.705f);
            ld_[4] = glm::vec3(0.113f, 0.887f, 0.705f);
            ls_[4] = glm::vec3(0.113f, 0.887f, 0.705f);
            current_light_type_[4] = "Spot";
            light_type_[4] = Spot;

            la_[5] = glm::vec3(0.887f, 0.842f, 0.113f);
            ld_[5] = glm::vec3(0.887f, 0.842f, 0.113f);
            ls_[5] = glm::vec3(0.887f, 0.842f, 0.113f);
            current_light_type_[5] = "Spot";
            light_type_[5] = Spot;

            la_[6] = glm::vec3(0.887f, 0.409f, 0.113f);
            ld_[6] = glm::vec3(0.887f, 0.409f, 0.113f);
            ls_[6] = glm::vec3(0.887f, 0.409f, 0.113f);
            current_light_type_[6] = "Spot";
            light_type_[6] = Spot;

        }
        ImGui::SameLine();
        if (ImGui::Button("Scenario 3"))
        {
            lightNum = 11;
            current_light_type_[0] = "Point";
            light_type_[0] = Point;
            la_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            ld_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            ls_[0] = glm::vec3(0.230f, 0.917f, 0.081f);

            la_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            ld_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            ls_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            current_light_type_[1] = "Direction";
            light_type_[1] = Direction;

            la_[2] = glm::vec3(0.887f, 0.409f, 0.113f);
            ld_[2] = glm::vec3(0.887f, 0.409f, 0.113f);
            ls_[2] = glm::vec3(0.887f, 0.409f, 0.113f);
            current_light_type_[2] = "Spot";
            light_type_[2] = Spot;

            la_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            ld_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            ls_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            current_light_type_[3] = "Point";
            light_type_[3] = Point;

            la_[4] = glm::vec3(0.887f, 0.842f, 0.113f);
            ld_[4] = glm::vec3(0.887f, 0.842f, 0.113f);
            ls_[4] = glm::vec3(0.887f, 0.842f, 0.113f);
            current_light_type_[4] = "Spot";
            light_type_[4] = Spot;

            la_[5] = glm::vec3(0.113f, 0.887f, 0.705f);
            ld_[5] = glm::vec3(0.113f, 0.887f, 0.705f);
            ls_[5] = glm::vec3(0.113f, 0.887f, 0.705f);
            current_light_type_[5] = "Spot";
            light_type_[5] = Spot;

            la_[6] = glm::vec3(0.795f, 0.113f, 0.887f);
            ld_[6] = glm::vec3(0.795f, 0.113f, 0.887f);
            ls_[6] = glm::vec3(0.795f, 0.113f, 0.887f);
            current_light_type_[6] = "Point";
            light_type_[6] = Point;

            la_[7] = glm::vec3(0.795f, 0.113f, 0.887f);
            ld_[7] = glm::vec3(0.795f, 0.113f, 0.887f);
            ls_[7] = glm::vec3(0.795f, 0.113f, 0.887f);
            current_light_type_[7] = "Spot";
            light_type_[7] = Spot;

            la_[8] = glm::vec3(0.113f, 0.887f, 0.705f);
            ld_[8] = glm::vec3(0.113f, 0.887f, 0.705f);
            ls_[8] = glm::vec3(0.113f, 0.887f, 0.705f);
            current_light_type_[8] = "Spot";
            light_type_[8] = Spot;

            la_[9] = glm::vec3(0.887f, 0.113f, 0.864f);
            ld_[9] = glm::vec3(0.887f, 0.113f, 0.8);
            ls_[9] = glm::vec3(0.887f, 0.113f, 0.8);
            current_light_type_[9] = "Point";
            light_type_[9] = Point;

            la_[10] = glm::vec3(0.956f, 0.056f, 0.056f);
            ld_[10] = glm::vec3(0.956f, 0.056f, 0.056f);
            ls_[10] = glm::vec3(0.956f, 0.056f, 0.056f);
            current_light_type_[10] = "Spot";
            light_type_[10] = Spot;

        }

        static const char* currLightNum = current_light_num_.c_str();
        const char* lights[] = { "Light#1", "Light#2", "Light#3", "Light#4", "Light#5", "Light#6", "Light#7", "Light#8",
    "Light#9", "Light#10", "Light#11", "Light#12", "Light#13", "Light#14", "Light#15", "Light#16" };
        if (ImGui::BeginCombo("Select Light", currLightNum))
        {
            for (const auto& light : lights)
            {
                bool is_selected = current_light_num_ == light;
                if (ImGui::Selectable(light, is_selected))
                {
                    current_light_num_ = light;
                    currLightNum = light;
                    std::string selectedLight = light;
                    auto pos = selectedLight.find_first_of('#');
                    selected_light_num_ = atoi(selectedLight.substr(pos + 1).c_str()) - 1;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();

                }
            }
            ImGui::EndCombo();
        }

        std::string currentLight = current_light_type_[selected_light_num_];
        const char* light_types[] = { "Point", "Direction", "Spot" };
        if (ImGui::BeginCombo("Light Type", currentLight.c_str()))
        {
            for (const auto& light : light_types)
            {
                bool is_selected = current_light_type_[selected_light_num_] == light;
                if (ImGui::Selectable(light, is_selected))
                {
                    current_light_type_[selected_light_num_] = light;

                    if (light == "Point")
                        light_type_[selected_light_num_] = Point;
                    else if (light == "Direction")
                        light_type_[selected_light_num_] = Direction;
                    else if (light == "Spot")
                        light_type_[selected_light_num_] = Spot;

                    currentLight = light;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&la_[selected_light_num_]));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&ld_[selected_light_num_]));
        ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&ls_[selected_light_num_]));
        if (current_light_type_[selected_light_num_] == "Spot")
        {
            ImGui::SliderFloat("Inner Angle", &spot_inner_[selected_light_num_], 0, 90);
            ImGui::SliderFloat("Outer Angle", &spot_outer_[selected_light_num_], 0, 90);
            if (spot_inner_[selected_light_num_] > spot_outer_[selected_light_num_])
                spot_inner_[selected_light_num_] = spot_outer_[selected_light_num_];
            ImGui::SliderFloat("Falloff", &spot_falloff_[selected_light_num_], 0, 10);
            if (spot_falloff_[selected_light_num_] == 0.f)
                spot_falloff_[selected_light_num_] = 1.f;
        }
    }
    ImGui::End();

    return Scene::RenderImGUI();
}

void DeferredScene::ProcessInput(GLFWwindow* pWwindow, double dt)
{
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
