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
    vao_ = 0;
    vnormal_vao_ = 0;
    fnormal_vao_ = 0;
    vertex_count_ = 0;
    vbo_pos_ = 0;
    vbo_norm_ = 0;
    ebo_ = 0;
    face_count_ = 0;
    normal_length_ = 1.00f;
    bounding_box_[0] = glm::vec3(0.f);
    initData();
}

Mesh::~Mesh()
{
    initData();
    glDeleteBuffers(1, vertex_indices_.data());
    glDeleteBuffers(1, &vbo_pos_);
    glDeleteBuffers(1, &vbo_norm_);
    glDeleteBuffers(1, &ebo_);
}

void Mesh::initData()
{
    vertex_buffer_.clear();
    vertex_indices_.clear();
    vertex_uv_.clear();
    vertex_normals_.clear();
    vertex_normal_display_.clear();
    face_centroid_.clear();
}

void Mesh::render(int Flag) const
{
    if (vao_ == 0) return;

    if (Flag == 0)
    {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, vertex_count_, GL_UNSIGNED_INT, 0);
    }
    else if (Flag == 1)
    {
        glBindVertexArray(vnormal_vao_);
        glDrawArrays(GL_LINES, 0, face_count_);
    }
    else if (Flag == 2)
    {
        glBindVertexArray(fnormal_vao_);
        glDrawArrays(GL_LINES, 0, face_count_);
    }
    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
    vertex_count_ = (GLuint)vertex_indices_.size();
    face_count_ = getTriangleCount() * 2;

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_pos_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_.size() * sizeof(GLfloat) * 3, vertex_buffer_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_indices_.size() * sizeof(GLuint), vertex_indices_.data(), GL_STATIC_DRAW);

    if (!vertex_normals_.empty())
    {
        glGenBuffers(1, &vbo_norm_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_norm_);
        glBufferData(GL_ARRAY_BUFFER, vertex_normals_.size() * sizeof(GLfloat) * 3, vertex_normals_.data(), GL_STATIC_DRAW);
    }

    if (!vertex_uv_.empty())
    {
        glGenBuffers(1, &vbo_uv_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uv_);
        glBufferData(GL_ARRAY_BUFFER, vertex_uv_.size() * sizeof(GLfloat) * 2, vertex_uv_.data(), GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, static_cast<void*>(0));

    if (!vertex_normals_.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_norm_);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, static_cast<void*>(0));
    }

    if (!vertex_uv_.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uv_);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, static_cast<void*>(0));
    }

    glBindVertexArray(0);
}

