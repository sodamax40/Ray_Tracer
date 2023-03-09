#include "rt_raytracing.h"
#include "rt_ray.h"
#include "rt_hitable.h"
#include "rt_sphere.h"
#include "rt_triangle.h"
#include "rt_box.h"
#include "cg_utils2.h" // Used for OBJ-mesh loading
#include <stdlib.h>    // Needed for drand48()
#include <random>
#include <material.h>

namespace rt
{

    // Store scene (world) in a global variable for convenience
    struct Scene
    {
        Sphere ground;
        std::vector<Sphere> spheres;
        std::vector<Box> boxes;
        std::vector<Triangle> mesh;
        Box mesh_bbox;
    } g_scene;

    bool hit_world(const Ray &r, float t_min, float t_max, HitRecord &rec, RTContext &rtx)
    {
        HitRecord temp_rec;
        bool hit_anything = false;
        float closest_so_far = t_max;

        if (g_scene.ground.hit(r, t_min, closest_so_far, temp_rec))
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
        for (unsigned int i = 0; i < g_scene.spheres.size(); ++i)
        {
            if (g_scene.spheres[i].hit(r, t_min, closest_so_far, temp_rec))
            {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        for (unsigned int i = 0; i < g_scene.boxes.size(); ++i)
        {
            if (g_scene.boxes[i].hit(r, t_min, closest_so_far, temp_rec))
            {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        // Adding bounding box test
        Sphere bunny_ball(rtx.center, rtx.radius);
        if (bunny_ball.hit(r, t_min, closest_so_far, temp_rec))
        {
            for (unsigned int i = 0; i < g_scene.mesh.size(); ++i)
            {

                if (g_scene.mesh[i].hit(r, t_min, closest_so_far, temp_rec))
                {

                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }
        }
        return hit_anything;
    }

    // This function should be called recursively (inside the function) for
    // bouncing rays when you compute the lighting for materials, like this
    //
    // if (hit_world(...)) {
    //     ...
    //     return color(rtx, r_bounce, max_bounces - 1);
    // }
    //
    // See Chapter 7 in the "Ray Tracing in a Weekend" book
    glm::vec3 color(RTContext &rtx, const Ray &r, int max_bounces)
    {
        if (max_bounces < 0)
            return glm::vec3(0.0f);

        HitRecord rec;
        if (hit_world(r, 0.0f, 9999.0f, rec, rtx))
        {
            rec.normal = glm::normalize(rec.normal); // Always normalise before use!
            if (rtx.show_normals)
            {
                return rec.normal * 0.5f + 0.5f;
            }
            else
            {
                Ray scattered;
                glm::vec3 attenuation;
                if (rec.mat_ptr->scatter(r, rec, attenuation, scattered, rtx.fuzz))
                {
                    return attenuation * color(rtx, scattered, rtx.max_bounces - 1);
                }
                else
                {
                    return glm::vec3(0, 0, 0);
                }
            }
        }

        // If no hit, return sky color
        glm::vec3 unit_direction = glm::normalize(r.direction());
        float t = 0.5f * (unit_direction.y + 1.0f);
        return (1.0f - t) * rtx.ground_color + t * rtx.sky_color;
    }

    // MODIFY THIS FUNCTION!
    void setupScene(RTContext &rtx, const char *filename)
    {
        g_scene.ground = Sphere(glm::vec3(0.0f, -1000.5f, 0.0f), 1000.0f, new lambertian(glm::vec3(0.8, 0.3, 0.3)));
        // add more balls here
        g_scene.spheres = {
            Sphere(glm::vec3(2.0f, 0.0f, 0.0f), 0.5f, new lambertian(glm::vec3(0.8, 0.3, 0.7))),
            Sphere(glm::vec3(1.5f, 0.0f, 0.8f), 0.5f, new fuzz_metal(glm::vec3(2.0, 2.0, 2.0))),
            Sphere(glm::vec3(1.5f, 0.0f, 0.0f), 0.5f, new fuzz_metal(glm::vec3(0.6, 0.6, 0.6))),
            Sphere(glm::vec3(-1.5f, 0.0f, 2.0f), 0.5f, new lambertian(glm::vec3(0.8, 0.8, 0.1))),
            Sphere(glm::vec3(0.8f, 0.0f, 1.0f), 0.5f, new fuzz_metal(glm::vec3(0.8, 1.1, 0.7))),
            Sphere(glm::vec3(-0.5f, 0.0f, -1.5f), 0.5f, new lambertian(glm::vec3(0.0, 0.0, 1.0))),
            Sphere(glm::vec3(1.2f, 0.0f, 2.5f), 0.5f, new fuzz_metal(glm::vec3(1.0, 1.0, 1.0))),
            Sphere(glm::vec3(1.5f, 0.0f, -2.5f), 0.5f, new lambertian(glm::vec3(1.0, 0.1, 1.8))),
        };

        // g_scene.boxes = {
        //     Box(glm::vec3(0.0f, -0.25f, 0.0f), glm::vec3(0.25f)),
        //     Box(glm::vec3(1.0f, -0.25f, 0.0f), glm::vec3(0.25f)),
        //     Box(glm::vec3(-1.0f, -0.25f, 0.0f), glm::vec3(0.25f)),
        // };

        // This part is adding the bunny
        cg::OBJMesh mesh;
        cg::objMeshLoad(mesh, filename);
        g_scene.mesh.clear();

        // bounding sphere set up
        glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < mesh.indices.size(); i += 3)
        {

            int i0 = mesh.indices[i + 0];
            int i1 = mesh.indices[i + 1];
            int i2 = mesh.indices[i + 2];
            glm::vec3 v0 = mesh.vertices[i0] + glm::vec3(0.0f, 0.135f, 0.0f);
            glm::vec3 v1 = mesh.vertices[i1] + glm::vec3(0.0f, 0.135f, 0.0f);
            glm::vec3 v2 = mesh.vertices[i2] + glm::vec3(0.0f, 0.135f, 0.0f);

            center += mesh.vertices[i0] + mesh.vertices[i1] + mesh.vertices[i2];

            g_scene.mesh.push_back(Triangle(v0, v1, v2, new metal(glm::vec3(1.0, 1.0, 1.0))));
        }

        // calculate the bounding sphere center
        center[0] = center[0] / mesh.indices.size();
        center[1] = center[1] / mesh.indices.size();
        center[2] = center[2] / mesh.indices.size();
        // calculate the bounding sphere radius
        float radius = 0.0;
        for (const auto &vertex : mesh.vertices)
        {
            float distance = glm::distance(vertex, center);
            if (distance > radius)
            {
                radius = distance;
            }
        }

        rtx.center = center;
        rtx.radius = radius;
    }

    // MODIFY THIS FUNCTION!
    void updateLine(RTContext &rtx, int y)
    {
        int nx = rtx.width;
        int ny = rtx.height;
        float aspect = float(nx) / float(ny);
        glm::vec3 lower_left_corner(-1.0f * aspect, -1.0f, -1.0f);
        glm::vec3 horizontal(2.0f * aspect, 0.0f, 0.0f);
        glm::vec3 vertical(0.0f, 2.0f, 0.0f);
        glm::vec3 origin(0.0f, 0.0f, 0.0f);
        glm::mat4 world_from_view = glm::inverse(rtx.view);

        // You can try parallelising this loop by uncommenting this line:
        //#pragma omp parallel for schedule(dynamic)
        for (int x = 0; x < nx; ++x)
        {
            // Implement anti-aliasing by adding per-sample jitter to the camera rays.
            float u, v;
            if (rtx.anti_aliasing)
            {
                u = (float(x) + random_double(0.0, 1.0)) / float(nx);
                v = (float(y) + random_double(0.0, 1.0)) / float(ny);
            }
            else
            {
                u = float(x) / float(nx);
                v = float(y) / float(ny);
            }
            Ray r(origin, lower_left_corner + u * horizontal + v * vertical);
            r.A = glm::vec3(world_from_view * glm::vec4(r.A, 1.0f));
            r.B = glm::vec3(world_from_view * glm::vec4(r.B, 0.0f));

            // Note: in the RTOW book, they have an inner loop for the number of
            // samples per pixel. Here, you do not need this loop, because we want
            // some interactivity and accumulate samples over multiple frames
            // instead (until the camera moves or the rendering is reset).

            if (rtx.current_frame <= 0)
            {
                // Here we make the first frame blend with the old image,
                // to smoothen the transition when resetting the accumulation
                glm::vec4 old = rtx.image[y * nx + x];
                rtx.image[y * nx + x] = glm::clamp(old / glm::max(1.0f, old.a), 0.0f, 1.0f);
            }
            glm::vec3 c = color(rtx, r, rtx.max_bounces);
            rtx.image[y * nx + x] += glm::vec4(c, 1.0f);
        }
    }

    void updateImage(RTContext &rtx)
    {
        if (rtx.freeze)
            return;                               // Skip update
        rtx.image.resize(rtx.width * rtx.height); // Just in case...

        updateLine(rtx, rtx.current_line % rtx.height);

        if (rtx.current_frame < rtx.max_frames)
        {
            rtx.current_line += 1;
            if (rtx.current_line >= rtx.height)
            {
                rtx.current_frame += 1;
                rtx.current_line = rtx.current_line % rtx.height;
            }
        }
    }

    void resetImage(RTContext &rtx)
    {
        rtx.image.clear();
        rtx.image.resize(rtx.width * rtx.height);
        rtx.current_frame = 0;
        rtx.current_line = 0;
        rtx.freeze = false;
    }

    void resetAccumulation(RTContext &rtx)
    {
        rtx.current_frame = -1;
    }

} // namespace rt
