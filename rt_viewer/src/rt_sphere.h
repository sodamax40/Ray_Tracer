#pragma once

#include "rt_hitable.h"

namespace rt
{

    class Sphere : public Hitable
    {
    public:
        Sphere() {}
        Sphere(const glm::vec3 &cen, float r) : center(cen), radius(r){};
        Sphere(const glm::vec3 &cen, float r, material *mat) : center(cen), radius(r), material_ptr(mat){};
        virtual bool hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const;

        glm::vec3 center;
        float radius;
        material *material_ptr;
    };

    // Ray-sphere test from "Ray Tracing in a Weekend" book (page 16)
    bool Sphere::hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const
    {
        glm::vec3 oc = r.origin() - center;
        float a = glm::dot(r.direction(), r.direction());
        float b = 2.0f * glm::dot(oc, r.direction());
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = (b * b - 4 * a * c);
        if (discriminant > 0.0f)
        {
            float temp1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
            float temp2 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
            float temp = (temp1 < t_max && temp1 > t_min) ? temp1 : temp2;
            if (temp < t_max && temp > t_min)
            {
                rec.t = temp;
                rec.p = r.point_at_parameter(rec.t);
                rec.normal = (rec.p - center) / radius;
                rec.mat_ptr = material_ptr;
                return true;
            }
        }
        return false;
    }

} // namespace rt
