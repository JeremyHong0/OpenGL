/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: mesh.cpp
Purpose: This file setup buffers, calculate vertex normals/UVs, and render models
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#include "mesh.h"

#include <iostream>
#include <set>
#include <glm/gtc/epsilon.hpp>


Mesh::Mesh()
{
    VAO = 0;
    vnormalVAO = 0;
    fnormalVAO = 0;
    vertexCount = 0;
    VBO_pos = 0;
    VBO_norm = 0;
    EBO = 0;
    faceCount = 0;
    normalLength = 1.00f;
    boundingBox[0] = glm::vec3(0.f);
    initData();
}

Mesh::~Mesh()
{
    initData();
    glDeleteBuffers(1, vertexIndices.data());
    glDeleteBuffers(1, &VBO_pos);
    glDeleteBuffers(1, &VBO_norm);
    glDeleteBuffers(1, &EBO);
}

void Mesh::initData()
{
    vertexBuffer.clear();
    vertexIndices.clear();
    vertexUVs.clear();
    vertexNormals.clear();
    vertexNormalDisplay.clear();
    faceCentroid.clear();
}

void Mesh::render(int Flag) const
{
    if (VAO == 0) return;

    if (Flag == 0)
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    }
    else if (Flag == 1)
    {
        glBindVertexArray(vnormalVAO);
        glDrawArrays(GL_LINES, 0, faceCount);
    }
    else if (Flag == 2)
    {
        glBindVertexArray(fnormalVAO);
        glDrawArrays(GL_LINES, 0, faceCount);
    }
    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
    vertexCount = (GLuint)vertexIndices.size();
    faceCount = getTriangleCount() * 2;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO_pos);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(GLfloat) * 3, vertexBuffer.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLuint), vertexIndices.data(), GL_STATIC_DRAW);

    if (!vertexNormals.empty())
    {
        glGenBuffers(1, &VBO_norm);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
        glBufferData(GL_ARRAY_BUFFER, vertexNormals.size() * sizeof(GLfloat) * 3, vertexNormals.data(), GL_STATIC_DRAW);
    }

    if (!vertexUVs.empty())
    {
        glGenBuffers(1, &VBO_uv);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_uv);
        glBufferData(GL_ARRAY_BUFFER, vertexUVs.size() * sizeof(GLfloat) * 2, vertexUVs.data(), GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);

    if (!vertexNormals.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
    }

    if (!vertexUVs.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_uv);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
    }

    glBindVertexArray(0);
}

void Mesh::setupVNormalMesh()
{
    glGenVertexArrays(1, &vnormalVAO);
    glGenBuffers(1, &VBO_pos);
    glGenBuffers(1, &EBO);

    glBindVertexArray(vnormalVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, vertexNormalDisplay.size() * sizeof(GLfloat) * 3, vertexNormalDisplay.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLuint), vertexIndices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);

    glBindVertexArray(0);
}

void Mesh::setupFNormalMesh()
{
    glGenVertexArrays(1, &fnormalVAO);
    glGenBuffers(1, &VBO_pos);
    glGenBuffers(1, &EBO);

    glBindVertexArray(fnormalVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, faceCentroid.size() * sizeof(GLfloat) * 3, faceCentroid.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLuint), vertexIndices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);

    glBindVertexArray(0);
}


GLfloat* Mesh::getVertexBuffer()
{
    return reinterpret_cast<GLfloat*>(vertexBuffer.data());
}

GLfloat* Mesh::getVertexNormals()
{
    return reinterpret_cast<GLfloat*>(vertexNormals.data());
}

GLfloat* Mesh::getVertexUVs()
{
    return reinterpret_cast<GLfloat*>(vertexUVs.data());
}

GLfloat* Mesh::getVertexNormalsForDisplay()
{
    return reinterpret_cast<GLfloat*>(vertexNormalDisplay.data());
}

GLuint* Mesh::getIndexBuffer()
{
    return vertexIndices.data();
}

unsigned int Mesh::getVertexBufferSize()
{
    return (unsigned int)vertexBuffer.size();
}

unsigned int Mesh::getIndexBufferSize()
{
    return (unsigned int)vertexIndices.size();
}

unsigned int Mesh::getTriangleCount()
{
    return getIndexBufferSize() / 3;
}

unsigned int Mesh::getVertexCount()
{
    return getVertexBufferSize();
}

unsigned int Mesh::getVertexNormalCount()
{
    return (unsigned int)vertexNormalDisplay.size();
}

glm::vec3 Mesh::getModelScale()
{
    glm::vec3 scale = boundingBox[1] - boundingBox[0];

    if (scale.x == 0.0)
        scale.x = 1.0;

    if (scale.y == 0.0)
        scale.y = 1.0;

    if (scale.z == 0.0)
        scale.z = 1.0;

    return scale;
}

