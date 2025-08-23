#include "scene.h"

SceneNode::SceneNode(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<PBRMaterial>& material): 
    mesh(mesh), 
    material(material),      
    localTransform(glm::mat4(1.0f)), 
    worldTransform(glm::mat4(1.0f)),
    position(0.0f), 
    rotation(0.0f), 
    scale(1.0f), 
    parent(nullptr) {
        // Initialize local aabb and world aabb
        if(this->mesh) {
            this->localAABB = std::make_shared<AABB>(mesh);
            this->worldAABB = std::make_shared<AABB>(mesh);
        } else {
            this->localAABB = nullptr;
            this->worldAABB = nullptr;
        }
}

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

    // Apply transform to world aabb
    if(this->worldAABB) {
        this->worldAABB->UpdateBox(this->worldTransform);
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
    shader->SetUniform("model", this->worldTransform); // set model first

    if (this->material) {
        this->material->UploadToShader(shader);
    }
}


// Draw this node and all its children recursively
void SceneNode::Draw(const std::shared_ptr<Shader>& shader) {
    // Draw if current node contains mesh
    if (this->mesh) {
        this->UploadToShader(shader);
        this->mesh->Draw();
    }

    // Recursively draw all child node
    for (auto& child : this->children) {
        child->Draw(shader);
    }
}

void SceneNode::SetLocalTransformMatrix(const glm::mat4& m) {
    this->localTransform = m;
    this->UpdateWorldTransform();
}

// Transform getter and setters
void SceneNode::SetPosition(const glm::vec3& p) {
    this->position = p;
    this->UpdateLocalTransform();
    this->UpdateWorldTransform(); // propagate transform effect to child
}

void SceneNode::SetRotation(const glm::vec3& r) {
    this->rotation = r;
    this->UpdateLocalTransform();
    this->UpdateWorldTransform();
}

void SceneNode::SetScale(const glm::vec3& s) {
    this->scale = s;
    this->UpdateLocalTransform();
    this->UpdateWorldTransform();
}

void SceneNode::SetMesh(const std::shared_ptr<Mesh>& mesh) {
    this->mesh = mesh;
    if (mesh) {
        localAABB = std::make_shared<AABB>(mesh);
        worldAABB = std::make_shared<AABB>(mesh);
        if (parent) {
            worldAABB->UpdateBox(worldTransform);
        }
    } else {
        localAABB.reset();
        worldAABB.reset();
    }
}

//==================Scene========================
void Scene::AddNode(const std::shared_ptr<SceneNode>& node) {
    this->rootNodes.push_back(node);
}

// Rendering all objects in the scene
void Scene::Render(const std::shared_ptr<Shader>& shader, glm::vec3 camPos) {
    // Clear queues
    this->queueOpaque.clear();
    this->queueMasked.clear();
    this->queueTransparent.clear();

    // Sort all the node to transparent, mask and opaque node
    for (auto& root : this->rootNodes) {
        this->CollectQueue(root);
    }

     // Opaque first (no blending, write depth)
    if (!queueOpaque.empty()) {
        for (auto& n : queueOpaque) {
            DrawNodeWithState(n, shader, /*blending=*/false, /*depthWrite=*/true);
        }
    }

    // Masked next, opaque part will be discarded
    if (!queueMasked.empty()) {
        for (auto& n : queueMasked) {
            DrawNodeWithState(n, shader, /*blending=*/false, /*depthWrite=*/true);
        }
    }

    // Transparent last: sort back-to-front, enable blending, disable depth write
    if (!queueTransparent.empty()) {
        SortTransparent(camPos);
        for (auto& n : queueTransparent) {
            DrawNodeWithState(n, shader, /*blending=*/true, /*depthWrite=*/false);
        }
    }

    // Reset default state
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

// Sort all the transparent objects using their squared distance
void Scene::SortTransparent(const glm::vec3& camPos) {
    // squared distance helper
    auto dist2 = [&](const std::shared_ptr<SceneNode>& node) -> float {
        glm::vec4 p4 = node->GetWorldTransform() * glm::vec4(0, 0, 0, 1); // world-space origin
        glm::vec3 p  = glm::vec3(p4);
        glm::vec3 w  = p - camPos;
        return glm::dot(w, w);
    };

    // Compare distance
    auto compare = [&](const std::shared_ptr<SceneNode>& a,
                       const std::shared_ptr<SceneNode>& b) {
        return dist2(a) > dist2(b);
    };

    // sort back-to-front for blending
    std::sort(queueTransparent.begin(), queueTransparent.end(), compare);
}

// collect and sort opaque, masked and transparent object
void Scene::CollectQueue(const std::shared_ptr<SceneNode>& node) {
    if(!node) {
        return;
    }

    if (auto mat = node->GetMaterial()) {
        switch (mat->GetAlphaMode()) {
            case PBRMaterial::AlphaMode::Opaque:
                queueOpaque.push_back(node);
                break;
            case PBRMaterial::AlphaMode::Mask:
                queueMasked.push_back(node);
                break;
            case PBRMaterial::AlphaMode::Blend:
                queueTransparent.push_back(node);
                break;
            default:
                queueOpaque.push_back(node);
                break;
        }
    }

    // Recurse collection
    for (auto& c : node->GetChildren()) this->CollectQueue(c);
}

// Draw a node with GL state derived from blending/depthWrite and material doubleSided
void Scene::DrawNodeWithState(const std::shared_ptr<SceneNode>& node, const std::shared_ptr<Shader>& shader, bool blending, bool depthWrite) {
    // Blending & depth write
    if (blending) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);

    // Face culling from material
    bool doubleSided = false;
    if (auto mat = node->GetMaterial()) doubleSided = mat->IsDoubleSided();

    if (doubleSided) {
        glDisable(GL_CULL_FACE); // Disable the face culling if the material is doublesided
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    // Draw
    node->Draw(shader);
}