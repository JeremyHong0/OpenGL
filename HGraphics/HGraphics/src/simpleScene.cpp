/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: simpleScene.cpp
Purpose: This file generate scene to load/render models.
Language: c++
Platform: VS2019 / Window
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include "simpleScene.h"
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <memory>
#include <queue>
#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* glsl_version = "#version 400";

SimpleScene::~SimpleScene()
{
    initMembers();
    camera_ = nullptr;
    mainShader = nullptr;
    drawNormalShader = nullptr;
    loadedModelName.clear();
    LightTypes.clear();
    UVTypes.clear();
    UVPipeline.clear();
    UVEntity.clear();
    loadedShader.clear();
    LightNums.clear();
    currentLightType.clear();
}

SimpleScene::SimpleScene(int windowWidth, int windowHeight) :
    Scene(windowWidth, windowHeight), angleOfRotation(0.0f), La_(16, glm::vec3(1.f)), Ld_(16, glm::vec3(1.f)),
    Ls_(16, glm::vec3(1.f)), spotInner(16, 15.f), spotOuter(16, 20.f), spotFalloff(16, 1.f),
    currentLightType_(16, "Point"), lightType(16, 0)
{
    initMembers();
    screen_width = static_cast<float>(windowWidth);
    screen_height = static_cast<float>(windowHeight);
}

void SimpleScene::initMembers()
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
    lightNum = 1;
    diffMap = 0;
    specMap = 0;
    fogMaxDist = 20.f;
    fogMinDist = 0.1f;
    fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
    LightTypes = { "Point", "Direction", "Spot" };
    LightNums = { "Light#1", "Light#2", "Light#3", "Light#4", "Light#5", "Light#6", "Light#7", "Light#8",
    "Light#9", "Light#10", "Light#11", "Light#12", "Light#13", "Light#14", "Light#15", "Light#16" };
    currentLightNum = "Light#1";
    currentLightType = "Point";
    UVTypes = { "Cylindrical", "Spherical", "Cube", "None" };
    currUVType = "Cylindrical";
    UVPipeline = { "CPU", "GPU" };
    currUVPipeline = "GPU";
    UVEntity = { "Position", "Normal" };
    currUVEntity = "Position";
}


int SimpleScene::Init()
{
    mainShader = std::make_unique<Shader>();
    drawNormalShader = std::make_unique<Shader>();
    lightSphereShader = std::make_unique<Shader>();
    lights_.emplace_back();

    mainShader->loadShader("shader/phongShading.vert",
        "shader/phongShading.frag");
    drawNormalShader->loadShader("shader/normalShader.vert",
        "shader/normalShader.frag");
    lightSphereShader->loadShader("shader/lightSphere.vert",
        "shader/lightSphere.frag");

    obj_manager_.loadOBJFile("models/bunny_high_poly.obj", "bunny_high_poly", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/cube.obj", "cube", true, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/4Sphere.obj", "4Sphere", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/bunny.obj", "bunny", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/cup.obj", "cup", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/cube2.obj", "cube2", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/lucy_princeton.obj", "lucy_princeton", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/rhino.obj", "rhino", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/starwars1.obj", "starwars1", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/sphere.obj", "sphere", false, Mesh::UVType::CYLINDRICAL_UV);
    obj_manager_.loadOBJFile("models/sphere_modified.obj", "sphere_modified", false, Mesh::UVType::CYLINDRICAL_UV);

    diffMap = obj_manager_.loadTexture("textures/metal_roof_diff_512x512.png");
    specMap = obj_manager_.loadTexture("textures/metal_roof_spec_512x512.png");
    gridMap = obj_manager_.loadTexture("textures/grid.png");


    obj_manager_.setupSphere("orbitSphere");
    obj_manager_.setupOrbitLine("orbitLine", orbitRadius);
    obj_manager_.setupPlane("plane");

    loadedModelName.emplace_back("bunny_high_poly");
    loadedModelName.emplace_back("cube");
    loadedModelName.emplace_back("4Sphere");
    loadedModelName.emplace_back("bunny");
    loadedModelName.emplace_back("cup");
    loadedModelName.emplace_back("cube2");
    loadedModelName.emplace_back("lucy_princeton");
    loadedModelName.emplace_back("rhino");
    loadedModelName.emplace_back("starwars1");
    loadedModelName.emplace_back("sphere");
    loadedModelName.emplace_back("sphere_modified");

    loadedShader.emplace_back("shader/phongShading");
    loadedShader.emplace_back("shader/phongLighting");
    loadedShader.emplace_back("shader/blinnShading");

    currentModelName = loadedModelName[0];
    currentVShader = loadedShader[0] + ".vert";
    currentFShader = loadedShader[0] + ".frag";
    {
        unsigned int uniformBlockIndex = glGetUniformBlockIndex(mainShader->getHandle(), "pvMatirix");
        glUniformBlockBinding(mainShader->getHandle(), uniformBlockIndex, 0);

        glGenBuffers(1, &uboMatirices);
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatirices);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatirices, 0, 2 * sizeof(glm::mat4));
    }

    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 2.9f, -8.f));
    mainShader->use();
    mainShader->SetUniform("material.diffuse", 0);
    mainShader->SetUniform("material.specular", 1);
    mainShader->SetUniform("material.grid", 0);

    return Scene::Init();
}

