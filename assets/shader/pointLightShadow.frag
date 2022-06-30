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

in vec3 fragPos;

uniform mat4 inverseMView;
uniform vec3 worldPos;

out float squaredDist;

void main(void) {
	vec3 dist = worldPos - fragPos;
	squaredDist = dot(dist, dist);
} 
