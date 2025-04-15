/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: mesh.h
Purpose: This file is header for mesh.
Language: c++
Platform: VS2019 / Window
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#ifndef MESH_H
#define MESH_H
#include <memory>
#include <string>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>



class Mesh
{
public:
    friend class OBJManager;
    Mesh();
    virtual ~Mesh();

    // Get attribute values buffer
    GLfloat* getVertexBuffer(); // attribute 0
    GLfloat* getVertexNormals(); // attribute 1
    GLfloat* getVertexUVs(); // attribute 2

    GLfloat* getVertexNormalsForDisplay(); // attribute 0

    unsigned int getVertexBufferSize() const;
    unsigned int getVertexCount() const;
    unsigned int getVertexNormalCount() const;

    // Get vertex index buffer
    GLuint* getIndexBuffer();
    unsigned int getIndexBufferSize() const;
    unsigned int getTriangleCount() const;

    glm::vec3 getModelScale() const;
    glm::vec3 getModelCentroid() const;
    glm::vec3 getCentroidVector(glm::vec3 vVertex) const;
    float getModelScaleRatio() const;
    glm::vec3 getMinBound() const;
    glm::vec3 getMaxBound() const;

    virtual void render(int Flag = 0) const;
    void setupMesh();
    void setupVNormalMesh();
    void setupFNormalMesh();

    GLfloat getNormalLength() const;
    void setNormalLength(GLfloat nLength);

    void clearVertexUVs();

    // initialize the data members
    void initData();

    // calculate vertex normals
    int calcVertexNormals(GLboolean bFlipNormals = false);

    // calculate the "display" normals
    void calcVertexNormalsForDisplay();

    // calculate texture coordinates
    enum class UVType
    {
        PLANAR_UV = 0,
        CYLINDRICAL_UV,
        SPHERICAL_UV,
        CUBE_MAPPED_UV
    };

    int calcUVs(UVType uvType = UVType::PLANAR_UV, bool posEntity = true);
    glm::vec2 calcCubeMap(glm::vec3 vEntity);

private:
    GLuint vao_;
    GLuint vnormal_vao_;
    GLuint fnormal_vao_;

    GLuint vertex_count_;
    GLuint face_count_;

    GLuint vbo_pos_;
    GLuint vbo_norm_;
    GLuint vbo_uv_;

    GLuint ebo_;

    std::vector<glm::vec3> face_centroid_;

    std::vector<GLuint> vertex_indices_;
    std::vector<glm::vec3> vertex_buffer_;
    std::vector<glm::vec2> vertex_uv_;
    std::vector<glm::vec3> vertex_normals_, vertex_normal_display_;
    std::vector<glm::vec3> vertex_tangent_, vertex_bitangent;

    glm::vec3 bounding_box_[2];
    GLfloat normal_length_;
};

#endif