int SimpleScene::Render()
{
    glClearColor(fogColor.x, fogColor.y, fogColor.z, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(bShowUV)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gridMap);
    }
    else
    {
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specMap);
    }
    view = camera_->GetViewMatrix();
    projection = glm::perspective(glm::radians(camera_->Zoom), (float)screen_width / (float)screen_height, 0.1f,
                                  100.0f);
    mainShader->use();

    model = glm::mat4(1.f);
    model = glm::translate(glm::vec3(0, -0.5f, 0)) * glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    mainShader->SetUniform("model", model);
    mainShader->SetUniform("normalMatrix", normalMatrix);
    mainShader->SetUniform("bModel", false);
    obj_manager_.GetMesh("plane")->render();

    model = glm::mat4(1.f);
    model = glm::translate(glm::vec3(0, 0.3f, 0));
    mainShader->SetUniform("model", model);
    obj_manager_.GetLineMesh("orbitLine")->render();

    model = glm::mat4(1.f);
    model = glm::rotate(glm::radians(-45.f), glm::vec3(0.0f, 1.0f, 0.0f));

    mainShader->SetUniform("model", model);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    drawNormModel = model;

    mainShader->SetUniform("view", view);
    mainShader->SetUniform("projection", projection);
    mainShader->SetUniform("normalMatrix", normalMatrix);
    mainShader->SetUniform("viewPos", camera_->GetPosition());
    mainShader->SetUniform("lightNum", lightNum);
    mainShader->SetUniform("globalAmbient", globalAmbient);

    mainShader->SetUniform("Fog.MaxDist", fogMaxDist);
    mainShader->SetUniform("Fog.MinDist", fogMinDist);
    mainShader->SetUniform("Fog.Color", fogColor);
    mainShader->SetUniform("bCalcUV", bCalcUVatGPU);
    mainShader->SetUniform("bCalcPos", bCalcUVPos);
    mainShader->SetUniform("Emissive", obj_color);
    mainShader->SetUniform("Ka", Ka);
    mainShader->SetUniform("Kd", Kd);
    mainShader->SetUniform("Ks", Ks);

    mainShader->SetUniform("bShowUV", bShowUV);

    mainShader->SetUniform("min_", obj_manager_.GetMesh(currentModelName)->getMinBound());
    mainShader->SetUniform("max_", obj_manager_.GetMesh(currentModelName)->getMaxBound());
    mainShader->SetUniform("bModel", true);

    for(int i = 0; i < lightNum; ++i)
    {
        std::stringstream Light;

        Light << "Lights[" << i << "].position";

        model = glm::mat4(1.f);
        lightPos = glm::vec4(1.f);
        model = glm::rotate(angleOfRotation, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::translate(glm::vec3(cosf(glm::radians(360.f/static_cast<float>(lightNum) * i)) * orbitRadius, 0.3f,
            sinf(glm::radians(360.f / static_cast<float>(lightNum) * i)) * orbitRadius));
        LightPositions[i] = model[3];
        if (currentLightType_[i] == "Point")
        {
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].lightType", lightType[i]);
            mainShader->SetUniform(Light.str(), LightPositions[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].ambient", La_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].diffuse", Ld_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].specular", Ls_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].constant", attConst[0]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].linear", attConst[1]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].quadratic", attConst[2]);
        }

        // directionLight
        else if (currentLightType_[i] == "Direction")
        {
            mainShader->SetUniform(Light.str(), LightPositions[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].lightType", lightType[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].direction", LightPositions[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].ambient", La_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].diffuse", Ld_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].specular", Ls_[i]);
        }

        // spotLight
        else if (currentLightType_[i] == "Spot")
        {
            mainShader->SetUniform(Light.str(), LightPositions[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].lightType", lightType[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].direction", -LightPositions[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].ambient", La_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].diffuse", Ld_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].specular", Ls_[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].falloff", spotFalloff[i]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].constant", attConst[0]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].linear", attConst[1]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].quadratic", attConst[2]);
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].inner_angle", glm::cos(glm::radians(spotInner[i])));
            mainShader->SetUniform("Lights[" + std::to_string(i) + "].outer_angle", glm::cos(glm::radians(spotOuter[i])));
        }
    }
    obj_manager_.GetMesh(currentModelName)->render();

    if (bReCalcUVs)
    {
        if (currUVPipeline == "CPU")
        {
            if (currUVType == "None")
            {
                obj_manager_.GetMesh(currentModelName)->clearVertexUVs();
                obj_manager_.GetMesh(currentModelName)->setupMesh();    
            }
            if (currUVType == "Cylindrical")
            {
                if (bCalcUVPos)
                {
                    obj_manager_.GetMesh(currentModelName)->calcUVs(Mesh::UVType::CYLINDRICAL_UV);
                    obj_manager_.GetMesh(currentModelName)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(currentModelName)->calcUVs(Mesh::UVType::CYLINDRICAL_UV, false);
                    obj_manager_.GetMesh(currentModelName)->setupMesh();
                }
            }
            if (currUVType == "Spherical")
            {
                if (bCalcUVPos)
                {
                    obj_manager_.GetMesh(currentModelName)->calcUVs(Mesh::UVType::SPHERICAL_UV);
                    obj_manager_.GetMesh(currentModelName)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(currentModelName)->calcUVs(Mesh::UVType::SPHERICAL_UV, false);
                    obj_manager_.GetMesh(currentModelName)->setupMesh();
                }
            }
            if (currUVType == "Cube")
            {
                if (bCalcUVPos)
                {
                    obj_manager_.GetMesh(currentModelName)->calcUVs(Mesh::UVType::CUBE_MAPPED_UV);
                    obj_manager_.GetMesh(currentModelName)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(currentModelName)->calcUVs(Mesh::UVType::CUBE_MAPPED_UV, false);
                    obj_manager_.GetMesh(currentModelName)->setupMesh();
                }
            }
        }
        else
        {
            obj_manager_.GetMesh(currentModelName)->setupMesh();
            if (currUVType == "Cylindrical")
                mainShader->SetUniform("mappingMode", 0);
            if (currUVType == "Spherical")
                mainShader->SetUniform("mappingMode", 1);
            if (currUVType == "Cube")
                mainShader->SetUniform("mappingMode", 2);
        }
        bReCalcUVs = false;
    }

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
        lightSphereShader->SetUniform("objectColor", Ld_[i]);

        obj_manager_.GetMesh("orbitSphere")->render();
    }

    if (bShowVNormal)
    {
        drawNormalShader->use();
        drawNormalShader->SetUniform("model", drawNormModel);
        drawNormalShader->SetUniform("view", view);
        drawNormalShader->SetUniform("projection", projection);
        drawNormalShader->SetUniform("color", glm::vec3(0.32f, 0.57f, 0.86f));
        obj_manager_.GetMesh(currentModelName)->render(1);
    }
    if (bShowFNormal)
    {
        drawNormalShader->use();
        drawNormalShader->SetUniform("model", drawNormModel);
        drawNormalShader->SetUniform("view", view);
        drawNormalShader->SetUniform("projection", projection);
        drawNormalShader->SetUniform("color", glm::vec3(0.2f, 0.49f, 0.0f));
        obj_manager_.GetMesh(currentModelName)->render(2);
    }
    if(bReloadShader)
    {
        mainShader->reloadShader(currentVShader.c_str(),
            currentFShader.c_str());
        mainShader->use();
        bReloadShader = false;
    }
    return 0;
}

