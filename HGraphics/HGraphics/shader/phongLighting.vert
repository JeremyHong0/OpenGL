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

out VS_OUT {
    vec3 outColor;
} vs_out;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D grid;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
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
};

uniform struct FogInfo {
  float MaxDist;
  float MinDist;
  vec3 Color;
} Fog;

#define NR_POINT_LIGHTS 16

uniform vec3 viewPos;
uniform vec3 globalAmbient;
uniform vec3 Emissive;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform DirLight dirLights[NR_POINT_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_POINT_LIGHTS];
uniform Material material;
uniform int lightNum;
uniform int lightType;
uniform bool bCalcUV;
uniform bool bCalcPos;
uniform bool bShowUV;
uniform bool bModel;
uniform int mappingMode;
uniform vec3 min_;
uniform vec3 max_;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 FragPos);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcFog(vec3 normal, vec3 vertPos, vec3 viewDir);

vec2 calcCubeMap(vec3 vEntity);
vec2 calcCylindricalUV(vec3 centVec);
vec2 calcSphericalUV(vec3 centVec);

uniform mat3 normalMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec2 FragTexCoord;

void main(){
    
    vec4 vertPos = model * vec4(aPos, 1.0);
    vec3 norm = normalMatrix * aNormal;
    vec3 viewDir = normalize(viewPos - vertPos.xyz);

    FragTexCoord = aTexCoords;
    vec3 EntityPos;
    if(bCalcPos)
        EntityPos = aPos;
    else
        EntityPos = aNormal;

    if(bCalcUV)
    {
        switch(mappingMode)
        {
            case 0:
                FragTexCoord = calcCylindricalUV(EntityPos);
                break;
            case 1:
                FragTexCoord = calcSphericalUV(EntityPos);
                break;
            case 2:
                FragTexCoord = calcCubeMap(EntityPos);
                break;
        }
    }
    else
        FragTexCoord = aTexCoords;
    vec3 result = vec3(0.f);
    vec3 fogResult = vec3(0.f);
    fogResult += CalcFog(norm, vertPos.xyz, viewDir);
    if(bModel)
        vs_out.outColor = globalAmbient * Ka + Emissive + fogResult;
    else
        vs_out.outColor = globalAmbient + fogResult;

    gl_Position = projection * view * vertPos;
}

