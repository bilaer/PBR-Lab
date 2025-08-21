#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "tiny_gltf.h"
#include "scene.h"      // SceneNode
#include "geometry.h"   // Mesh, Vertex
#include "material.h"   // PBRMaterial
#include <glm/glm.hpp>

class GLTFLoader {
public:
    GLTFLoader() = default;

    // 与你的 GLB loader 一致：外部先 new 一个空的 SceneNode 传进来
    // 返回 true 表示加载成功，所有 glTF 内容会挂在 rootNode 下
    bool LoadFile(const std::string& filepath, const std::shared_ptr<SceneNode>& rootNode);

private:
    // 材质数组：索引与 glTF material 对齐
    std::vector<std::shared_ptr<PBRMaterial>> m_materials;
    std::string m_baseDir;

    // ---------------- helpers ----------------
    static std::string GetBaseDir(const std::string& path);
    static std::string JoinPath(const std::string& base, const std::string& rel);
    static bool EndsWithCaseInsensitive(const std::string& s, const char* suf);

    // 仅处理 .gltf 外链；.glb 内嵌 / dataURI 返回空串
    std::string ResolveTexturePath(const tinygltf::Model& model, int textureIndex) const;

    // 材质 / 节点 / primitive 构建
    void BuildMaterials(const tinygltf::Model& model);
    void BuildNodeRecursive(const tinygltf::Model& model, int nodeIndex,
                            const std::shared_ptr<SceneNode>& parent);
    std::shared_ptr<SceneNode> BuildPrimitiveNode(const tinygltf::Model& model,
                                                  const tinygltf::Primitive& prim, int materialIndex);

    // attribute & index 读取
    static bool ReadVec3Attrib(const tinygltf::Model& model, int accessorIndex,
                               std::vector<glm::vec3>& out);
    static bool ReadVec2Attrib(const tinygltf::Model& model, int accessorIndex,
                               std::vector<glm::vec2>& out);
    static bool ReadVec4Attrib(const tinygltf::Model& model, int accessorIndex,
                               std::vector<glm::vec4>& out);
    static bool ReadIndices(const tinygltf::Model& model, int accessorIndex,
                            std::vector<unsigned int>& out);

    // 应用 glTF 节点变换到 SceneNode
    static void ApplyNodeTransform(const tinygltf::Node& n, const std::shared_ptr<SceneNode>& node);
};
