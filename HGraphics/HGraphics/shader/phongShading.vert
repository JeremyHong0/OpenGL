/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.vert
Purpose: This file is vertex shader for rendering loaded models
Language: glsl
Platform: OpenGL 4.0
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 EPos;
out vec3 Enorm;

// layout(std140, binding = 0) uniform pvMatirix
// {
// 	mat4 projection;
// 	mat4 view;
// };

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;


void main()
{
    vec3 centroidVec = normalize(aPos);

    EPos = aPos;
    Enorm = aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalMatrix * aNormal;

    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

