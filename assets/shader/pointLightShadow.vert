/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: deferredShading.vert
Purpose: This file is vertex shader to output g-buffer for deferred shading
Language: glsl
Platform: OpenGL 4.5
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Jan 8, 2022
End Header ---------------------------------------------------------*/
#version 450 core
layout(location = 0) in vec3 position;

uniform mat4 mvp;

out vec3 fragPos;

void main() {
	gl_Position = mvp * vec4(position, 1.0);
	fragPos = position;
}