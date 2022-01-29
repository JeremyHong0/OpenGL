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
#define NR_POINT_LIGHTS 16

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 direction;
    float inner_angle;
    float outer_angle;
    float falloff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;   

    int lightType;    
};

in vec2 TexCoords;

uniform vec3 viewPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gUV;
uniform sampler2D gDepth;

uniform vec3 Emissive;
uniform Light Lights[NR_POINT_LIGHTS];
uniform int lightType;
uniform int lightCount;
uniform int drawBuffer;

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, vec3 FragPos);
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcFog(vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 uvs;
float depthFromEye;
void main()
{    
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    uvs = texture(gUV, TexCoords).rgb;
    depthFromEye = texture(gDepth, TexCoords).r;

    vec3 lighting;// = uvs * 0.01;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    if(drawBuffer == 0)
    {
        for(int i = 0; i < lightCount; ++i)
        {
            if(Lights[i].lightType == 0)
                lighting += CalcPointLight(Lights[i], Normal, FragPos, viewDir);
            else if(Lights[i].lightType == 1)
                lighting += CalcDirLight(Lights[i], Normal, viewDir, FragPos);
            else if(Lights[i].lightType == 2)
                lighting += CalcSpotLight(Lights[i], Normal, FragPos, viewDir);
        }
    }
    FragColor = vec4(lighting, 1.0);
    if(drawBuffer == 1)
        FragColor = vec4(FragPos, 1.0);
    else if(drawBuffer == 2)
        FragColor = vec4(Normal, 1.0);
    else if(drawBuffer == 3)
        FragColor = texture(gUV, TexCoords);
    else if(drawBuffer == 4)
        FragColor = texture(gDepth, TexCoords);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = 2 * dot(lightDir, normal)*normal - lightDir;
    float spec = 0.f;
    if(dot(normal, lightDir) > 0.0)
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    }
    else
        spec = 0.f;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    ambient = vec3(0.3 * uvs * depthFromEye);//light.ambient / 32;
    diffuse = light.diffuse * diff * uvs;
    specular = light.specular * spec;
    // combine results
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = 2 * dot(lightDir, normal)*normal - lightDir;
    float spec = 0.f;
    if(dot(normal, lightDir) > 0.0)
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    }
    else
        spec = 0.f;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    ambient = vec3(0.3 * uvs * depthFromEye);//light.ambient / 32;
    diffuse = light.diffuse * diff * uvs;
    specular = light.specular * spec;
   
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // combine results
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = 2 * dot(lightDir, normal)*normal - lightDir;
    float spec = 0.f;
    if(dot(normal, lightDir) > 0.0)
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    }
    else
        spec = 0.f;
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation= min(1.0 / (light.falloff + light.linear * distance + light.quadratic * (distance * distance)), 1.0);
    // spotlight intensity
    float theta = dot(-lightDir, normalize(light.direction)); 
    float epsilon = light.inner_angle - light.outer_angle;
    float intensity = clamp((theta - light.outer_angle) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    ambient = vec3(0.3 * uvs * depthFromEye);//light.ambient / 32;
    diffuse = light.diffuse * diff * uvs;
    specular = light.specular * spec;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
