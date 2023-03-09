#pragma once
#ifndef MATERIALH
#define MATERIALH

#include "rt_ray.h"
#include "rt_hitable.h"
#include <random>
#include <stdlib.h>

struct HitRecord;

namespace rt
{
    // random number generator
    inline double random_double(double min, double max)
    {
        static std::uniform_real_distribution<double> distribution(min, max);
        static std::mt19937 generator;
        return distribution(generator);
    }

    inline static glm::vec3 random(double min, double max)
    {
        return glm::vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

    glm::vec3 random_in_unit_sphere()
    {
        while (true)
        {
            glm::vec3 p = random(-1, 1);
            if (p[0] * p[0] + p[1] * p[1] + p[2] * p[2] >= 1.0)
                continue;
            else
                return p;
        }
    }

    glm::vec3 reflect(const glm::vec3 &v, const glm::vec3 &n)
    {
        return v - 2 * glm::dot(v, n) * n;
    }

    class material
    {
    public:
        virtual bool scatter(
            const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered, const float fuzz) const = 0;
    };

    class lambertian : public material
    {
    public:
        lambertian(const glm::vec3 &a) : albedo(a) {}
        virtual bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered, const float fuzz) const
        {
            glm::vec3 target = rec.p + rec.normal + glm::normalize(random_in_unit_sphere());
            scattered = Ray(rec.p, target - rec.p);
            attenuation = albedo;
            return true;
        }

        glm::vec3 albedo;
    };

    class metal : public material
    {
    public:
        metal(const glm::vec3 &a) : albedo(a) {}
        virtual bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered, const float fuzz) const
        {
            glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
            scattered = Ray(rec.p, reflected);
            attenuation = albedo;
            return (glm::dot(scattered.direction(), rec.normal) > 0);
        }
        glm::vec3 albedo;
    };

    class fuzz_metal : public material
    {
    public:
        fuzz_metal(const glm::vec3 &a) : albedo(a) {}
        virtual bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered, const float fuzz) const
        {
            glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
            glm::vec3 fuzz_vec = random_in_unit_sphere();
            fuzz_vec[0] = fuzz_vec[0] * fuzz;
            fuzz_vec[1] = fuzz_vec[1] * fuzz;
            fuzz_vec[2] = fuzz_vec[2] * fuzz;
            scattered = Ray(rec.p, reflected + fuzz_vec);
            attenuation = albedo;
            return (glm::dot(scattered.direction(), rec.normal) > 0);
        }
        glm::vec3 albedo;
        // float fuzz;
    };
}
#endif