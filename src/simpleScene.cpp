/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: simpleScene.cpp
Purpose: This file generate scene to load/render models.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include "simpleScene.h"
#define STB_IMAGE_IMPLEMENTATION

#include <array>

#include "stb_image.h"
#include <memory>
#include <queue>
#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* glsl_version = "#version 450";

SimpleScene::~SimpleScene()
{
    loaded_model_name_.clear();
    loaded_shader_.clear();
    initMembers();
    glDeleteVertexArrays(1, &skybox_vao_);
    glDeleteBuffers(1, skybox_vbo_pos_);
    glDeleteBuffers(1, &skybox_vbo_uv_);
    glDeleteBuffers(1, &skybox_ebo_);
    glDeleteBuffers(1, &fbo_);
    glDeleteBuffers(1, &rbo_);
}

SimpleScene::SimpleScene(int windowWidth, int windowHeight) :
    Scene(windowWidth, windowHeight), angle_of_rotation_(0.0f), current_light_type_(16, "Point"),
    la_(16, glm::vec3(1.f)),
    ld_(16, glm::vec3(1.f)), ls_(16, glm::vec3(1.f)),
	light_type_(16, 0), spot_inner_(16, 15.f),
    spot_outer_(16, 20.f), spot_falloff_(16, 1.f)
{
    initMembers();
    screen_width_ = static_cast<float>(windowWidth);
    screen_height_ = static_cast<float>(windowHeight);
}

void SimpleScene::initMembers()
{
    angle_of_rotation_ = 0.0f;
    orbit_radius_ = 2.5f;
    b_show_v_normal_ = false;
    b_show_f_normal_ = false;
    b_reload_shader_ = false;
    b_recalc_uv_ = false;
    b_rotate_ = true;
    b_calc_uv_gpu_ = true;
    normal_size_ = 0.2f;
    total_light_num_ = 1;
    diff_texture_ = 0;
    spec_texture_ = 0;
    fog_max_dist_ = 20.f;
    fog_min_dist_ = 0.1f;
    fog_color_ = glm::vec3(0.5f, 0.5f, 0.5f);
    current_light_num_ = "Light#1";
    current_uv_type_ = "Cylindrical";
    current_uv_pipeline_ = "GPU";
    current_uv_entity_ = "Position";
}


