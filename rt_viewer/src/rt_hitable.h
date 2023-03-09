#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

namespace rt
{
    class material;
    struct HitRecord
    {
        float t;
        glm::vec3 p;
        glm::vec3 normal;
        material *mat_ptr;
    };

    // class material;

    class Hitable
    {
    public:
        virtual bool hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const = 0;
    };

} // namespace rt
