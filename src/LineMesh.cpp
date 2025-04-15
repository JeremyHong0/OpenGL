/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: LineMesh.cpp
Purpose: This file setup buffers, render lines.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include "LineMesh.h"

LineMesh::LineMesh()
{
    vao_ = 0;
    vbo_pos_ = 0;
    vertex_count_ = 0;
    ebo_ = 0;
}

LineMesh::~LineMesh()
{
    vertex_buffer_.clear();
    glDeleteBuffers(1, &vbo_pos_);
    glDeleteBuffers(1, &ebo_);
}

void LineMesh::render(int bFlag) const
{
    if (vao_ == 0) return;

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINE_LOOP, 0, vertex_count_);
    glBindVertexArray(0);
}

void LineMesh::setupLineMesh()
{
    vertex_count_ = (GLuint)vertex_buffer_.size();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_pos_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_.size() * sizeof(GLfloat) * 3, vertex_buffer_.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, static_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(0);
}
