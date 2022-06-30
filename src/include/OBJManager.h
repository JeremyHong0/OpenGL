/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: OBJManager.h
Purpose: This file is header for object manager.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef OBJ_MANAGER_H
#define OBJ_MANAGER_H

#include <string>
#include <fstream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

    void loadTexture(char const* filepath, std::string textureName);
    unsigned int getTexture(const std::string& name);
    int ReadSectionFile(std::string const& filepath);

    GLuint loadOBJFile(std::string fileName, std::string modelName, bool bNormalFlag, Mesh::UVType uvType);
    unsigned int load_cubemap(const std::string& face);
    std::unordered_map<std::string, Mesh*> scene_mesh_;
    std::unordered_map<std::string, LineMesh*> scene_line_mesh_;
    std::unordered_map<std::string, unsigned int> textures;
    std::vector<std::string> loaded_models;
    std::vector<Mesh> meshes;
    
    unsigned int loadCubemap(std::vector<std::string> faces);
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

    int LoadModel(std::string const& filepath, Mesh* mesh);

    void ProcessNode(aiNode* node, const aiScene* scene);

    void ProcessMesh(aiMesh* mesh, const aiScene* scene);


    // data members
    Mesh* current_mesh_;
    LineMesh* cuurent_line_mesh_;
};

extern OBJManager* OBJ_MANAGER;

#endif
