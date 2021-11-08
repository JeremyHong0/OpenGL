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
    VAO = 0;
    VBO = 0;
    vertexCount = 0;
    EBO = 0;
}

LineMesh::~LineMesh()
{
    vertexBuffer.clear();
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void LineMesh::render(int bFlag) const
{
    if (VAO == 0) return;

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
    glBindVertexArray(0);
}

void LineMesh::setupLineMesh()
{
    vertexCount = (GLuint)vertexBuffer.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(GLfloat) * 3, vertexBuffer.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(0);
}
