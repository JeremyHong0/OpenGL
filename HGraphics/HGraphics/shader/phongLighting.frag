/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.frag
Purpose: This file is fragment shader for rendering loaded models
Language: glsl
Platform: OpenGL 4.0
Project:  s.hong_CS300_1
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#version 450 core
out vec4 FragColor;

in VS_OUT {
    vec3 outColor;
} fs_in;
  
void main() {
  FragColor = vec4(fs_in.outColor, 1.0);
}