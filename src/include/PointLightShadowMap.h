#pragma once
#ifndef POINT_LIGHT_SHADOW_MAP_H
#define POINT_LIGHT_SHADOW_MAP_H

#include "ShadowMap.h"

class PointLightShadowMap : public ShadowMap {
public:
    GLuint cubeMap;

    PointLightShadowMap(int width, int height);
    ~PointLightShadowMap();
};

#endif