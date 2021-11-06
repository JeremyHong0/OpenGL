#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <string>
#include <glm/glm.hpp>
#include <utility>

class Light
{
public:
    enum class LightType
    {
        Point = 0,
        Direction,
        Spot
    };

    Light() : position(glm::vec3(0.f)), La(glm::vec3(0.f)),
    Ld(glm::vec3(0.f)), Ls(glm::vec3(0.f)), Ltype("Point"), lightCount(0) {}

    Light(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 spec, std::string type)
        : position(pos), La(ambient), Ld(diffuse), Ls(spec), Ltype(std::move(type))
    {
        ++lightCount;
    }

    ~Light()
    {
    }

    glm::vec3 position;

    glm::vec3 La;
    glm::vec3 Ld;
    glm::vec3 Ls;

    int lightCount = 0;
    std::string Ltype;

    glm::vec3 getLightPosition() const { return position; }
    glm::vec3 getLightAmbient() const { return La; }
    glm::vec3 getLightDiffuse() const { return Ld; }
    glm::vec3 getLightSpec() const { return Ls; }
    int getLightCount() const { return lightCount; }
    
};


#endif

