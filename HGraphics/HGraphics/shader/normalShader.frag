/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: normalShader.frag
Purpose: This file is fragment shader for rendering normal of vertexs and faces  
Language: glsl
Platform: OpenGL 4.0
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#version 400 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.f);
}
