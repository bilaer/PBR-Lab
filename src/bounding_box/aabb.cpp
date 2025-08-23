#include "bounding_box/aabb.h"
#include "config.h"
#include <iostream>



AABB::AABB(const std::shared_ptr<Mesh>& mesh) {
    this->min = glm::vec3(std::numeric_limits<float>::max());
    this->max = glm::vec3(std::numeric_limits<float>::lowest());
    this->boxIndices = {
        0, 1, 1, 3, 3, 2, 2, 0,  // bottom box
        4, 5, 5, 7, 7, 6, 6, 4,  // top box
        0, 4, 1, 5, 2, 6, 3, 7   // vertical connections
    };

    // Get min and max point from object mesh data
    const auto& vertices = mesh->GetVertices();  
    for (const auto& vertex : vertices) {
        this->max = glm::max(vertex.position, this->max);
        this->min = glm::min(vertex.position, this->min);
    }
    // Generate aabbox's vertices data using min and max points
    this->GenerateBoxVertices();
    // Save the orignal vertex for model matrix transformation
    this->originalVertex = this->boxVertex;
}

AABB::AABB() {
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::lowest());
}

bool AABB::IsValid() const {
    return (min.x <= max.x && min.y <= max.y && min.z <= max.z);
}

// Update min and max point using given vertices
void AABB::CalMinMax(const std::vector<glm::vec3>& vertices) {
    for(const auto& vertex : vertices) {
        this->min = glm::min(this->min, vertex);
        this->max = glm::max(this->max, vertex);
    }
}

// When object is moved or rotated, using this function to update the box data
void AABB::UpdateBox(const glm::mat4& model) {
    std::vector<glm::vec3> newVertex(this->originalVertex.size());

    for (size_t i = 0; i < this->originalVertex.size(); ++i) {
        newVertex[i] = glm::vec3(model * glm::vec4(this->originalVertex[i], 1.0f));
    }

    // Recalculate min and max value
    this->min = glm::vec3(std::numeric_limits<float>::max());
    this->max = glm::vec3(std::numeric_limits<float>::lowest());
    this->CalMinMax(newVertex);

    // Regnerate the box
    this->GenerateBoxVertices();

}

void AABB::GenerateBoxVertices() {
    this->boxVertex = {
        // fix y axis, change z and y
        glm::vec3(min.x, min.y, min.z),  // 0
        glm::vec3(min.x, min.y, max.z),  // 1
        glm::vec3(min.x, max.y, min.z),  // 2
        glm::vec3(min.x, max.y, max.z),  // 3
        glm::vec3(max.x, min.y, min.z),  // 4
        glm::vec3(max.x, min.y, max.z),  // 5
        glm::vec3(max.x, max.y, min.z),  // 6
        glm::vec3(max.x, max.y, max.z)   // 7
    };
}

// Test if ray is intersecting with aabb box 
// based on slab algorithm
bool AABB::IntersectRay(const Ray& ray, float& tminOut) {
    float txmin, txmax, tymin, tymax, tzmin, tzmax;
    // Check if ray is parallel to x axis
    if(std::abs(ray.direction.x) < EPSILON) {
        if (ray.origin.x < this->min.x || ray.origin.x > this->max.x) {
            return false;
        }
        txmin = std::numeric_limits<float>::lowest();
        txmax = std::numeric_limits<float>::max();
    } else {
        float tx1 = (this->min.x - ray.origin.x) / ray.direction.x;
        float tx2 = (this->max.x - ray.origin.x) / ray.direction.x;
        txmin = glm::min(tx1, tx2);
        txmax = glm::max(tx1, tx2);
    }
    // Check if ray is parallel to y axis
    if(std::abs(ray.direction.y) < EPSILON) {
        if (ray.origin.y < this->min.y || ray.origin.y > this->max.y) {
            return false;
        }
        tymin = std::numeric_limits<float>::lowest();
        tymax = std::numeric_limits<float>::max();
    } else {
        float ty1 = (this->min.y - ray.origin.y) / ray.direction.y;
        float ty2 = (this->max.y - ray.origin.y) / ray.direction.y;
        tymin = glm::min(ty1, ty2);
        tymax = glm::max(ty1, ty2);
    }
    // Check if ray is parallel to z axis
    if(std::abs(ray.direction.z) < EPSILON) {
        if (ray.origin.z < this->min.z || ray.origin.z > this->max.z) {
            return false;
        }
        tzmin = std::numeric_limits<float>::lowest();
        tzmax = std::numeric_limits<float>::max();
    } else {
        float tz1 = (this->min.z - ray.origin.z) / ray.direction.z;
        float tz2 = (this->max.z - ray.origin.z) / ray.direction.z;
        tzmin = glm::min(tz1, tz2);
        tzmax = glm::max(tz1, tz2);
    }
    // Compute the overlapping interval [tmin, tmax] across all three axes.
    // - tmin: the latest entry time — when the ray enters the overlapping region, so using max
    // - tmax: the earliest exit time — when the ray exits the overlapping region.
    // If tmin > tmax, there's no intersection because the ray misses the box in at least one axis.
    float tmin = glm::max(glm::max(txmin, tymin), tzmin);
    float tmax = glm::min(glm::min(txmax, tymax), tzmax);

    // return false if it is empty set
    if(tmin > tmax) {
        return false;
    } else {
        tminOut = tmin;
        return true;
    }
}



