/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.hpp
Purpose: This file is header for shader.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef SHADER_HPP
#define SHADER_HPP
#include <glad/glad.h>  // include glad to get all the required OpenGL headers
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // the program ID
    unsigned m_ID;

    // constructor reads and builds the shader
    Shader();

    unsigned loadShader(const char* vertex_file_path, const char* fragment_file_path, const char* geometryPath = nullptr);
    void reloadShader(const char* vertex_file_path, const char* fragment_file_path, const char* geometryPath = nullptr);

    // use/activate the shader
    void use();
    int getHandle();
    void cleanup();

    // utility uniform functions
    void SetUniform(const std::string& name, GLboolean value) const;
    void SetUniform(const std::string& name, GLint value) const;
    void SetUniform(const std::string& name, GLuint value) const;
    void SetUniform(const std::string& name, GLfloat value) const;
    void SetUniform(const std::string& name, const GLdouble value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, GLfloat x, GLfloat y, GLfloat z) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;
    void SetUniform(const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const;
    void SetUniform(const std::string& name, const glm::mat4& mat) const;
    void SetUniform(const std::string& name, const glm::mat3& mat) const;
    void SetUniform(const std::string& name, GLuint numParams, const float* params) const;


private:
    std::string vertex_code_;
    std::string fragment_code_;
    std::string geometry_code_;
    std::ifstream v_shader_file_;
    std::ifstream f_shader_file_;
    std::ifstream g_shader_file_;
};
#endif