glm::vec3 Mesh::getModelCentroid()
{
    return glm::vec3(boundingBox[0] + boundingBox[1]) * 0.5f;
}

glm::vec3 Mesh::getCentroidVector(glm::vec3 vVertex)
{
    return glm::normalize(vVertex - getModelCentroid());
}

float Mesh::getModelScaleRatio()
{
    glm::vec3 scale = boundingBox[1] - boundingBox[0];
    float result = 0.f;
    result = glm::max(scale.x, scale.y);
    result = glm::max(result, scale.z);
    return 1.f / (result);
}

glm::vec3 Mesh::getMinBound()
{
    return boundingBox[0];
}

glm::vec3 Mesh::getMaxBound()
{
    return boundingBox[1];
}

struct compareVec
{
    bool operator()(const glm::vec3& lhs, const glm::vec3& rhs) const
    {
        glm::highp_bvec3 result = glm::lessThan(lhs, rhs);

        return glm::epsilonEqual(lhs.x, rhs.x, FLT_EPSILON) ?
            (glm::epsilonEqual(lhs.y, rhs.y, FLT_EPSILON) ? (glm::epsilonEqual(lhs.z, rhs.z, FLT_EPSILON) ? false : result.z) : result.y) : result.x;
    }
};

int Mesh::calcVertexNormals(GLboolean bFlipNormals)
{
    int rFlag = -1;

    // vertices and indices must be populated
    if (vertexBuffer.empty() || vertexIndices.empty())
    {
        std::cout << "Cannot calculate vertex normals for empty mesh." << std::endl;
        return rFlag;
    }

    // Initialize vertex normals
    GLuint numVertices = getVertexCount();
    vertexNormals.resize(numVertices, glm::vec3(0.0f));
    vertexNormalDisplay.resize(static_cast<__int64>(numVertices) * 2, glm::vec3(0.0f));
    faceCentroid.resize(static_cast<__int64>(getTriangleCount())* 2, glm::vec3(0.f));

    std::vector<std::set<glm::vec3, compareVec>> vNormalSet;
    vNormalSet.resize(numVertices);
    setNormalLength(0.1f);

    // For every face
    int index = 0;
    for (; index < vertexIndices.size();)
    {
        GLuint a = vertexIndices.at(index++);
        GLuint b = vertexIndices.at(index++);
        GLuint c = vertexIndices.at(index++);

        glm::vec3 vA = vertexBuffer[a];
        glm::vec3 vB = vertexBuffer[b];
        glm::vec3 vC = vertexBuffer[c];

        // Edge vectors
        glm::vec3 E1 = vB - vA;
        glm::vec3 E2 = vC - vA;

        glm::vec3 N = glm::normalize(glm::cross(E1, E2));

        glm::vec3 faceCenter = (vA + vB + vC) / 3.f;
        faceCentroid[static_cast<__int64>(static_cast<__int64>(index) / 3 - 1) * 2] = (faceCenter);

        glm::vec3 F1 = vA - faceCenter;
        glm::vec3 F2 = vB - faceCenter;
        glm::vec3 fN = glm::normalize(glm::cross(F1, F2));

        if (bFlipNormals)
            fN = fN * -1.0f;

        faceCentroid[static_cast<__int64>(static_cast<__int64>(index) / 3 - 1) * 2 + 1] = (faceCenter) + normalLength * fN;

        if (bFlipNormals)
            N = N * -1.0f;

        // For vertex a
        vNormalSet.at(a).insert(N);
        vNormalSet.at(b).insert(N);
        vNormalSet.at(c).insert(N);
    }

    // Now sum up the values per vertex
    for (int i = 0; i < vNormalSet.size(); ++i)
    {
        glm::vec3 vNormal(0.0f);

        auto nIt = vNormalSet[i].begin();
        while (nIt != vNormalSet[i].end())
        {
            vNormal += (*nIt);
            ++nIt;
        }

        // save vertex normal
        vertexNormals[i] = glm::normalize(vNormal);

        // save normal to display
        glm::vec3 vA = vertexBuffer[i];

        vertexNormalDisplay[2 * static_cast<__int64>(i)] = vA;
        vertexNormalDisplay[(2 * static_cast<__int64>(i)) + 1] = vA + (normalLength * vertexNormals[i]);
    }

    // success
    rFlag = 0;

    return rFlag;
}

