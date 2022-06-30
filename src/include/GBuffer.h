#pragma once

#ifndef GBUFFFER_H
#define GBUFFER_H
#include <glad/glad.h>

class GBuffer
{
public:
    GLuint position, normal, color, depth, light, effect1, effect2;

    GBuffer(int widthIn, int heightIn);
    ~GBuffer();

    GLuint getFBO() const;
    int getWidth() const;
    int getHeight() const;
    void setDrawBuffers();
    void setDrawLight();
    void setDrawEffect();
    void setDrawNone();
    void setReadEffect();
    void bind();
    void bindDraw();
    void bindRead();
    void unbind();
    void unbindDraw();
    void unbindRead();
    void setGeomTextures();
    void setReadBuffers();

private:
    GLenum drawBuffers[3];

    GLuint fbo;

    int width, height;
};

#endif