int SimpleScene::Init(GLFWwindow* pWindow)
{
    main_shader_ = std::make_unique<Shader>();
    draw_normal_shader_ = std::make_unique<Shader>();
    light_sphere_shader_ = std::make_unique<Shader>();
    skybox_shader_ = std::make_unique<Shader>();

    main_shader_->loadShader("../assets/shader/phongShading.vert",
        "../assets/shader/phongShading.frag");
    draw_normal_shader_->loadShader("../assets/shader/normalShader.vert",
        "../assets/shader/normalShader.frag");
    light_sphere_shader_->loadShader("../assets/shader/lightSphere.vert",
        "../assets/shader/lightSphere.frag");
    skybox_shader_->loadShader("../assets/shader/skybox.vert",
        "../assets/shader/skybox.frag");

    diff_texture_ = obj_manager_.getTexture("diffTexture");
    spec_texture_ = obj_manager_.getTexture("specTexture");
    grid_texture_ = obj_manager_.getTexture("gridTexture");
    std::array<std::string, 6> faces = { {
	    "../assets/textures/left.jpg",
	    "../assets/textures/right.jpg",
	    "../assets/textures/bottom.jpg",
	    "../assets/textures/top.jpg",
	    "../assets/textures/back.jpg",
	    "../assets/textures/front.jpg"
    } };

    for (int i = 0; i < 6; ++i)
    {
        cubemap_texture_[i] = obj_manager_.load_cubemap(faces[i]);
    }

    frame_buffer_cam_[0] = std::make_unique<Camera>(glm::vec3(0.0f, 0.f, 0.f), glm::vec3(-1.0f, 0.0f, 0.0f));
    frame_buffer_cam_[1] = std::make_unique<Camera>(glm::vec3(0.0f, 0.f, 0.f), glm::vec3(1.0f, 0.0f, 0.0f));
    frame_buffer_cam_[2] = std::make_unique<Camera>(glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.f, 0.f, -1.f));
    frame_buffer_cam_[3] = std::make_unique<Camera>(glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.f, 0.f, 1.f));
    frame_buffer_cam_[4] = std::make_unique<Camera>(glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.0f, 0.0f, -1.0f));
    frame_buffer_cam_[5] = std::make_unique<Camera>(glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.0f, 0.0f, 1.0f));

    const std::array<std::array<glm::vec3, 4>, 6> skyboxVertices = { {
        // Left face
        {{
            {-1.0f, -1.0f,  1.0f},
            {-1.0f, -1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f},
            {-1.0f,  1.0f,  1.0f}
        }},
        // Right face
        {{
            { 1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f},
            { 1.0f,  1.0f, -1.0f}
        }},
        // Bottom face
        {{
            {-1.0f, -1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f},
            { 1.0f, -1.0f, -1.0f},
            {-1.0f, -1.0f, -1.0f}
        }},
        // Top face
        {{
            {-1.0f,  1.0f, -1.0f},
            { 1.0f,  1.0f, -1.0f},
            { 1.0f,  1.0f,  1.0f},
            {-1.0f,  1.0f,  1.0f}
        }},
        // Back face
        {{
            { 1.0f, -1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f},
            {-1.0f,  1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f}
        }},
        // Front face
        {{
            {-1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f, -1.0f},
            { 1.0f,  1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f}
        }}
    } };
    const std::array<glm::vec2, 4> skyboxUV = { {
	    {0.0f, 1.0f},
	    {1.0f, 1.0f},
	    {1.0f, 0.0f},
	    {0.0f, 0.0f}
	} };

    glGenVertexArrays(1, &skybox_vao_);
    for (int i = 0; i < 6; ++i)
    {
        glGenBuffers(1, &skybox_vbo_pos_[i]);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_pos_[i]);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), skyboxVertices[i].data(), GL_STATIC_DRAW);
        glGenBuffers(1, &skybox_vbo_uv_);
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_uv_);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), skyboxUV.data(), GL_STATIC_DRAW);
    }

    glGenBuffers(1, &skybox_ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_manager_.GetMesh("quad")->getIndexBufferSize() * sizeof(glm::ivec3), obj_manager_.GetMesh("quad")->getIndexBuffer(), GL_STATIC_DRAW);

    loaded_shader_.emplace_back("../assets/shader/phongShading");
    loaded_shader_.emplace_back("../assets/shader/blinnShading");
    loaded_shader_.emplace_back("../assets/shader/phongLighting");

    current_model_name_ = obj_manager_.loaded_models[0];
    current_v_shader_ = loaded_shader_[0] + ".vert";
    current_f_shader_ = loaded_shader_[0] + ".frag";

    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 0.5f, -6.f));

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    for (int i = 0; i < 6; ++i)
    {
        glGenTextures(1, &fb_texture_[i]);
        glBindTexture(GL_TEXTURE_2D, fb_texture_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)screen_width_, (GLsizei)screen_height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)screen_width_, (GLsizei)screen_height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    main_shader_->SetUniform("mappingMode", 2);

    return Scene::Init(pWindow);
}

