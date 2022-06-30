/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: OBJManager.cpp
Purpose: This file loads and setup models.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include <iostream>
#include <cstring>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec3.hpp>
#include <cfloat>
#include <set>

#include "OBJManager.h"

#include <glm/gtx/transform.hpp>

#include "stb_image.h"

OBJManager* OBJ_MANAGER = nullptr;

OBJManager::OBJManager()
{
    assert(OBJ_MANAGER == nullptr && "You can create obj manager only once!");
    OBJ_MANAGER = this;
    initData();
}

OBJManager::~OBJManager()
{
    initData();
    OBJ_MANAGER = nullptr;
}

void OBJManager::initData()
{
    current_mesh_ = nullptr;
    scene_mesh_.clear();
    scene_line_mesh_.clear();
    loaded_models.clear();
}

Mesh* OBJManager::GetMesh(const std::string& name)
{
    if (scene_mesh_.find(name) == scene_mesh_.end())
        return nullptr;
    return scene_mesh_[name];
}

LineMesh* OBJManager::GetLineMesh(const std::string& name)
{
    if (scene_line_mesh_.find(name) == scene_line_mesh_.end())
        return nullptr;
    return scene_line_mesh_[name];
}

double OBJManager::ReadOBJFile(std::string filepath, Mesh* pMesh, Mesh::UVType uvType,
                               ReadMethod r, GLboolean bFlipNormals)
{
    int rFlag = -1;

    if (pMesh)
        current_mesh_ = pMesh;
    else
        return rFlag;


    switch (r)
    {
    case ReadMethod::LINE_BY_LINE:
        rFlag = ReadOBJFile_LineByLine(filepath);
        break;

    case ReadMethod::BLOCK_IO:
        rFlag = ReadOBJFile_BlockIO(filepath);
        break;

    default:
        std::cout << "Unknown value for OBJReader::ReadMethod in function ReadObjFile." << std::endl;
        std::cout << "Quitting ..." << std::endl;
        rFlag = -1;
        break;
    }
    int size = current_mesh_->getVertexBufferSize();

    glm::vec3 scale = glm::vec3(current_mesh_->getModelScaleRatio());
    glm::vec3 centroid = glm::vec3(0.f) - current_mesh_->getModelCentroid();
    glm::mat4 model = glm::scale(scale) * glm::translate(centroid);

    for (int i = 0; i < size; ++i)
    {
        current_mesh_->vertex_buffer_[i] = glm::vec3(model * glm::vec4(current_mesh_->vertex_buffer_[i], 1.f));
    }

    // Now calculate vertex normals
    current_mesh_->calcVertexNormals(bFlipNormals);
    current_mesh_->calcUVs(uvType);
    current_mesh_->setupMesh();
    current_mesh_->setupVNormalMesh();
    current_mesh_->setupFNormalMesh();

    return 0;
}

void OBJManager::loadTexture(char const* filepath, std::string textureName)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filepath, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        textures.insert(std::pair<std::string, unsigned int>(textureName, textureID));

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filepath << std::endl;
        stbi_image_free(data);
    }
}

unsigned int OBJManager::getTexture(const std::string& name)
{
    if (textures.find(name) == textures.end())
        return -1;
    return textures[name];
}

GLuint OBJManager::loadOBJFile(std::string fileName, std::string modelName, bool bNormalFlag, Mesh::UVType uvType)
{
    const GLuint rFlag = static_cast<GLuint>(-1);
    Mesh* mesh = new Mesh();

    if (ReadOBJFile(fileName, mesh, uvType, OBJManager::ReadMethod::LINE_BY_LINE, bNormalFlag) != 1.0)
    {
        scene_mesh_.insert(std::pair<std::string, Mesh*>(modelName, mesh));
        if(modelName != "quad")
            loaded_models.emplace_back(modelName);
    }
    else
        delete mesh;

    /*if (LoadModel(fileName, mesh) != 1)
    {
        scene_mesh_.insert(std::pair<std::string, Mesh*>(modelName, mesh));
        if (modelName != "quad")
            loaded_models.emplace_back(modelName);
    }
    else
        delete mesh;*/

    return rFlag;
}

int OBJManager::ReadSectionFile(std::string const& filepath)
{
    int rFlag = -1;

    std::ifstream inFile;
    inFile.open(filepath);

    if (inFile.bad() || inFile.eof() || inFile.fail())
        return rFlag;

    std::vector<std::string> modelFilesToRead;
    while (!inFile.eof())
    {
        char buffer[256] = "\0";
        std::string path = "models/Power_Plant_Files/";

        inFile.getline(buffer, 256, '\n');
        modelFilesToRead.emplace_back(path.append(buffer));
    }
    modelFilesToRead.pop_back();

    for (auto i : modelFilesToRead)
    {
        loadOBJFile(i, i, false, Mesh::UVType::CUBE_MAPPED_UV);
    }

    return rFlag;

}

unsigned int OBJManager::loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

unsigned int OBJManager::load_cubemap(const std::string& face)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;

    unsigned char* data = stbi_load(face.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "cubemap texture failed to load" << std::endl;
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return textureID;
}

