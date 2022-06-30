/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: phongShading.frag
Purpose: This file is fragment shader for rendering buffers to full screen quad
Language: glsl
Platform: OpenGL 4.5
Project:  HGraphics
Author: Elliott Hong <s.hong@digipen.edu>
Creation date: Jan 8, 2022
End Header ---------------------------------------------------------*/
#version 450 core
const float PI = 3.1415926;

out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 viewPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gDepth;

uniform vec3 Emissive;
uniform vec3 worldPos;
uniform mat4 lightSpaceMatrix;
uniform int lightType;
uniform int lightCount;
uniform int drawBuffer;

float depthFromEye;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(gDepth, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    float bias = 0.03;
    //if (PCF == 1.0)
    {
        vec2 texelSize = 1.0 / textureSize(gDepth, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(gDepth, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;

        return shadow;
    }
    float closetDepth = texture(gDepth, projCoords.xy).r;
    shadow = currentDepth - bias > closetDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{    
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 albedoMap = texture(gAlbedo, TexCoords).rgb;
    depthFromEye = texture(gDepth, TexCoords).r;

    vec3 color = texture(gAlbedo, TexCoords).rgb;
    vec3 normal = normalize(Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(worldPos - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = ShadowCalculation(FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}