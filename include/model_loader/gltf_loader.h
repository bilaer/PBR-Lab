#pragma once
#include <memory>
#include <string>
#include <vector>
#include "tiny_gltf.h"
#include "scene.h"
#include "geometry.h"
#include "material.h"

// Vertex: { position, normals, texCoord, tangent, bitangent }
// Mesh API: SetVertices/SetIndices/SetupBuffers()
// SceneNode: SetLocalTransformMatrix/UpdateLocalTransform/UpdateWorldTransform/AddChild(...)

class GlbLoader {
public:
    // Load .glb or .gltf and attach nodes under the given SceneNode.
    // Returns true on success.
    bool LoadFile(const std::string& path, const std::shared_ptr<SceneNode>& parent);

private:
    // Recursively build SceneNode tree from a glTF node.
    std::shared_ptr<SceneNode> BuildNodeRecursive(const tinygltf::Model& model,
                                                  int nodeIndex,
                                                  const std::string& gltfPath);

    // Build Mesh from a glTF primitive.
    std::shared_ptr<Mesh> LoadMesh(const tinygltf::Model& model,
                                   const tinygltf::Primitive& primitive);

    // Build PBRMaterial from a glTF material definition.
    std::shared_ptr<PBRMaterial> LoadMaterial(const tinygltf::Model& model,
                                              int materialIndex,
                                              const std::string& gltfPath);

    // ---- helpers ----
    // Return a pointer to the element data inside glTF buffers.
    static const unsigned char* AccessorElemPtr(const tinygltf::Model& model,
                                                const tinygltf::Accessor& acc,
                                                size_t elemIndex);

    // Extract directory from path.
    static std::string DirOf(const std::string& p);

    // Save embedded image to a temporary file (png) and return path,
    // or return resolved path if external uri exists.
    static std::string ResolveImagePathOrTemp(const std::string& baseDir,
                                              const tinygltf::Image& img,
                                              const std::string& hintFileName);

    // Fill bitangents using tangent.xyz, normal and a default w sign (+1).
    static void FillBitangentsFromTangentW(std::vector<Vertex>& vertices, float defaultW = 1.0f);
};
