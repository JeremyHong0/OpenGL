/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: skybox.vert
Purpose: This file is vertex shader for rendering skybox
Language: glsl
Platform: OpenGL 4.5
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;

void main()
{
    texCoords = aTexCoords;
	mat4 view_xy = view;
	for(int i = 0; i < 3; ++i)
		view_xy[3][i] = 0.0;

	vec4 pos = projection * view_xy * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}