void Mesh::setupVNormalMesh()
{
    glGenVertexArrays(1, &vnormal_vao_);
    glGenBuffers(1, &vbo_pos_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vnormal_vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glBufferData(GL_ARRAY_BUFFER, vertex_normal_display_.size() * sizeof(GLfloat) * 3, vertex_normal_display_.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_indices_.size() * sizeof(GLuint), vertex_indices_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, static_cast<void*>(0));

    glBindVertexArray(0);
}

void Mesh::setupFNormalMesh()
{
    glGenVertexArrays(1, &fnormal_vao_);
    glGenBuffers(1, &vbo_pos_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(fnormal_vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glBufferData(GL_ARRAY_BUFFER, face_centroid_.size() * sizeof(GLfloat) * 3, face_centroid_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_indices_.size() * sizeof(GLuint), vertex_indices_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, static_cast<void*>(0));

    glBindVertexArray(0);
}


GLfloat* Mesh::getVertexBuffer()
{
    return reinterpret_cast<GLfloat*>(vertex_buffer_.data());
}

GLfloat* Mesh::getVertexNormals()
{
    return reinterpret_cast<GLfloat*>(vertex_normals_.data());
}

GLfloat* Mesh::getVertexUVs()
{
    return reinterpret_cast<GLfloat*>(vertex_uv_.data());
}

GLfloat* Mesh::getVertexNormalsForDisplay()
{
    return reinterpret_cast<GLfloat*>(vertex_normal_display_.data());
}

GLuint* Mesh::getIndexBuffer()
{
    return vertex_indices_.data();
}

unsigned int Mesh::getVertexBufferSize()
{
    return (unsigned int)vertex_buffer_.size();
}

unsigned int Mesh::getIndexBufferSize()
{
    return (unsigned int)vertex_indices_.size();
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
    return (unsigned int)vertex_normal_display_.size();
}

glm::vec3 Mesh::getModelScale()
{
    glm::vec3 scale = bounding_box_[1] - bounding_box_[0];

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
    return glm::vec3(bounding_box_[0] + bounding_box_[1]) * 0.5f;
}

glm::vec3 Mesh::getCentroidVector(glm::vec3 vVertex)
{
    return glm::normalize(vVertex - getModelCentroid());
}

float Mesh::getModelScaleRatio()
{
    glm::vec3 scale = bounding_box_[1] - bounding_box_[0];
    float result = 0.f;
    result = glm::max(scale.x, scale.y);
    result = glm::max(result, scale.z);
    return 1.f / (result);
}

glm::vec3 Mesh::getMinBound()
{
    return bounding_box_[0];
}

glm::vec3 Mesh::getMaxBound()
{
    return bounding_box_[1];
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
    if (vertex_buffer_.empty() || vertex_indices_.empty())
    {
        std::cout << "Cannot calculate vertex normals for empty mesh." << std::endl;
        return rFlag;
    }

    // Initialize vertex normals
    GLuint numVertices = getVertexCount();
    vertex_normals_.resize(numVertices, glm::vec3(0.0f));
    vertex_normal_display_.resize(static_cast<__int64>(numVertices) * 2, glm::vec3(0.0f));
    face_centroid_.resize(static_cast<__int64>(getTriangleCount())* 2, glm::vec3(0.f));

    std::vector<std::set<glm::vec3, compareVec>> vNormalSet;
    vNormalSet.resize(numVertices);
    setNormalLength(0.1f);

    // For every face
    int index = 0;
    for (; index < vertex_indices_.size();)
    {
        GLuint a = vertex_indices_.at(index++);
        GLuint b = vertex_indices_.at(index++);
        GLuint c = vertex_indices_.at(index++);

        glm::vec3 vA = vertex_buffer_[a];
        glm::vec3 vB = vertex_buffer_[b];
        glm::vec3 vC = vertex_buffer_[c];

        // Edge vectors
        glm::vec3 E1 = vB - vA;
        glm::vec3 E2 = vC - vA;

        glm::vec3 N = glm::normalize(glm::cross(E1, E2));

        glm::vec3 faceCenter = (vA + vB + vC) / 3.f;
        face_centroid_[static_cast<__int64>(static_cast<__int64>(index) / 3 - 1) * 2] = (faceCenter);

        glm::vec3 F1 = vA - faceCenter;
        glm::vec3 F2 = vB - faceCenter;
        glm::vec3 fN = glm::normalize(glm::cross(F1, F2));

        if (bFlipNormals)
            fN = fN * -1.0f;

        face_centroid_[static_cast<__int64>(static_cast<__int64>(index) / 3 - 1) * 2 + 1] = (faceCenter) + normal_length_ * fN;

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
        vertex_normals_[i] = glm::normalize(vNormal);

        // save normal to display
        glm::vec3 vA = vertex_buffer_[i];

        vertex_normal_display_[2 * static_cast<__int64>(i)] = vA;
        vertex_normal_display_[(2 * static_cast<__int64>(i)) + 1] = vA + (normal_length_ * vertex_normals_[i]);
    }

    // success
    rFlag = 0;

    return rFlag;
}

void Mesh::calcVertexNormalsForDisplay()
{
    GLuint numVertices = getVertexCount();
    vertex_normal_display_.resize(static_cast<__int64>(numVertices) * 2, glm::vec3(0.0f));

    for (int iNormal = 0; iNormal < vertex_normals_.size(); ++iNormal)
    {
        glm::vec3 normal = vertex_normals_[iNormal] * normal_length_;

        vertex_normal_display_[2 * static_cast<__int64>(iNormal)] = vertex_buffer_[iNormal];
        vertex_normal_display_[(2 * static_cast<__int64>(iNormal)) + 1] = vertex_buffer_[iNormal] + normal;
    }
}

GLfloat& Mesh::getNormalLength()
{
    return normal_length_;
}

void Mesh::setNormalLength(GLfloat nLength)
{
    normal_length_ = nLength;
}

void Mesh::clearVertexUVs()
{
    vertex_uv_.clear();
}

int Mesh::calcUVs(UVType uvType, bool posEntity)
{
    int rFlag = -1;

    // clear any existing UV
    vertex_uv_.clear();


    glm::vec3 delta = getModelScale();
    glm::vec3 centroidVec = glm::vec3(0.f);

    for (int nVertex = 0; nVertex < vertex_buffer_.size(); ++nVertex)
    {
        glm::vec3 V = vertex_buffer_[nVertex];
        glm::vec2 uv(0.0f);

        glm::vec3 normVertex = glm::vec3((V.x - bounding_box_[0].x) / delta.x,
                                         (V.y - bounding_box_[0].y) / delta.y,
                                         (V.z - bounding_box_[0].z) / delta.z);
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
            uv.y = (z - bounding_box_[0].y) / (bounding_box_[1].y - bounding_box_[0].y);
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

        vertex_uv_.push_back(uv);
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
