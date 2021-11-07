/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: OBJManager.cpp
Purpose: This file loads and setup models.
Language: c++
Platform: VS2019 / Window
Project:  s.hong_CS300_1
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

OBJManager::OBJManager()
{
    initData();
}

OBJManager::~OBJManager()
{
    initData();
}

void OBJManager::initData()
{
    _currentMesh = nullptr;
    scene_mesh_.clear();
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
        _currentMesh = pMesh;
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
    int size = _currentMesh->getVertexBufferSize();

    glm::vec3 scale = glm::vec3(_currentMesh->getModelScaleRatio());
    glm::vec3 centroid = glm::vec3(0.f) - _currentMesh->getModelCentroid();
    glm::mat4 model = glm::scale(scale) * glm::translate(centroid);

    for (int i = 0; i < size; ++i)
    {
        _currentMesh->vertexBuffer[i] = glm::vec3(model * glm::vec4(_currentMesh->vertexBuffer[i], 1.f));
    }

    // Now calculate vertex normals
    _currentMesh->calcVertexNormals(bFlipNormals);
    _currentMesh->calcUVs(uvType);
    _currentMesh->setupMesh();
    _currentMesh->setupVNormalMesh();
    _currentMesh->setupFNormalMesh();

    return 0;
}

unsigned int OBJManager::loadTexture(char const* filepath)
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
        glTexImage2D(GL_TEXTURE_2D, 0, format, 512, 512, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filepath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

GLuint OBJManager::loadOBJFile(std::string fileName, std::string modelName, bool bNormalFlag, Mesh::UVType uvType)
{
    const GLuint rFlag = static_cast<GLuint>(-1);
    Mesh* mesh = new Mesh();

    if (ReadOBJFile(fileName, mesh, uvType, OBJManager::ReadMethod::LINE_BY_LINE, bNormalFlag) != 1.0)
    {
        scene_mesh_.insert(std::pair<std::string, Mesh*>(modelName, mesh));
    }
    else
        delete mesh;

    return rFlag;
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
            mesh->vertexBuffer.emplace_back(glm::vec3(x, y, z));
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
                mesh->vertexIndices.push_back(f);
                mesh->vertexIndices.push_back(f2);
                mesh->vertexIndices.push_back(f + 1);
            }
            if (i != (horizontalCount - 1))
            {
                mesh->vertexIndices.push_back(f + 1);
                mesh->vertexIndices.push_back(f2);
                mesh->vertexIndices.push_back(f2 + 1);
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
        mesh->vertexBuffer.emplace_back(glm::vec3(x, 0.f, y));
    }
    mesh->setupLineMesh();
    scene_line_mesh_.insert(std::pair<std::string, LineMesh*>(name, mesh));
}

void OBJManager::setupPlane(const std::string& name)
{
    int numSegments = 1;

    Mesh* mesh = new Mesh();

    for (int i = 0; i < numSegments + 1; i++) { // y
        for (int j = 0; j < numSegments + 1; j++) { // x
            float y = 10.f * (static_cast<float>(i) / numSegments - 0.5f);
            float x = 10.f * (static_cast<float>(j) / numSegments - 0.5f);

            mesh->vertexBuffer.emplace_back(glm::vec3(x, y, 0));
        }
    }

    for (int i = 0; i < numSegments; i++) { // y
        int a = (numSegments + 1) * i;

        for (int j = 0; j < numSegments; j++) { // x
            int b = a + j;
            int c = b + numSegments + 1;

            /**idx++ = b;
            *idx++ = b + 1;
            *idx++ = c;*/
            mesh->vertexIndices.push_back(b);
            mesh->vertexIndices.push_back(b + 1);
            mesh->vertexIndices.push_back(c);
            /**idx++ = c;
            *idx++ = b + 1;
            *idx++ = c + 1;*/
            mesh->vertexIndices.push_back(c);
            mesh->vertexIndices.push_back(b + 1);
            mesh->vertexIndices.push_back(c + 1);
        }
    }
    mesh->calcVertexNormals(true);
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

    _currentMesh->boundingBox[0] = min;
    _currentMesh->boundingBox[1] = max;

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

        _currentMesh->boundingBox[0] = min;
        _currentMesh->boundingBox[1] = max;
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

            _currentMesh->vertexBuffer.emplace_back(x, y, z);
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

            _currentMesh->vertexNormals.push_back(glm::normalize(vNormal));
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
        _currentMesh->vertexIndices.push_back(firstIndex);
        _currentMesh->vertexIndices.push_back(secondIndex);
        _currentMesh->vertexIndices.push_back(thirdIndex);

        token = strtok(nullptr, delims);

        while (token != nullptr)
        {
            secondIndex = thirdIndex;
            thirdIndex = static_cast<unsigned int&&>(atoi(token) - 1);

            _currentMesh->vertexIndices.push_back(firstIndex);
            _currentMesh->vertexIndices.push_back(secondIndex);
            _currentMesh->vertexIndices.push_back(thirdIndex);

            token = strtok(nullptr, delims);
        }

        break;

    case '#':
    default:
        break;
    }

    return;
}
