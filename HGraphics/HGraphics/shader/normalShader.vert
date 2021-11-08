/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: normalShader.vert
Purpose: This file is vertex shader for rendering normal of vertexs and faces  
Language: glsl
Platform: OpenGL 4.5
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#version 400 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}