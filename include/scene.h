#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "geometry.h"
#include "material.h"
#include "bounding_box/aabb.h"
#include "shader.h"
#include <vector>
#include <memory>

// SceneNode class, which is derived from std::enable_shared_from_this to allow creating shared_ptr of itself
class SceneNode : public std::enable_shared_from_this<SceneNode> {
public:
    // Constructor that initializes the mesh and material
    SceneNode(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<PBRMaterial>& material);
    SceneNode(): mesh(nullptr), material(nullptr), localAABB(nullptr), worldAABB(nullptr) {};

    // transform getter and setter
    void SetPosition(const glm::vec3& p);
    void SetRotation(const glm::vec3& r); // Rotation in radians
    void SetScale(const glm::vec3& s);
    const glm::vec3 GetPosition() const { return this->position; };
    const glm::vec3 GetRotation() const { return this->rotation; };
    const glm::vec3 GetScale() const { return this->scale; };

    // API for glb 
    void SetLocalTransformMatrix(const glm::mat4& m);
    void SetMesh(const std::shared_ptr<Mesh>& mesh);
    void SetMaterial(const std::shared_ptr<PBRMaterial>& material) { this->material = material; };
    std::shared_ptr<Mesh> GetMesh() const { return this->mesh; };
    std::shared_ptr<PBRMaterial> GetMaterial() const { return this->material; };
    
    // Getter functions for local and world transformation matrices
    glm::mat4 GetLocalTransform() const { return this->localTransform; }
    glm::mat4 GetWorldTransform() const { return this->worldTransform; }

    // Function to add a child node
    void AddChild(const std::shared_ptr<SceneNode>& child);
    std::vector<std::shared_ptr<SceneNode>> GetChildren() { return this->children; };

    // Update the local and world transformation matrices
    void UpdateLocalTransform();
    void UpdateWorldTransform();

    // Function to upload material to the shader
    void UploadToShader(const std::shared_ptr<Shader>& shader);
    
    // Recursive function to draw this node and its children
    void Draw(const std::shared_ptr<Shader>& shader); 

private:
    std::shared_ptr<Mesh> mesh;                   // Mesh object
    std::shared_ptr<PBRMaterial> material;        // Material object
    glm::mat4 localTransform;                     // Local transformation matrix
    glm::mat4 worldTransform;                     // World transformation matrix
    glm::vec3 position;                           // Position of the node
    glm::vec3 rotation;                           // Rotation in pitch/yaw/roll
    glm::vec3 scale;                              // Scale of the node
    std::shared_ptr<SceneNode> parent;            // Parent node
    std::vector<std::shared_ptr<SceneNode>> children;  // Child nodes
    std::shared_ptr<AABB> worldAABB = nullptr;
    std::shared_ptr<AABB> localAABB = nullptr;
};

class Scene {
    public:
        Scene() {};

        // Add root node into root vector
        void AddNode(const std::shared_ptr<SceneNode>& node);

        // Draw a node with GL state derived from blending/depthWrite and material doubleSided
        void DrawNodeWithState(const std::shared_ptr<SceneNode>& node,
                                const std::shared_ptr<Shader>& shader,
                                bool blending, bool depthWrite);
        // Render all the root scene node
        void Render(const std::shared_ptr<Shader>& shader, glm::vec3 camPos);
    private:
        std::vector<std::shared_ptr<SceneNode>> rootNodes;

        // Aplha queue
        std::vector<std::shared_ptr<SceneNode>> queueOpaque;
        std::vector<std::shared_ptr<SceneNode>> queueMasked;
        std::vector<std::shared_ptr<SceneNode>> queueTransparent;  
        void SortTransparent(const glm::vec3& camPos); // Sort transparent queue
        void CollectQueue(const std::shared_ptr<SceneNode>& node); // collect and sort opaque, masked and transparent object
};