void Mesh::calcVertexNormalsForDisplay()
{
    GLuint numVertices = getVertexCount();
    vertexNormalDisplay.resize(static_cast<__int64>(numVertices) * 2, glm::vec3(0.0f));

    for (int iNormal = 0; iNormal < vertexNormals.size(); ++iNormal)
    {
        glm::vec3 normal = vertexNormals[iNormal] * normalLength;

        vertexNormalDisplay[2 * static_cast<__int64>(iNormal)] = vertexBuffer[iNormal];
        vertexNormalDisplay[(2 * static_cast<__int64>(iNormal)) + 1] = vertexBuffer[iNormal] + normal;
    }
}

GLfloat& Mesh::getNormalLength()
{
    return normalLength;
}

void Mesh::setNormalLength(GLfloat nLength)
{
    normalLength = nLength;
}

void Mesh::clearVertexUVs()
{
    vertexUVs.clear();
}

int Mesh::calcUVs(UVType uvType, bool posEntity)
{
    int rFlag = -1;

    // clear any existing UV
    vertexUVs.clear();


    glm::vec3 delta = getModelScale();
    glm::vec3 centroidVec = glm::vec3(0.f);

    for (int nVertex = 0; nVertex < vertexBuffer.size(); ++nVertex)
    {
        glm::vec3 V = vertexBuffer[nVertex];
        glm::vec2 uv(0.0f);

        glm::vec3 normVertex = glm::vec3((V.x - boundingBox[0].x) / delta.x,
                                         (V.y - boundingBox[0].y) / delta.y,
                                         (V.z - boundingBox[0].z) / delta.z);
        if (posEntity)
            centroidVec = V;//getCentroidVector(V);
        else
            centroidVec = glm::normalize(normVertex);


        float theta(0.0f);
        float z(0.0f);
        float phi(0.0f);

        switch (uvType)
        {
        case UVType::PLANAR_UV:
            uv.x = (centroidVec.x - (-1.0f)) / (2.0f);
            uv.y = (centroidVec.y - (-1.0f)) / (2.0f);
            break;

        case UVType::CYLINDRICAL_UV:
            theta = glm::degrees(static_cast<float>(glm::atan(centroidVec.z, centroidVec.x)));
            theta += 180.0f;

            //z = (centroidVec.z + 1.0f) * 0.5f;
            z = centroidVec.y;

            uv.x = 1 - theta / 360.f;
            uv.y = (z - boundingBox[0].y) / (boundingBox[1].y - boundingBox[0].y);
            break;

        case UVType::SPHERICAL_UV:
            theta = glm::degrees(static_cast<float>(glm::atan(centroidVec.z, centroidVec.x)));
            theta += 180.0f;

            phi = 180.f - acosf(centroidVec.y / centroidVec.length()) * 180.f / acosf(-1);

            uv.x =  1 - theta / 360.0f;
            uv.y = (phi / 180.0f);
            break;

        case UVType::CUBE_MAPPED_UV:
            uv = calcCubeMap(centroidVec);
            break;
        }

        vertexUVs.push_back(uv);
    }

    return rFlag;
}

glm::vec2 Mesh::calcCubeMap(glm::vec3 vEntity)
{
    float x = vEntity.x;
    float y = vEntity.y;
    float z = vEntity.z;

    float absX = abs(x);
    float absY = abs(y);
    float absZ = abs(z);

    int isXPositive = x > 0 ? 1 : 0;
    int isYPositive = y > 0 ? 1 : 0;
    int isZPositive = z > 0 ? 1 : 0;

    float maxAxis, uc, vc;
    glm::vec2 uv = glm::vec2(0.0);

    // POSITIVE X
    if (bool(isXPositive) && (absX >= absY) && (absX >= absZ))
    {
        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = -z / absX;
        vc = y / absX;
    }

    // NEGATIVE X
    else if (!bool(isXPositive) && absX >= absY && absX >= absZ)
    {
        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = z / absX;
        vc = y / absX;
    }

    // POSITIVE Y
    else if (bool(isYPositive) && absY >= absX && absY >= absZ)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        maxAxis = absY;
        uc = x / absY;
        vc = -z / absY;
    }

    // NEGATIVE Y
    else if (!bool(isYPositive) && absY >= absX && absY >= absZ)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        maxAxis = absY;
        uc = x / absY;
        vc = z / absY;
    }

    // POSITIVE Z
    else if (bool(isZPositive) && absZ >= absX && absZ >= absY)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = x / absZ;
        vc = y / absZ;
    }

    // NEGATIVE Z
    else if (!bool(isZPositive) && absZ >= absX && absZ >= absY)
    {
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = -x / absZ;
        vc = y / absZ;
    }

    // Convert range from -1 to 1 to 0 to 1
    uv.s = 0.5f * (uc + 1.0f);
    uv.t = 0.5f * (vc + 1.0f);

    return uv;
}
