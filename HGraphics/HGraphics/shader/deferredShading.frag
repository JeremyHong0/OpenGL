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
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gUV;
layout (location = 3) out vec3 gDepth;

in vec3 FragPos;
in vec3 Normal;
in vec3 FragUV;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);

    gUV = FragUV;
    float distFromCamera = length(FragPos);
    gDepth = vec3(distFromCamera);
}