vec3 CalcFog(vec3 normal, vec3 vertPos, vec3 viewDir)
{
    float dist = length( viewPos );
    float fogFactor = (Fog.MaxDist - dist) /
                      (Fog.MaxDist - Fog.MinDist);
    vec3 result = vec3(0.f);
    for(int i = 0; i < lightNum; i++)
    {
        if(lightType == 0)
            result += CalcPointLight(pointLights[i], normal, vertPos, viewDir);
        if(lightType == 1)
            result += CalcDirLight(dirLights[i], normal, viewDir, vertPos);
        if(lightType == 2) 
            result += CalcSpotLight(spotLights[i], normal, vertPos, viewDir);
    }
    vec3 color = fogFactor * result + (1 - fogFactor) * Fog.Color;
    return color;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;
    float spec = 0.f;
    float Ks_r = 0.f;
    if(dot(normal, lightDir) > 0.0)
    {
        Ks_r = texture(material.specular, FragTexCoord).r;
        spec = pow(max(dot(viewDir, reflectDir), 0.0), Ks_r*Ks_r*32+0.00001);
    }
    else
        spec = 0.f;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    if(bModel)
    {
        if(bShowUV)
        {
            ambient = light.ambient * vec3(texture(material.grid, FragTexCoord))/16;
            diffuse = Kd * light.diffuse * diff * vec3(texture(material.grid, FragTexCoord));
            specular = Ks * light.specular * pow(max(dot(viewDir, reflectDir),0.0),32) * vec3(texture(material.grid, FragTexCoord));
        }
        else
        { 
            ambient = light.ambient * vec3(texture(material.diffuse, FragTexCoord))/16;
            diffuse = Kd * light.diffuse * diff * vec3(texture(material.diffuse, FragTexCoord));
            specular = Ks * light.specular * spec * vec3(texture(material.specular, FragTexCoord));
        }
    }
    else
    {
        ambient = light.ambient / 16;
        diffuse = light.diffuse * diff;
        specular = light.specular * pow(max(dot(viewDir, reflectDir),0.0),32);
    }
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 opLightDir = -normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;
    float spec = 0.f;
    float Ks_r = 0.f;
    if(dot(normal, lightDir) > 0.0)
    {
        Ks_r = texture(material.specular, FragTexCoord).r;
        spec = pow(max(dot(viewDir, reflectDir), 0.0), Ks_r*Ks_r*32+0.00001);
    }
    else 
        spec = 0.f;
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    if(bModel)
    {
        if(bShowUV)
        {
            ambient = light.ambient * vec3(texture(material.grid, FragTexCoord))/16;
            diffuse = Kd * light.diffuse * diff * vec3(texture(material.grid, FragTexCoord));
            specular = Ks * light.specular * pow(max(dot(viewDir, reflectDir),0.0),32) * vec3(texture(material.grid, FragTexCoord));
        }
        else
        { 
            ambient = light.ambient * vec3(texture(material.diffuse, FragTexCoord))/16;
            diffuse = Kd * light.diffuse * diff * vec3(texture(material.diffuse, FragTexCoord));
            specular = Ks * light.specular * spec * vec3(texture(material.specular, FragTexCoord));
        }
    }
    else
    {
        ambient = light.ambient / 16;
        diffuse = light.diffuse * diff;
        specular = light.specular * pow(max(dot(viewDir, reflectDir),0.0),32);
    }
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = 2*dot(lightDir, normal)*normal - lightDir;
    float spec = 0.f;
    float Ks_r = 0.f;
    if(dot(normal, lightDir) > 0.0)
    {
        Ks_r = texture(material.specular, FragTexCoord).r;
        spec = pow(max(dot(viewDir, reflectDir), 0.0), Ks_r*Ks_r*32+0.00001);
    }
    else
        spec = 0.f;
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.falloff + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(-lightDir, normalize(light.direction)); 
    float epsilon = light.inner_angle - light.outer_angle;
    float intensity = clamp((theta - light.outer_angle) / epsilon, 0.0, 1.0);
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    if(bModel)
    {
        if(bShowUV)
        {
            ambient = light.ambient * vec3(texture(material.grid, FragTexCoord))/16;
            diffuse = Kd * light.diffuse * diff * vec3(texture(material.grid, FragTexCoord));
            specular = Ks * light.specular * pow(max(dot(viewDir, reflectDir),0.0),32) * vec3(texture(material.grid, FragTexCoord));
        }
        else
        { 
            ambient = light.ambient * vec3(texture(material.diffuse, FragTexCoord))/16;
            diffuse = Kd * light.diffuse * diff * vec3(texture(material.diffuse, FragTexCoord));
            specular = Ks * light.specular * spec * vec3(texture(material.specular, FragTexCoord));
        }
    }
    else
    {
        ambient = light.ambient / 16;
        diffuse = light.diffuse * diff;
        specular = light.specular * pow(max(dot(viewDir, reflectDir),0.0),32);
    }
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (diffuse + specular);
}
vec2 calcCylindricalUV(vec3 centVec)
{
    float theta = degrees(atan(centVec.z, centVec.x));
    theta += 180.f;

    float z = centVec.y;

    return vec2( 1 - (theta / 360.f), (z - min_.y)/ (max_.y - min_.y));
}

vec2 calcSphericalUV(vec3 centVec)
{
    float theta = degrees(atan(centVec.z / centVec.x));
    theta += 180.f;
    float pi = 180.f - acos(centVec.y / length(centVec)) * 180.f / acos(-1);

    return vec2( 1 - (theta / 360.f), (pi / 180.f));
}

vec2 calcCubeMap(vec3 vEntity)
{
    float x = vEntity.x;
    float y = vEntity.y;
    float z = vEntity.z;

    float absX = abs(x);
    float absY = abs(y);
    float absZ = abs(z);

    int isXPositive = x > 0 ? 1 : 0;
    int isYPositive = y > 0 ? 1 : 0;
    int isZPositive = z > 0 ? 1 : 0;

    float maxAxis, uc, vc;
    vec2 uv = vec2(0.0);

    // POSITIVE X
    if (bool(isXPositive) && (absX >= absY) && (absX >= absZ))
    {
        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = -z/absX;
        vc = y/absX;
    }

        // NEGATIVE X
    else if (!bool(isXPositive) && absX >= absY && absX >= absZ)
    {
        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = z/absX;
        vc = y/absX;
    }

        // POSITIVE Y
    else if (bool(isYPositive) && absY >= absX && absY >= absZ)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        maxAxis = absY;
        uc = x/absY;
        vc = -z/absY;
    }

        // NEGATIVE Y
    else if (!bool(isYPositive) && absY >= absX && absY >= absZ)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        maxAxis = absY;
        uc = x/absY;
        vc = z/absY;
    }

        // POSITIVE Z
    else if (bool(isZPositive) && absZ >= absX && absZ >= absY)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = x/absZ;
        vc = y/absZ;
    }

        // NEGATIVE Z
    else if (!bool(isZPositive) && absZ >= absX && absZ >= absY)
    {
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = -x/absZ;
        vc = y/absZ;
    }

    // Convert range from -1 to 1 to 0 to 1
    uv.s = 0.5f * (uc + 1.0f);
    uv.t = 0.5f * (vc + 1.0f);

    return uv;
}