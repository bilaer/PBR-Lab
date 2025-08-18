#include "scene.h"

// Update the local transformation matrix
void SceneNode::UpdateLocalTransform() {
    this->localTransform = glm::mat4(1.0f);  // Initialize as unit matrix

    // Translate
    this->localTransform = glm::translate(this->localTransform, this->position);

    // Rotation (XYZ order)
    this->localTransform = glm::rotate(this->localTransform, this->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    this->localTransform = glm::rotate(this->localTransform, this->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    this->localTransform = glm::rotate(this->localTransform, this->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    this->localTransform = glm::scale(this->localTransform, this->scale);
}

// Update the world transformation matrix
void SceneNode::UpdateWorldTransform() {
    if (this->parent) {
        // If there is a parent, combine the parent's world transform with this node's local transform
        this->worldTransform = parent->worldTransform * this->localTransform;
    } else {
        // If no parent, the world transform is equal to the local transform
        this->worldTransform = this->localTransform;
    }

    // Recursively update the world transformation matrix of child nodes
    for (auto& child : children) {
        child->UpdateWorldTransform();
    }
}

// Add a child node to the current node
void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child) {
    this->children.push_back(child);
    child->parent = shared_from_this();  // Set the parent of the child node
    child->UpdateWorldTransform();      // Update the child's world transform
}

// Upload material and transformation matrices to the shader
void SceneNode::UploadToShader(const std::shared_ptr<Shader>& shader) {
    shader->Use();
    this->material->UploadToShader(shader);  // Upload the material to the shader
    shader->SetUniform("model", this->worldTransform); // Upload model matrix
}

// Draw this node and all its children recursively
void SceneNode::Draw(const std::shared_ptr<Shader>& shader) {
    // Upload material and transformation matrices
    this->UploadToShader(shader);

    // Draw the mesh if it exists
    if (this->mesh) {
        this->mesh->Draw();
    }

    // Recursively draw all child nodes
    for (auto& child : this->children) {
        child->Draw(shader);
    }
}

//==================Scene========================
void Scene::AddNode(const std::shared_ptr<SceneNode>& node) {
    this->rootNodes.push_back(node);
}

void Scene::Render(const std::shared_ptr<Shader>& shader) {
    // 遍历所有根节点并绘制它们
    for (auto& rootNode : this->rootNodes) {
        rootNode->Draw(shader);  // draw recursively
    }
}