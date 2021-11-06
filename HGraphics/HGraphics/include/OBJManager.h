/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: OBJManager.h
Purpose: This file is header for object manager.
Language: c++
Platform: VS2019 / Window
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef OBJ_MANAGER_H
#define OBJ_MANAGER_H

#include <string>
#include <fstream>
#include <vector>

#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "mesh.h"
#include "LineMesh.h"

class OBJManager
{
public:
    OBJManager();
    virtual ~OBJManager();

    // initialize the data
    void initData();
    Mesh* GetMesh(const std::string& name);
    LineMesh* GetLineMesh(const std::string& name);

    // Read data from a file
    enum class ReadMethod { LINE_BY_LINE, BLOCK_IO };

    double ReadOBJFile(std::string filepath,
                       Mesh* pMesh, Mesh::UVType uvType,
                       ReadMethod r = ReadMethod::LINE_BY_LINE,
                       GLboolean bFlipNormals = false);

    unsigned int loadTexture(char const* filepath);

    GLuint loadOBJFile(std::string fileName, std::string modelName, bool bNormalFlag, Mesh::UVType uvType);
    std::unordered_map<std::string, Mesh*> scene_mesh_;
    std::unordered_map<std::string, LineMesh*> scene_line_mesh_;

    void setupSphere(const std::string& modelName);
    void setupOrbitLine(const std::string& name, float radius);
    void setupPlane(const std::string& name);

private:

    // Read OBJ file line by line
    int ReadOBJFile_LineByLine(std::string filepath);

    // Read the OBJ file in blocks -- works for files smaller than 1GB
    int ReadOBJFile_BlockIO(std::string filepath);

    // Parse individual OBJ record (one line delimited by '\n')
    void ParseOBJRecord(char* buffer, glm::vec3& min, glm::vec3& max);

    // data members
    Mesh* _currentMesh;
    LineMesh* _currLineMesh;
};
#endif