int SimpleScene::postRender()
{
    if(bRotate)
        angleOfRotation += 0.01f;
    return 0;
}

void SimpleScene::SetupImGUI(GLFWwindow* pWwindow)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(pWwindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void SimpleScene::RenderImGUI()
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
            for (const auto& model : loadedModelName)
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

    //Shader config
    if (ImGui::CollapsingHeader("Shader"))
    {
        static const char* current_shader = currentVShader.c_str();
        if (ImGui::BeginCombo("Current Shader", current_shader))
        {
            for (const auto& shader : loadedShader)
            {
                bool is_selected = currentVShader == shader;
                std::string currentVShaderPath = shader + ".vert";
                std::string currentFShaderPath = shader + ".frag";
                if (ImGui::Selectable(shader.c_str(), is_selected))
                {
                    currentVShader = currentVShaderPath;
                    currentFShader = currentFShaderPath;
                    current_shader = shader.c_str();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("reload shaders"))
            bReloadShader = true;
    }

    //Material config
    if (ImGui::CollapsingHeader("Material"))
    {
        ImGui::Text("Surface Color Tints");
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&Ka));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&Kd));
        ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&Ks));
        ImGui::ColorEdit3("Emissive", reinterpret_cast<float*>(&obj_color));

        ImGui::Checkbox("Visualize UV", &bShowUV);

        static const char* currentUV = currUVType.c_str();
        if (ImGui::BeginCombo("Texture projection mode", currentUV))
        {
            for (const auto& uv : UVTypes)
            {
                bool is_selected = currUVType == uv;
                if (ImGui::Selectable(uv.c_str(), is_selected))
                {
                    currUVType = uv;
                    currentUV = uv.c_str();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        static const char* calcUVMode = currUVPipeline.c_str();
        if (ImGui::BeginCombo("Texture projection pipeline", calcUVMode))
        {
            for (const auto& mode : UVPipeline)
            {
                bool is_selected = currUVPipeline == mode;
                if (ImGui::Selectable(mode.c_str(), is_selected))
                {
                    currUVPipeline = mode;
                    calcUVMode = mode.c_str();
                    if (currUVPipeline == "GPU")
                        bCalcUVatGPU = true;
                    else
                        bCalcUVatGPU = false;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        static const char* calcUVEntitiy = currUVEntity.c_str();
        if (ImGui::BeginCombo("Texture Entity", calcUVEntitiy))
        {
            for (const auto& mode : UVEntity)
            {
                bool is_selected = currUVEntity == mode;
                if (ImGui::Selectable(mode.c_str(), is_selected))
                {
                    currUVEntity = mode;
                    calcUVEntitiy = mode.c_str();
                    if (currUVEntity == "Position")
                        bCalcUVPos = true;
                    else
                        bCalcUVPos = false;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("recalculate uv"))
            bReCalcUVs = true;
    }

    //Global Constant config
    if (ImGui::CollapsingHeader("Global Constant"))
    {
        ImGui::DragFloat3("Attenuation Constant", attConst, 0.01f, 0.0f, 2.0f);
        ImGui::ColorEdit3("Global Ambient", reinterpret_cast<float*>(&globalAmbient));
        ImGui::ColorEdit3("Fog Color", reinterpret_cast<float*>(&fogColor));
        ImGui::DragFloat("Fog Min", &fogMinDist, 0.1f);
        ImGui::DragFloat("Fog Max", &fogMaxDist, 0.1f);
    }

    //Light config
    

    ImGui::End();

    ImGui::Begin("Light Controls");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    if (ImGui::CollapsingHeader("Light"))
    {
        ImGui::SliderInt("Light Count", &lightNum, 1, 16);

        ImGui::Text("Light Orbit");
        ImGui::Checkbox("Orbit Enabled", &bRotate);

        ImGui::Text("Lighting Scenarios");
        if (ImGui::Button("Scenario 1"))
        {
            lightNum = 6;
            currentLightType_[0] = "Point";
            lightType[0] = 0;
            La_[0] = glm::vec3(1.f);
            Ld_[0] = glm::vec3(1.f);
            Ls_[0] = glm::vec3(1.f);

            La_[1] = glm::vec3(1.f);
            Ld_[1] = glm::vec3(1.f);
            Ls_[1] = glm::vec3(1.f);
            currentLightType_[1] = "Point";
            lightType[1] = 0;

            La_[2] = glm::vec3(1.f);
            Ld_[2] = glm::vec3(1.f);
            Ls_[2] = glm::vec3(1.f);
            currentLightType_[2] = "Point";
            lightType[2] = 0;

            La_[3] = glm::vec3(1.f);
            Ld_[3] = glm::vec3(1.f);
            Ls_[3] = glm::vec3(1.f);
            currentLightType_[3] = "Point";
            lightType[3] = 0;

            La_[4] = glm::vec3(1.f);
            Ld_[4] = glm::vec3(1.f);
            Ls_[4] = glm::vec3(1.f);
            currentLightType_[4] = "Point";
            lightType[4] = 0;

            La_[5] = glm::vec3(1.f);
            Ld_[5] = glm::vec3(1.f);
            Ls_[5] = glm::vec3(1.f);
            currentLightType_[5] = "Point";
            lightType[5] = 0;

            scenario = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Scenario 2"))
        {
            lightNum = 7;
            currentLightType_[0] = "Spot";
            lightType[0] = 2;
            La_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            Ld_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            Ls_[0] = glm::vec3(0.230f, 0.917f, 0.081f);

            La_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            Ld_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            Ls_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            currentLightType_[1] = "Spot";
            lightType[1] = 2;

            La_[2] = glm::vec3(0.956f, 0.056f, 0.056f);
            Ld_[2] = glm::vec3(0.956f, 0.056f, 0.056f);
            Ls_[2] = glm::vec3(0.956f, 0.056f, 0.056f);
            currentLightType_[2] = "Spot";
            lightType[2] = 2;

            La_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ld_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ls_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            currentLightType_[3] = "Spot";
            lightType[3] = 2;

            La_[4] = glm::vec3(0.113f, 0.887f, 0.705f);
            Ld_[4] = glm::vec3(0.113f, 0.887f, 0.705f);
            Ls_[4] = glm::vec3(0.113f, 0.887f, 0.705f);
            currentLightType_[4] = "Spot";
            lightType[4] = 2;

            La_[5] = glm::vec3(0.887f, 0.842f, 0.113f);
            Ld_[5] = glm::vec3(0.887f, 0.842f, 0.113f);
            Ls_[5] = glm::vec3(0.887f, 0.842f, 0.113f);
            currentLightType_[5] = "Spot";
            lightType[5] = 2;

            La_[6] = glm::vec3(0.887f, 0.409f, 0.113f);
            Ld_[6] = glm::vec3(0.887f, 0.409f, 0.113f);
            Ls_[6] = glm::vec3(0.887f, 0.409f, 0.113f);
            currentLightType_[6] = "Spot";
            lightType[6] = 2;

            scenario = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Scenario 3"))
        {
            lightNum = 11;
            currentLightType_[0] = "Point";
            lightType[0] = 0;
            La_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            Ld_[0] = glm::vec3(0.230f, 0.917f, 0.081f);
            Ls_[0] = glm::vec3(0.230f, 0.917f, 0.081f);

            La_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            Ld_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            Ls_[1] = glm::vec3(0.086f, 0.350f, 0.971f);
            currentLightType_[1] = "Direction";
            lightType[1] = 1;

            La_[2] = glm::vec3(0.887f, 0.409f, 0.113f);
            Ld_[2] = glm::vec3(0.887f, 0.409f, 0.113f);
            Ls_[2] = glm::vec3(0.887f, 0.409f, 0.113f);
            currentLightType_[2] = "Spot";
            lightType[2] = 2;

            La_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ld_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ls_[3] = glm::vec3(0.795f, 0.113f, 0.887f);
            currentLightType_[3] = "Point";
            lightType[3] = 0;

            La_[4] = glm::vec3(0.887f, 0.842f, 0.113f);
            Ld_[4] = glm::vec3(0.887f, 0.842f, 0.113f);
            Ls_[4] = glm::vec3(0.887f, 0.842f, 0.113f);
            currentLightType_[4] = "Spot";
            lightType[4] = 2;

            La_[5] = glm::vec3(0.113f, 0.887f, 0.705f);
            Ld_[5] = glm::vec3(0.113f, 0.887f, 0.705f);
            Ls_[5] = glm::vec3(0.113f, 0.887f, 0.705f);
            currentLightType_[5] = "Spot";
            lightType[5] = 2;

            La_[6] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ld_[6] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ls_[6] = glm::vec3(0.795f, 0.113f, 0.887f);
            currentLightType_[6] = "Point";
            lightType[6] = 0;

            La_[7] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ld_[7] = glm::vec3(0.795f, 0.113f, 0.887f);
            Ls_[7] = glm::vec3(0.795f, 0.113f, 0.887f);
            currentLightType_[7] = "Spot";
            lightType[7] = 2;

            La_[8] = glm::vec3(0.113f, 0.887f, 0.705f);
            Ld_[8] = glm::vec3(0.113f, 0.887f, 0.705f);
            Ls_[8] = glm::vec3(0.113f, 0.887f, 0.705f);
            currentLightType_[8] = "Spot";
            lightType[8] = 2;

            La_[9] = glm::vec3(0.887f, 0.113f, 0.864f);
            Ld_[9] = glm::vec3(0.887f, 0.113f, 0.8);
            Ls_[9] = glm::vec3(0.887f, 0.113f, 0.8);
            currentLightType_[9] = "Point";
            lightType[9] = 0;

            La_[10] = glm::vec3(0.956f, 0.056f, 0.056f);
            Ld_[10] = glm::vec3(0.956f, 0.056f, 0.056f);
            Ls_[10] = glm::vec3(0.956f, 0.056f, 0.056f);
            currentLightType_[10] = "Spot";
            lightType[10] = 2;

            scenario = 0;
        }

        std::string selectedLight;
        static const char* currLightNum = currentLightNum.c_str();
        if (ImGui::BeginCombo("Select Light", currLightNum))
        {
            for (const auto& light : LightNums)
            {
                bool is_selected = currentLightNum == light;
                if (ImGui::Selectable(light.c_str(), is_selected))
                {
                    currentLightNum = light;
                    currLightNum = light.c_str();
                    selectedLight = light;
                    auto pos = selectedLight.find_first_of('#');
                    selectedLightNum = atoi(selectedLight.substr(pos + 1).c_str()) - 1;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                    
                }
            }
            ImGui::EndCombo();
        }

        std::string currentLight = currentLightType_[selectedLightNum];
        if (ImGui::BeginCombo("Light Type", currentLight.c_str()))
        {
            for (const auto& light : LightTypes)
            {
                bool is_selected = currentLightType_[selectedLightNum] == light;
                if (ImGui::Selectable(light.c_str(), is_selected))
                {
                    currentLightType_[selectedLightNum] = light;

                    if (light == "Point")
                        lightType[selectedLightNum] = 0;
                    else if (light == "Direction")
                        lightType[selectedLightNum] = 1;
                    else if (light == "Spot")
                        lightType[selectedLightNum] = 2;
                    currentLight = light.c_str();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&La_[selectedLightNum]));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&Ld_[selectedLightNum]));
        ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&Ls_[selectedLightNum]));
        if (currentLightType_[selectedLightNum] == "Spot")
        {
            ImGui::SliderFloat("Inner Angle", &spotInner[selectedLightNum], 0, 90);
            ImGui::SliderFloat("Outer Angle", &spotOuter[selectedLightNum], 0, 90);
            if (spotInner[selectedLightNum] > spotOuter[selectedLightNum])
                spotInner[selectedLightNum] = spotOuter[selectedLightNum];
            ImGui::SliderFloat("Falloff", &spotFalloff[selectedLightNum], 0, 10);
            if (spotFalloff[selectedLightNum] == 0.f)
                spotFalloff[selectedLightNum] = 1.f;
        }
    }
    ImGui::End();
}

void SimpleScene::ProcessInput(GLFWwindow* pWwindow, double dt)
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
    if (glfwGetKey(pWwindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(pWwindow, true);
}
