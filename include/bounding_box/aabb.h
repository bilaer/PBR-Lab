#pragma once
#include "geometry.h"
#include "shader.h"
#include "camera.h"
#include <vector>

class AABB {
    public:
        AABB(const std::shared_ptr<Mesh>& mesh);
        AABB();
        ~AABB() = default;
        // Move the abbbox to the world space and recalculate the min and max
        void UpdateBox(const glm::mat4& model);
        bool IntersectRay(const Ray& ray, float& tminOut);
        bool IsValid() const;
        // Setter/getter
        const glm::vec3& GetMin() const { return min; }
        const glm::vec3& GetMax() const { return max; }
        const std::vector<glm::vec3>& GetVertices() const { return boxVertex; }
    private:
        glm::vec3 min;
        glm::vec3 max;
        std::vector<glm::vec3> boxVertex;
        std::vector<glm::vec3> originalVertex;
        std::vector<unsigned int> boxIndices;
        void CalMinMax(const std::vector<glm::vec3>& vertices);
        void GenerateBoxVertices();
};