void OBJManager::setupSphere(const std::string& modelName)
{
    float PI = 3.141596535f;
    Mesh* mesh = new Mesh();

    float x, y, z, xy;
    const int verticalCount = 100;
    const int horizontalCount = 100;
    const float verticalStep = 2.f * PI / static_cast<float>(verticalCount);
    const float horizontalStep = PI / static_cast<float>(horizontalCount);

    for (int i = 0; i <= horizontalCount; ++i)
    {
        const float horizontalAngle = PI / 2.f - static_cast<float>(i) * horizontalStep;
        xy = cosf(horizontalAngle);
        z = sinf(horizontalAngle);

        for (int j = 0; j <= verticalCount; ++j)
        {
            const float verticalAngle = static_cast<float>(j) * verticalStep;

            x = xy * cosf(verticalAngle);
            y = xy * sinf(verticalAngle);
            mesh->vertex_buffer_.emplace_back(glm::vec3(x, y, z));
        }
    }
    int f, f2;
    for (int i = 0; i < horizontalCount; ++i)
    {
        f = i * (verticalCount + 1);
        f2 = f + verticalCount + 1;

        for (int j = 0; j < verticalCount; ++j, ++f, ++f2)
        {
            if (i != 0)
            {
                mesh->vertex_indices_.push_back(f);
                mesh->vertex_indices_.push_back(f2);
                mesh->vertex_indices_.push_back(f + 1);
            }
            if (i != (horizontalCount - 1))
            {
                mesh->vertex_indices_.push_back(f + 1);
                mesh->vertex_indices_.push_back(f2);
                mesh->vertex_indices_.push_back(f2 + 1);
            }
        }
    }
    mesh->calcVertexNormals(false);
    mesh->setupMesh();
    scene_mesh_.insert(std::pair<std::string, Mesh*>(modelName, mesh));
}

void OBJManager::setupOrbitLine(const std::string& name, float radius)
{
    float twicePi = 3.14f * 2.f;
    LineMesh* mesh = new LineMesh();

    float x, y;
    for (int i = 0; i <= 100; ++i)
    {
        x = (radius * cos(i * twicePi / 100));
        y = (radius * sin(i * twicePi / 100));
        mesh->vertex_buffer_.emplace_back(glm::vec3(x, 0.f, y));
    }
    mesh->setupLineMesh();
    scene_line_mesh_.insert(std::pair<std::string, LineMesh*>(name, mesh));
}

void OBJManager::setupPlane(const std::string& name)
{
    int numSegments = 17;

    Mesh* mesh = new Mesh();

    for (int i = 0; i < numSegments + 1; i++) { // y
        for (int j = 0; j < numSegments + 1; j++) { // x
            float y = 2.f * (static_cast<float>(i) / numSegments - 0.5f);
            float x = 2.f * (static_cast<float>(j) / numSegments - 0.5f);

            mesh->vertex_buffer_.emplace_back(glm::vec3(x, y, 0));
        }
    }

    for (int i = 0; i < numSegments; i++) { // y
        int a = (numSegments + 1) * i;

        for (int j = 0; j < numSegments; j++) { // x
            int b = a + j;
            int c = b + numSegments + 1;

            mesh->vertex_indices_.push_back(b);
            mesh->vertex_indices_.push_back(b + 1);
            mesh->vertex_indices_.push_back(c);

            mesh->vertex_indices_.push_back(c);
            mesh->vertex_indices_.push_back(b + 1);
            mesh->vertex_indices_.push_back(c + 1);
        }
    }
    mesh->calcVertexNormals(true);
    mesh->calcUVs(Mesh::UVType::PLANAR_UV, true);
    mesh->setupMesh();
    scene_mesh_.insert(std::pair<std::string, Mesh*>(name, mesh));
}

int OBJManager::ReadOBJFile_LineByLine(std::string filepath)
{
    int rFlag = -1;
    glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    std::ifstream inFile;
    inFile.open(filepath);

    if (inFile.bad() || inFile.eof() || inFile.fail())
        return rFlag;

    while (!inFile.eof())
    {
        char buffer[256] = "\0";
        inFile.getline(buffer, 256, '\n');

        ParseOBJRecord(buffer, min, max);
    }

    current_mesh_->bounding_box_[0] = min;
    current_mesh_->bounding_box_[1] = max;

    return rFlag;
}

