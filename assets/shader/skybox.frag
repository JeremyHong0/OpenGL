/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: skybox.frag
Purpose: This file is fragment shader for rendering skybox
Language: glsl
Platform: OpenGL 4.5
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Sep 29, 2021
End Header ---------------------------------------------------------*/
#version 450 core
out vec4 FragColor;

in vec2 texCoords;
uniform sampler2D skybox;

void main()
{
    FragColor = texture(skybox, texCoords);
}