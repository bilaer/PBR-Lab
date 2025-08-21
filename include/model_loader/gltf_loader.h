#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "tiny_gltf.h"   // 你项目里已集成
#include "scene.h"       // 提供 SceneNode 定义与 SetLocalTransformMatrix 等
#include "geometry.h"    // 提供 Mesh / Vertex
#include "material.h"    // 提供 PBRMaterial

class GLTFLoader {
public:
    GLTFLoader() = default;

    // 与你的 GLB loader 接口一致：外面先 new 一个空的 SceneNode 传进来
    // 返回 true 表示加载+构建成功；所有内容会挂到 rootNode 下
    bool LoadFile(const std::string& filepath, const std::shared_ptr<SceneNode>& rootNode);

private:
    // ===== materials =====
    std::vector<std::shared_ptr<PBRMaterial>> m_materials; // 按 glTF material 索引对齐
    std::string m_baseDir;

    // ===== helpers =====
    static std::string GetBaseDir(const std::string& path);
    static std::string JoinPath(const std::string& base, const std::string& rel);

    // 将 glTF Texture 索引解析为磁盘路径（仅处理 .gltf 外链 / 非 dataURI）
    // 返回空字符串表示不可用（比如 .glb 内嵌 或 dataURI）
    std::string ResolveTexturePath(const tinygltf::Model& model, int textureIndex) const;

    // 解析 glTF 中的材质到你的 PBRMaterial
    void BuildMaterials(const tinygltf::Model& model);

    // 递归构建节点层级
    void BuildNodeRecursive(const tinygltf::Model& model, int nodeIndex,
                            const std::shared_ptr<SceneNode>& parent);

    // 从 glTF Primitive 构建 Mesh 与绑定材质，返回一个 SceneNode（带 mesh+material）
    std::shared_ptr<SceneNode> BuildPrimitiveNode(const tinygltf::Model& model,
                                                  const tinygltf::Primitive& prim, int materialIndex);

    // 读 attribute 辅助
    bool ReadVec3Attrib(const tinygltf::Model& model, int accessorIndex,
                        std::vector<glm::vec3>& out) const;

    bool ReadVec2Attrib(const tinygltf::Model& model, int accessorIndex,
                        std::vector<glm::vec2>& out) const;

    bool ReadIndices(const tinygltf::Model& model, int accessorIndex,
                     std::vector<unsigned int>& out) const;

    // 将 T/R/S 或 matrix 应用到 SceneNode
    void ApplyNodeTransform(const tinygltf::Node& n, const std::shared_ptr<SceneNode>& node) const;
};
