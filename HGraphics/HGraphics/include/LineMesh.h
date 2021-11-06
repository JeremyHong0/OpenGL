/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: LineMesh.h
Purpose: This file is header for line mesh.
Language: c++
Platform: VS2019 / Window
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef LINE_MESH_H
#define LINE_MESH_H

#include "mesh.h"

class LineMesh : public Mesh
{
    friend class OBJManager;
public:

    LineMesh();
    ~LineMesh();

    void render(int bFlag = 0) const override;
    void setupLineMesh();

private:
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint vertexCount;


    std::vector<glm::vec3> vertexBuffer;
};


#endif