int OBJManager::ReadOBJFile_BlockIO(std::string filepath)
{
    int rFlag = -1;
    long int OneGBinBytes = 1024 * 1024 * 1024 * sizeof(char);

    glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Check the file size, if > 1 GB, abort
    std::ifstream inFile(filepath, std::ifstream::in | std::ifstream::binary);

    if (inFile.bad() || inFile.eof())
        return rFlag;

    char* fileContents = NULL;
    long int count = 0;

    // get the file size
    inFile.seekg(0, std::ifstream::end);
    count = (long)inFile.tellg();
    inFile.seekg(0, std::ifstream::beg);

    if (count <= 0 || count >= OneGBinBytes)
    {
        std::cout << " Error reading file " << filepath << std::endl;
        std::cout << "File size reported as : " << count << " bytes." << std::endl;
    }
    else if (count > 0)
    {
        const char* delims = "\n\r";
        fileContents = (char*)malloc((unsigned int)sizeof(char) * (count + 1));
        inFile.read(fileContents, count);
        fileContents[count] = '\0';

        rFlag = 0;

        // Now parse the obj file
        char* currPtr = fileContents;
        char* token = strpbrk(currPtr, delims);

        while (token != nullptr)
        {
            int numChars = *(int*)(token - currPtr);
            char ObjLine[256];
            strncpy(ObjLine, currPtr, numChars);
            ObjLine[numChars] = '\0';

            ParseOBJRecord(ObjLine, min, max);

            currPtr = token + 1;
            token = strpbrk(currPtr, delims);
        }

        free(fileContents);

        current_mesh_->bounding_box_[0] = min;
        current_mesh_->bounding_box_[1] = max;
    }

    return rFlag;
}

void OBJManager::ParseOBJRecord(char* buffer, glm::vec3& min, glm::vec3& max)
{
    const char* delims = " \r\n\t";
    GLfloat x, y, z;

    GLfloat temp;
    GLuint firstIndex, secondIndex, thirdIndex;

    char* token = strtok(buffer, delims);

    // account for empty lines
    if (token == nullptr)
        return;

    switch (token[0])
    {
    case 'v':
        // vertex coordinates
        if (token[1] == '\0')
        {
            token = strtok(nullptr, delims);
            temp = static_cast<GLfloat&&>(atof(token));
            if (min.x > temp)
                min.x = temp;
            if (max.x <= temp)
                max.x = temp;
            x = temp;

            token = strtok(nullptr, delims);
            temp = static_cast<GLfloat&&>(atof(token));
            if (min.y > temp)
                min.y = temp;
            if (max.y <= temp)
                max.y = temp;
            y = temp;

            token = strtok(nullptr, delims);
            temp = static_cast<GLfloat&&>(atof(token));
            if (min.z > temp)
                min.z = temp;
            if (max.z <= temp)
                max.z = temp;
            z = temp;

            current_mesh_->vertex_buffer_.emplace_back(x, y, z);
        }
            // vertex normals
        else if (token[1] == 'n')
        {
            glm::vec3 vNormal;

            token = strtok(nullptr, delims);
            if (token == nullptr)
                break;

            vNormal[0] = static_cast<GLfloat&&>(atof(token));

            token = strtok(nullptr, delims);
            if (token == nullptr)
                break;

            vNormal[1] = static_cast<GLfloat&&>(atof(token));

            token = strtok(nullptr, delims);
            if (token == nullptr)
                break;

            vNormal[2] = static_cast<GLfloat&&>(atof(token));

            current_mesh_->vertex_normals_.push_back(glm::normalize(vNormal));
        }

        break;

    case 'f':
        token = strtok(nullptr, delims);
        if (token == nullptr)
            break;
        firstIndex = static_cast<unsigned int&&>(atoi(token) - 1);

        token = strtok(nullptr, delims);
        if (token == nullptr)
            break;
        secondIndex = static_cast<unsigned int&&>(atoi(token) - 1);

        token = strtok(nullptr, delims);
        if (token == nullptr)
            break;
        thirdIndex = static_cast<unsigned int&&>(atoi(token) - 1);

        // push back first triangle
        current_mesh_->vertex_indices_.push_back(firstIndex);
        current_mesh_->vertex_indices_.push_back(secondIndex);
        current_mesh_->vertex_indices_.push_back(thirdIndex);

        token = strtok(nullptr, delims);

        while (token != nullptr)
        {
            secondIndex = thirdIndex;
            thirdIndex = static_cast<unsigned int&&>(atoi(token) - 1);

            current_mesh_->vertex_indices_.push_back(firstIndex);
            current_mesh_->vertex_indices_.push_back(secondIndex);
            current_mesh_->vertex_indices_.push_back(thirdIndex);

            token = strtok(nullptr, delims);
        }

        break;

    case '#':
    default:
        break;
    }

    return;
}

int OBJManager::LoadModel(std::string const& filepath, Mesh* mesh)
{
    int Flag = -1;

    if (mesh)
        current_mesh_ = mesh;
    else
        return Flag;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return -1;
    }
    //directory = path.substr(0, path.find_last_of('/'));

    ProcessNode(scene->mRootNode, scene);

    return 0;
}

void OBJManager::ProcessNode(aiNode* node, const aiScene* scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
        //meshes.push_back(ProcessMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

void OBJManager::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        current_mesh_->vertex_buffer_.push_back(vector);
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            current_mesh_->vertex_normals_.push_back(vector);
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            current_mesh_->vertex_uv_.push_back(vec);
        }
        else
            current_mesh_->vertex_uv_.emplace_back(0.f, 0.f);

    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            current_mesh_->vertex_indices_.push_back(face.mIndices[j]);
    }

    current_mesh_->setupMesh();
}


