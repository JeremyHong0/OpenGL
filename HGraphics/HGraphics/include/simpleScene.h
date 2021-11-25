/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: simpleScene.h
Purpose: This file is header for simple scene.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
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


private:
    void initMembers();

    std::unique_ptr<Shader> main_shader_;
    std::unique_ptr<Shader> draw_normal_shader_;
    std::unique_ptr<Shader> light_sphere_shader_;
    std::unique_ptr<Shader> skybox_shader_;

    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Camera> frame_buffer_cam_[6];

    GLfloat angle_of_rotation_;

    unsigned int ubo_matirices_;

    //matrix for model rendering
    glm::mat4 model_ = glm::mat4(1.0f);
    glm::vec3 scale_ = glm::vec3(0.0f);
    glm::mat4 view_ = glm::mat4(1.0f);
    glm::mat4 projection_ = glm::mat4(1.0f);
    glm::mat3 normal_matrix_ = glm::mat3(0.0f);
    glm::vec3 global_ambient_ = glm::vec3(0.f, 0.f, 0.1f);

    //matrix for drawing normal
    glm::mat4 draw_norm_model_ = glm::mat4(1.0f);
    glm::vec3 draw_norm_scale_ = glm::vec3(0.0f);
    glm::mat4 draw_norm_view_ = glm::mat4(1.0f);
    glm::mat4 draw_norm_projection_ = glm::mat4(1.0f);

    glm::vec4 light_pos_ = glm::vec4(1.f);
    glm::vec3 obj_color_ = glm::vec3(0.f);
    glm::vec3 ka_ = glm::vec3(0.f,0.f,0.01f);
    glm::vec3 kd_ = glm::vec3(1.f);
    glm::vec3 ks_ = glm::vec3(1.f);

    float screen_width_, screen_height_;

    float orbit_radius_;

    int selected_light_num_ = 0;
    float normal_size_;
    bool b_show_v_normal_;
    bool b_show_f_normal_;
    bool b_reload_shader_;
    bool b_recalc_uv_;
    bool b_rotate_;
    bool b_calc_uv_gpu_;
    bool b_calc_uv_pos_ = true;
    bool b_show_uv_ = false;
    bool b_show_reflect_ = true;
    bool b_show_refract_;

    OBJManager obj_manager_;

    std::vector<std::string> loaded_model_name_;
    std::vector<std::string> loaded_shader_;
    std::string current_model_name_;
    std::string current_v_shader_;
    std::string current_f_shader_;
    std::string current_light_num_;
    std::vector<std::string> current_light_type_;

    glm::vec3 light_positions_[16];
    std::vector<glm::vec3> la_;
    std::vector<glm::vec3> ld_;
    std::vector<glm::vec3> ls_;
    std::vector<int> light_type_;
    std::vector<float> spot_inner_;
    std::vector<float> spot_outer_;
    std::vector<float> spot_falloff_;
    int total_light_num_;

    unsigned int diff_texture_;
    unsigned int spec_texture_;
    unsigned int grid_texture_;
    unsigned int cubemap_texture_[6];

    float fog_max_dist_;
    float fog_min_dist_;
    glm::vec3 fog_color_;
    float att_const_[4] = {1.f, 0.22f,0.2f};
    float ratio_ = 1.f;
    float mix_ratio_ = 0.5f;
    float fresnel_ = 0.5f;

    std::string current_uv_type_;
    std::string current_uv_pipeline_;
    std::string current_uv_entity_;

    unsigned int fbo_;
    unsigned int texture_colorbuffer_;
    unsigned int rbo_;
    GLuint fb_texture_[6];
    GLuint skybox_vao_;
    GLuint skybox_vbo_pos_[6];
    GLuint skybox_vbo_uv_;
    GLuint skybox_ebo_;

    enum eLightTypes
    {
        Point = 0,
        Direction,
        Spot
    };
};


#endif //SIMPLE_SCENE_H
