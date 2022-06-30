/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: deferredShading.frag
Purpose: This file is fragment shader to output g-buffer for deferred shading
Language: glsl
Platform: OpenGL 4.5
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Jan 8, 2022
End Header ---------------------------------------------------------*/
#version 450 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

uniform vec3 diffColor;

uniform sampler2D texture_diff;
uniform sampler2D texture_normal;

uniform int isWithTexture;

float near = 10;
float far = 10000.0;

in vec3 FragPos;
in vec3 Normal;
in vec2 FragUV;
in vec3 Tangents;

void main()
{
    if(isWithTexture == 1.0)
    {
        gAlbedo = vec4(texture(texture_diff, FragUV * 4.0).rgb, 1.0);

        vec3 delta = texture(texture_normal, FragUV * 4.0).xyz;

        delta = delta * 2.0 - vec3(1.0);
        vec3 T = normalize(Tangents);
        vec3 B = normalize(cross(T, Normal));

        vec3 norm = delta.x*T + delta.y*B + delta.z*Normal;

        vec3 finalNorm = norm * 0.5 + 0.5;
        gNormal = vec4(finalNorm, 1.0);
    }
    else
    {
        gAlbedo = vec4(diffColor, 1.0);
        gNormal = vec4(Normal * 0.5 + 0.5, 1.0);
    }

    gPosition = vec4(FragPos, 1.0);
}