int SimpleScene::Render()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(fog_color_.x, fog_color_.y, fog_color_.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    view_ = camera_->GetViewMatrix();
    projection_ = glm::perspective(glm::radians(camera_->zoom_), (float)screen_width_ / (float)screen_height_, 0.1f,
        100.0f);
    skybox_shader_->use();
    skybox_shader_->SetUniform("view", view_);
    skybox_shader_->SetUniform("projection", projection_);

    glBindVertexArray(skybox_vao_);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    for (int i = 0; i < 6; ++i)
    {
        glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_pos_[i]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_uv_);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubemap_texture_[i]);
        skybox_shader_->SetUniform("skybox", 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_ebo_);
        glDrawElements(GL_TRIANGLES, obj_manager_.GetMesh("quad")->getIndexBufferSize(), GL_UNSIGNED_INT, 0);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    main_shader_->use();

    if (b_show_uv_)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grid_texture_);
    }
    else
    {
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diff_texture_);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, spec_texture_);
    }

    model_ = glm::mat4(1.f);
    main_shader_->SetUniform("model", model_);
    normal_matrix_ = glm::mat3(glm::transpose(glm::inverse(model_)));
    draw_norm_model_ = model_;

    main_shader_->SetUniform("view", view_);
    main_shader_->SetUniform("projection", projection_);
    main_shader_->SetUniform("normalMatrix", normal_matrix_);
    main_shader_->SetUniform("viewPos", camera_->GetPosition());
    main_shader_->SetUniform("lightNum", total_light_num_);
    main_shader_->SetUniform("globalAmbient", global_ambient_);

    main_shader_->SetUniform("Fog.MaxDist", fog_max_dist_);
    main_shader_->SetUniform("Fog.MinDist", fog_min_dist_);
    main_shader_->SetUniform("Fog.Color", fog_color_);
    main_shader_->SetUniform("bCalcUV", b_calc_uv_gpu_);
    main_shader_->SetUniform("bCalcPos", b_calc_uv_pos_);
    main_shader_->SetUniform("Emissive", obj_color_);
    main_shader_->SetUniform("Ka", ka_);
    main_shader_->SetUniform("Kd", kd_);
    main_shader_->SetUniform("Ks", ks_);

    main_shader_->SetUniform("bShowUV", b_show_uv_);
    main_shader_->SetUniform("bShowReflect", b_show_reflect_);
    main_shader_->SetUniform("bShowRefract", b_show_refract_);

    main_shader_->SetUniform("min_", obj_manager_.GetMesh(current_model_name_)->getMinBound());
    main_shader_->SetUniform("max_", obj_manager_.GetMesh(current_model_name_)->getMaxBound());
    main_shader_->SetUniform("bModel", true);

    for (int i = 0; i < total_light_num_; ++i)
    {
        std::stringstream Light;

        Light << "Lights[" << i << "].position";

        model_ = glm::mat4(1.f);
        light_pos_ = glm::vec4(1.f);
        model_ = glm::rotate(angle_of_rotation_, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::translate(glm::vec3(cosf(glm::radians(360.f / static_cast<float>(total_light_num_) * i)) * orbit_radius_, 0.3f,
                sinf(glm::radians(360.f / static_cast<float>(total_light_num_) * i)) * orbit_radius_));
        light_positions_[i] = model_[3];
        if (current_light_type_[i] == "Point")
        {
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].lightType", light_type_[i]);
            main_shader_->SetUniform(Light.str(), light_positions_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].ambient", la_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].diffuse", ld_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].specular", ls_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].constant", att_const_[0]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].linear", att_const_[1]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].quadratic", att_const_[2]);
        }

        // directionLight
        else if (current_light_type_[i] == "Direction")
        {
            main_shader_->SetUniform(Light.str(), light_positions_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].lightType", light_type_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].direction", light_positions_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].ambient", la_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].diffuse", ld_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].specular", ls_[i]);
        }

        // spotLight
        else if (current_light_type_[i] == "Spot")
        {
            main_shader_->SetUniform(Light.str(), light_positions_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].lightType", light_type_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].direction", -light_positions_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].ambient", la_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].diffuse", ld_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].specular", ls_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].falloff", spot_falloff_[i]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].constant", att_const_[0]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].linear", att_const_[1]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].quadratic", att_const_[2]);
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].inner_angle", glm::cos(glm::radians(spot_inner_[i])));
            main_shader_->SetUniform("Lights[" + std::to_string(i) + "].outer_angle", glm::cos(glm::radians(spot_outer_[i])));
        }
    }
    obj_manager_.GetMesh(current_model_name_)->render();

    if (b_recalc_uv_)
    {
        if (current_uv_pipeline_ == "CPU")
        {
            if (current_uv_type_ == "None")
            {
                obj_manager_.GetMesh(current_model_name_)->clearVertexUVs();
                obj_manager_.GetMesh(current_model_name_)->setupMesh();
            }
            else if (current_uv_type_ == "Cylindrical")
            {
                if (b_calc_uv_pos_)
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::CYLINDRICAL_UV);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::CYLINDRICAL_UV, false);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
            }
            else if (current_uv_type_ == "Spherical")
            {
                if (b_calc_uv_pos_)
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::SPHERICAL_UV);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::SPHERICAL_UV, false);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
            }
            else if (current_uv_type_ == "Cube")
            {
                if (b_calc_uv_pos_)
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::CUBE_MAPPED_UV);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::CUBE_MAPPED_UV, false);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
            }
            else if (current_uv_type_ == "Planar")
            {
                if (b_calc_uv_pos_)
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::PLANAR_UV);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
                else
                {
                    obj_manager_.GetMesh(current_model_name_)->calcUVs(Mesh::UVType::PLANAR_UV, false);
                    obj_manager_.GetMesh(current_model_name_)->setupMesh();
                }
            }
        }
        else
        {
            obj_manager_.GetMesh(current_model_name_)->setupMesh();
            if (current_uv_type_ == "Cylindrical")
                main_shader_->SetUniform("mappingMode", 0);
            if (current_uv_type_ == "Spherical")
                main_shader_->SetUniform("mappingMode", 1);
            if (current_uv_type_ == "Cube")
                main_shader_->SetUniform("mappingMode", 2);
            if (current_uv_type_ == "Planar")
                main_shader_->SetUniform("mappingMode", 3);
        }
        b_recalc_uv_ = false;
    }

    //Enviroment mapping
    if (b_show_reflect_|| b_show_refract_)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        main_shader_->use();

        main_shader_->SetUniform("fresnel", fresnel_);
        main_shader_->SetUniform("inputRatio", ratio_);
        main_shader_->SetUniform("mixRatio", mix_ratio_);
        
        for (int i = 0; i < 6; ++i)
        {
            glActiveTexture(GL_TEXTURE4 + i);
            glBindTexture(GL_TEXTURE_2D, fb_texture_[i]);
            main_shader_->SetUniform("cube[" + std::to_string(i) + "]", 4 + i);
        }
        for (int i = 0; i < 6; ++i)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture_[i], 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 envProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
            glm::mat4 envView = frame_buffer_cam_[i]->GetViewMatrix();
            glm::mat4 envModel(1.0f);

            main_shader_->SetUniform("model", envModel);
            main_shader_->SetUniform("view", envView);
            main_shader_->SetUniform("projection", envProj);

            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            skybox_shader_->use();
            glm::mat4 skyboxView = glm::mat4(glm::mat3(envView));
            skybox_shader_->SetUniform("view", skyboxView);
            skybox_shader_->SetUniform("projection", envProj);

            glBindVertexArray(skybox_vao_);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            for (int j = 0; j < 6; ++j)
            {
                glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_pos_[j]);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));

                glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo_uv_);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<void*>(0));

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, cubemap_texture_[j]);
                if (cubemap_texture_[i] == 0) {
                    std::cerr << "ERROR::CUBEMAP:: Texture " << i << " is not loaded correctly!" << std::endl;
                }

                skybox_shader_->SetUniform("skybox", 0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_ebo_);
                glDrawElements(GL_TRIANGLES, obj_manager_.GetMesh("quad")->getIndexBufferSize(), GL_UNSIGNED_INT, 0);
            }

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            light_sphere_shader_->use();
            light_sphere_shader_->SetUniform("view", envView);
            light_sphere_shader_->SetUniform("projection", envProj);
            for (auto j = 0; j < total_light_num_; ++j)
            {
                model_ = glm::mat4(1.f);
                model_ = glm::rotate(angle_of_rotation_, glm::vec3(0.0f, 1.0f, 0.0f)) *
                    glm::translate(glm::vec3(cosf(glm::radians(360.f / static_cast<float>(total_light_num_) * j)) * orbit_radius_, 0.0f,
                        sinf(glm::radians(360.f / static_cast<float>(total_light_num_) * j)) * orbit_radius_))
                    * glm::scale(glm::vec3(0.08f));
                normal_matrix_ = glm::mat3(glm::transpose(glm::inverse(model_)));
                light_sphere_shader_->SetUniform("model", model_);
                light_sphere_shader_->SetUniform("objectColor", ld_[j]);

                obj_manager_.GetMesh("orbitSphere")->render();
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    light_sphere_shader_->use();
    light_sphere_shader_->SetUniform("view", view_);
    light_sphere_shader_->SetUniform("projection", projection_);
    model_ = glm::mat4(1.f);
    light_sphere_shader_->SetUniform("model", model_);
    obj_manager_.GetLineMesh("orbitLine")->render();

    for (auto i = 0; i < total_light_num_; ++i)
    {
        model_ = glm::mat4(1.f);
        model_ = glm::rotate(angle_of_rotation_, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::translate(glm::vec3(cosf(glm::radians(360.f / static_cast<float>(total_light_num_) * i)) * orbit_radius_, 0.0f,
                sinf(glm::radians(360.f / static_cast<float>(total_light_num_) * i)) * orbit_radius_))
            * glm::scale(glm::vec3(0.08f));
        normal_matrix_ = glm::mat3(glm::transpose(glm::inverse(model_)));
        light_sphere_shader_->SetUniform("model", model_);
        light_sphere_shader_->SetUniform("objectColor", ld_[i]);

        obj_manager_.GetMesh("orbitSphere")->render();
    }


    if (b_show_v_normal_)
    {
        draw_normal_shader_->use();
        draw_normal_shader_->SetUniform("model", draw_norm_model_);
        draw_normal_shader_->SetUniform("view", view_);
        draw_normal_shader_->SetUniform("projection", projection_);
        draw_normal_shader_->SetUniform("color", glm::vec3(0.32f, 0.57f, 0.86f));
        obj_manager_.GetMesh(current_model_name_)->render(1);
    }
    if (b_show_f_normal_)
    {
        draw_normal_shader_->use();
        draw_normal_shader_->SetUniform("model", draw_norm_model_);
        draw_normal_shader_->SetUniform("view", view_);
        draw_normal_shader_->SetUniform("projection", projection_);
        draw_normal_shader_->SetUniform("color", glm::vec3(0.2f, 0.49f, 0.0f));
        obj_manager_.GetMesh(current_model_name_)->render(2);
    }
    if (b_reload_shader_)
    {
        main_shader_->reloadShader(current_v_shader_.c_str(),
            current_f_shader_.c_str());
        main_shader_->use();
        b_reload_shader_ = false;
    }

    return 0;
}

int SimpleScene::postRender()
{
    if (b_rotate_)
        angle_of_rotation_ += 0.01f;
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

int SimpleScene::RenderImGUI()
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
        static const char* current_item = current_model_name_.c_str();
        if (ImGui::BeginCombo("Loaded Model", current_item))
        {
            for (const auto& model : obj_manager_.loaded_models)
            {
                const bool is_selected = current_model_name_ == model;
                if (ImGui::Selectable(model.c_str(), is_selected))
                {
                    current_model_name_ = model;
                    current_item = model.c_str();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::Checkbox("Draw Vertex Normal", &b_show_v_normal_);
        ImGui::Checkbox("Draw Face Normal", &b_show_f_normal_);
    }

    //Shader config
    if (ImGui::CollapsingHeader("Shader"))
    {
        static const char* current_shader = current_v_shader_.c_str();
        if (ImGui::BeginCombo("Current Shader", current_shader))
        {
            for (const auto& shader : loaded_shader_)
            {
                const bool is_selected = current_v_shader_ == shader;
                const std::string&& currentVShaderPath = shader + ".vert";
                const std::string&& currentFShaderPath = shader + ".frag";
                if (ImGui::Selectable(shader.c_str(), is_selected))
                {
                    current_v_shader_ = currentVShaderPath;
                    current_f_shader_ = currentFShaderPath;
                    current_shader = shader.c_str();
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("reload shader"))
            b_reload_shader_ = true;
    }

    //Material config
    if (ImGui::CollapsingHeader("Material"))
    {
        ImGui::Text("Surface Color Tints");
        ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&ka_));
        ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&kd_));
        ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&ks_));
        ImGui::ColorEdit3("Emissive", reinterpret_cast<float*>(&obj_color_));

        ImGui::Checkbox("Visualize UV", &b_show_uv_);
        ImGui::Checkbox("Visualize Reflection", &b_show_reflect_);
        ImGui::Checkbox("Visualize Refraction", &b_show_refract_);
        ImGui::DragFloat("Ratio", &ratio_, 0.05f, 0.0f, 100.0f, "%f");
        ImGui::DragFloat("Fresnel Constant", &fresnel_, 0.001f, 0.01f, 1.0f, "%f");
        ImGui::DragFloat("Mix Ratio", &mix_ratio_, 0.01f, 0.0f, 1.0f, "%f");

        const char* refractionMaterials[] = { "Air", "Hydrogen", "Water", "Olive Oil", "Ice",
            "Quartz", "Diamond", "Sapphire", "Moissanite","Acrylic" };

        static int currMaterial = 0;
        if (ImGui::Combo("Refraction Materials", &currMaterial, refractionMaterials, IM_ARRAYSIZE(refractionMaterials)))
        {
            switch (currMaterial)
            {
            case 0:
                ratio_ = 1.000293f;
                break;
            case 1:
                ratio_ = 1.000132f;
                break;
            case 2:
                ratio_ = 1.333f;
                break;
            case 3:
                ratio_ = 1.47f;
                break;
            case 4:
                ratio_ = 1.31f;
                break;
            case 5:
                ratio_ = 1.46f;
                break;
            case 6:
                ratio_ = 2.417f;
                break;
            case 7:
                ratio_ = 1.77f;
                break;
            case 8:
                ratio_ = 2.65f;
                break;
            default:
                ratio_ = 1.49f;
                break;
            }
        }

        static const char* currentUV = current_uv_type_.c_str();
        const char* uv_types[] = { "Cylindrical", "Spherical", "Cube", "Planar", "None" };
        if (ImGui::BeginCombo("Texture projection mode", currentUV))
        {
            for (const auto& uv : uv_types)
            {
                bool is_selected = current_uv_type_ == uv;
                if (ImGui::Selectable(uv, is_selected))
                {
                    b_recalc_uv_ = true;
                    current_uv_type_ = uv;
                    currentUV = uv;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        static const char* calcUVMode = current_uv_pipeline_.c_str();
        if (ImGui::BeginCombo("Texture projection pipeline", calcUVMode))
        {
            const char* uv_pipelines[] = { "CPU", "GPU" };
            for (const auto& mode : uv_pipelines)
            {
                bool is_selected = current_uv_pipeline_ == mode;
                if (ImGui::Selectable(mode, is_selected))
                {
                    b_recalc_uv_ = true;
                    current_uv_pipeline_ = mode;
                    calcUVMode = mode;
                    if (current_uv_pipeline_ == "GPU")
                        b_calc_uv_gpu_ = true;
                    else
                        b_calc_uv_gpu_ = false;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        static const char* calcUVEntity = current_uv_entity_.c_str();
        if (ImGui::BeginCombo("Texture Entity", calcUVEntity))
        {
            const char* uv_entities[] = { "Position", "Normal" };
            for (const auto& mode : uv_entities)
            {
                const bool is_selected = current_uv_entity_ == mode;
                if (ImGui::Selectable(mode, is_selected))
                {
                    b_recalc_uv_ = true;
                    current_uv_entity_ = mode;
                    if (current_uv_entity_ == "Position")
                        b_calc_uv_pos_ = true;
                    else
                        b_calc_uv_pos_ = false;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("recalculate uv"))
            b_recalc_uv_ = true;
    }

    //Global Constant config
    if (ImGui::CollapsingHeader("Global Constant"))
    {
        ImGui::DragFloat3("Attenuation Constant", att_const_, 0.01f, 0.0f, 2.0f);
        ImGui::ColorEdit3("Global Ambient", reinterpret_cast<float*>(&global_ambient_));
        ImGui::ColorEdit3("Fog Color", reinterpret_cast<float*>(&fog_color_));
        ImGui::DragFloat("Fog Min", &fog_min_dist_, 0.1f);
        ImGui::DragFloat("Fog Max", &fog_max_dist_, 0.1f);
    }
    ImGui::End();

    //Light config
    ImGui::Begin("Light Controls");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    if (ImGui::CollapsingHeader("Light"))
    {
        ImGui::SliderInt("Light Count", &total_light_num_, 1, 16);

        ImGui::Text("Light Orbit");
        ImGui::Checkbox("Orbit Enabled", &b_rotate_);

        ImGui::Text("Lighting Scenarios");
        if (ImGui::Button("Scenario 1"))
        {
            total_light_num_ = 6;
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
            total_light_num_ = 7;
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
            total_light_num_ = 11;
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

        const char* currLightNum = current_light_num_.c_str();
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
