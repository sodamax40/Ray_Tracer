#pragma once

#include "rt_hitable.h"

namespace rt {

class Box : public Hitable {
  public:
    Box() {}
    Box(const glm::vec3 &cen, const glm::vec3 r) : center(cen), radius(r){};
    virtual bool hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const;

    glm::vec3 center;
    glm::vec3 radius;
};

// Ray-box test adapted from branchless code at
// https://tavianator.com/fast-branchless-raybounding-box-intersections/
bool Box::hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const
{
    glm::vec3 oc = r.origin() - center;
    glm::vec3 t0 = (-radius - oc) / r.direction();
    glm::vec3 t1 = (radius - oc) / r.direction();
    float temp1 = glm::compMax(glm::min(t0, t1));
    float temp2 = glm::compMin(glm::max(t1, t0));
    float temp = (temp1 < t_max && temp1 > t_min) ? temp1 : temp2;
    if (temp1 <= temp2 && temp1 < t_max && temp > t_min) {
        rec.t = temp;  // TODO Handle case where origin is inside box
        rec.p = r.point_at_parameter(rec.t);
        glm::vec3 npc = (rec.p - center) / radius;
        rec.normal = glm::sign(npc) * glm::step(glm::compMax(glm::abs(npc)), glm::abs(npc));
        return true;
    }
    return false;
}

}  // namespace rt
