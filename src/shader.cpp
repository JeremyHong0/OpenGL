/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.cpp
Purpose: This file compile shader files and send uniform variables.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include "shader.hpp"

#include <vector>

Shader::Shader()
{
    m_ID = 0;
}

unsigned Shader::loadShader(const char* vertex_file_path, const char* fragment_file_path, const char* geometryPath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertex_file_path);
        fShaderFile.open(fragment_file_path);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        if (geometryPath != nullptr)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // Create Shader and Compile
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    // Check whether compile is successful
    int success = 0;


    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Error, Vertex Shader Compilation Failed ! " << infoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Error, Frag Shader Compilation Failed ! " << infoLog << std::endl;
    }

    unsigned int geometryShader;
    if (geometryPath != nullptr)
    {
        const char* gShaderCode = geometryCode.c_str();
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &gShaderCode, NULL);
        glCompileShader(geometryShader);
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            std::cout << "Error, Geometry Shader Compilation Failed ! " << infoLog << std::endl;
        }
    }

    m_ID = glCreateProgram();

    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
    if (geometryPath != nullptr)
        glAttachShader(m_ID, geometryShader);
    glLinkProgram(m_ID);

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::cout << "Error, Program Linking Failed ! " << infoLog << std::endl;
    }

    // Delete Shader after linking it to program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryPath != nullptr)
        glDeleteShader(geometryShader);

    return m_ID;
}

void Shader::reloadShader(const char* vertex_file_path, const char* fragment_file_path, const char* geometryPath)
{
    glDeleteProgram(m_ID);
    GLuint reloaded_program = loadShader(vertex_file_path, fragment_file_path, geometryPath);
    if(reloaded_program)
        m_ID = reloaded_program;
}

void Shader::use()
{
    glUseProgram(m_ID);
}

int Shader::getHandle()
{
    return m_ID;
}

void Shader::cleanup()
{

}

void Shader::SetUniform(const std::string& name, GLboolean value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform1i(location, value);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, GLint value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform1i(location, value);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, GLuint value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform1ui(location, value);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, GLfloat value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform1f(location, value);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, const GLdouble value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform1d(location, value);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, const glm::vec3& value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform3f(location, value.x, value.y, value.z);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, const glm::vec2& value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform2f(location, value.x, value.y);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, GLfloat x, GLfloat y, GLfloat z) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform3f(location, x, y, z);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, const glm::vec4& value) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform4f(location, value.x, value.y, value.z, value.w);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform4f(location, x, y, z, w);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, const glm::mat4& mat) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniformMatrix4fv(location, 1, false, &mat[0][0]);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, const glm::mat3& mat) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniformMatrix3fv(location, 1, false, &mat[0][0]);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;
}

void Shader::SetUniform(const std::string& name, GLuint numParams, const float* params) const
{
    GLint location = glGetUniformLocation(m_ID, name.c_str());

    if (location >= 0)
        glUniform3fv(location, numParams, params);
    else
        std::cout << "Uniform variable " << name << " doesn't exist" << std::endl;

}
