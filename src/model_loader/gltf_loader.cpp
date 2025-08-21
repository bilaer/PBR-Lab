#include "model_loader/gltf_loader.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ---------- path helpers ----------
std::string GLTFLoader::GetBaseDir(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return std::string();
    return path.substr(0, pos + 1);
}
std::string GLTFLoader::JoinPath(const std::string& base, const std::string& rel) {
    if (base.empty()) return rel;
    if (rel.empty()) return base;
    if (base.back() == '/' || base.back() == '\\') return base + rel;
    return base + '/' + rel;
}

// NOTE: 仅处理 .gltf 外链贴图。对于 .glb 的内嵌/ dataURI 返回空串（上层选择忽略或自行扩展写盘方案）
std::string GLTFLoader::ResolveTexturePath(const tinygltf::Model& model, int textureIndex) const {
    if (textureIndex < 0 || textureIndex >= (int)model.textures.size()) return std::string();
    int imageIndex = model.textures[textureIndex].source;
    if (imageIndex < 0 || imageIndex >= (int)model.images.size()) return std::string();
    const auto& img = model.images[imageIndex];

    // data:image/...;base64,xxxxx 不做解析
    if (img.uri.empty() || img.uri.rfind("data:", 0) == 0) {
        return std::string();
    }
    return JoinPath(m_baseDir, img.uri);
}

// ---------- main entry ----------
bool GLTFLoader::LoadFile(const std::string& filepath, const std::shared_ptr<SceneNode>& rootNode) {
    if (!rootNode) return false;

    m_baseDir = GetBaseDir(filepath);

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ok = false;
    // 判断后缀选择 ASCII/GLB
    auto ends_with = [](const std::string& s, const char* suf){
        size_t n = std::strlen(suf);
        if (s.size() < n) return false;
        for (size_t i=0;i<n;++i) {
            char a = std::tolower(s[s.size()-n+i]);
            char b = std::tolower(suf[i]);
            if (a!=b) return false;
        }
        return true;
    };

    if (ends_with(filepath, ".glb")) {
        ok = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
    } else {
        ok = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
    }

    if (!warn.empty()) std::cerr << "[glTF warn] " << warn << "\n";
    if (!ok) {
        std::cerr << "[glTF error] " << err << "\n";
        return false;
    }

    // materials
    BuildMaterials(model);

    // scene 根：如果 glTF 指定 default scene，用里面的 nodes；否则全量 nodes 当根
    if (model.scenes.empty()) {
        // 没有 scene，遍历所有根节点（一般都会有 scene，这里兜底）
        for (int i = 0; i < (int)model.nodes.size(); ++i) {
            // 只挂没有被其他节点引用过的节点（粗略判断：被引用的一般出现在 scenes[].nodes）
            // 为简单起见，这里全部挂上
            BuildNodeRecursive(model, i, rootNode);
        }
    } else {
        int sceneIndex = model.defaultScene >= 0 ? model.defaultScene : 0;
        const auto& sc = model.scenes[sceneIndex];
        for (int ni : sc.nodes) {
            BuildNodeRecursive(model, ni, rootNode);
        }
    }

    return true;
}

// ---------- materials ----------
void GLTFLoader::BuildMaterials(const tinygltf::Model& model) {
    m_materials.clear();
    m_materials.reserve(model.materials.size());

    for (const auto& m : model.materials) {
        auto mat = std::make_shared<PBRMaterial>();

        // alphaMode / alphaCutoff / doubleSided
        const std::string& am = m.alphaMode.empty() ? "OPAQUE" : m.alphaMode;
        if (am == "BLEND")      mat->SetAlphaMode(PBRMaterial::AlphaMode::Blend);
        else if (am == "MASK")  mat->SetAlphaMode(PBRMaterial::AlphaMode::Mask);
        else                    mat->SetAlphaMode(PBRMaterial::AlphaMode::Opaque);

        mat->SetAlphaCutoff((float)m.alphaCutoff); // 默认为 0.5
        mat->SetDoubleSided(m.doubleSided);

        // pbrMetallicRoughness
        const auto& pbr = m.pbrMetallicRoughness;

        // baseColorFactor
        if (pbr.baseColorFactor.size() == 4) {
            glm::vec3 c((float)pbr.baseColorFactor[0],
                        (float)pbr.baseColorFactor[1],
                        (float)pbr.baseColorFactor[2]);
            mat->SetBaseColor(c);
            mat->SetBaseAlpha((float)pbr.baseColorFactor[3]);
        }

        // metallic / roughness
        mat->SetMetalness((float)pbr.metallicFactor);
        mat->SetRoughness((float)pbr.roughnessFactor);

        // emissive
        if (m.emissiveFactor.size() == 3) {
            glm::vec3 e((float)m.emissiveFactor[0],
                        (float)m.emissiveFactor[1],
                        (float)m.emissiveFactor[2]);
            mat->SetEmissive(e);
        }

        // ----- textures (仅 .gltf 外链；.glb 内嵌返回空路径则跳过) -----
        // baseColorTexture → albedo
        if (pbr.baseColorTexture.index >= 0) {
            auto path = ResolveTexturePath(model, pbr.baseColorTexture.index);
            if (!path.empty()) mat->LoadAlbedoMap(path);
        }

        // metallicRoughnessTexture → 你的 “RM map”
        if (pbr.metallicRoughnessTexture.index >= 0) {
            auto path = ResolveTexturePath(model, pbr.metallicRoughnessTexture.index);
            if (!path.empty()) mat->LoadRoughnessMetalMap(path);
        }

        // normalTexture
        if (m.normalTexture.index >= 0) {
            auto path = ResolveTexturePath(model, m.normalTexture.index);
            if (!path.empty()) mat->LoadNormalMap(path);
        }

        // occlusionTexture → AO
        if (m.occlusionTexture.index >= 0) {
            auto path = ResolveTexturePath(model, m.occlusionTexture.index);
            if (!path.empty()) mat->LoadAoMap(path);
        }

        // emissiveTexture
        if (m.emissiveTexture.index >= 0) {
            auto path = ResolveTexturePath(model, m.emissiveTexture.index);
            if (!path.empty()) mat->LoadEmissiveMap(path);
        }

        m_materials.push_back(mat);
    }
}

// ---------- nodes ----------
void GLTFLoader::BuildNodeRecursive(const tinygltf::Model& model, int nodeIndex,
                                    const std::shared_ptr<SceneNode>& parent)
{
    if (nodeIndex < 0 || nodeIndex >= (int)model.nodes.size()) return;
    const auto& n = model.nodes[nodeIndex];

    // 这个 SceneNode 仅承载变换（mesh 在其子节点里按 primitive 拆开）
    auto holder = std::make_shared<SceneNode>(nullptr, nullptr);
    ApplyNodeTransform(n, holder);
    parent->AddChild(holder);

    // 若该 glTF 节点有 mesh，则为每个 primitive 创建一个带 Mesh+Material 的子节点
    if (n.mesh >= 0 && n.mesh < (int)model.meshes.size()) {
        const auto& m = model.meshes[n.mesh];
        for (const auto& prim : m.primitives) {
            int matIndex = prim.material; // 可能是 -1
            auto primNode = BuildPrimitiveNode(model, prim, matIndex);
            holder->AddChild(primNode);
        }
    }

    // 递归 children
    for (int child : n.children) {
        BuildNodeRecursive(model, child, holder);
    }
}

// ---------- primitive→mesh ----------
std::shared_ptr<SceneNode> GLTFLoader::BuildPrimitiveNode(const tinygltf::Model& model,
                                                          const tinygltf::Primitive& prim, int materialIndex)
{
    // 读取 attributes
    std::vector<glm::vec3> pos, nrm;
    std::vector<glm::vec2> uv;
    std::vector<unsigned int> idx;

    // POSITION（必需）
    auto itP = prim.attributes.find("POSITION");
    if (itP == prim.attributes.end()) {
        // 缺少 POSITION，给个空 mesh，避免崩
        auto node = std::make_shared<SceneNode>(nullptr, nullptr);
        return node;
    }
    ReadVec3Attrib(model, itP->second, pos);

    // NORMAL（可选）
    auto itN = prim.attributes.find("NORMAL");
    if (itN != prim.attributes.end()) {
        ReadVec3Attrib(model, itN->second, nrm);
    } else {
        nrm.assign(pos.size(), glm::vec3(0.0f));
    }

    // TEXCOORD_0（可选）
    auto itUV = prim.attributes.find("TEXCOORD_0");
    if (itUV != prim.attributes.end()) {
        ReadVec2Attrib(model, itUV->second, uv);
    } else {
        uv.assign(pos.size(), glm::vec2(0.0f));
    }

    // indices（可选；若无则顺序三角化的 glTF 很少见，这里仍然兜底）
    if (prim.indices >= 0) {
        ReadIndices(model, prim.indices, idx);
    } else {
        // 无索引，顺序三角（按 0,1,2 / 3,4,5 ...）
        idx.resize(pos.size());
        for (size_t i = 0; i < pos.size(); ++i) idx[i] = (unsigned int)i;
    }

    // 组装你的 Vertex
    std::vector<Vertex> verts;
    verts.resize(pos.size());
    for (size_t i = 0; i < pos.size(); ++i) {
        verts[i].position = pos[i];
        verts[i].normals  = (i < nrm.size() ? nrm[i] : glm::vec3(0.0f));
        verts[i].texCoord = (i < uv.size()  ? uv[i]  : glm::vec2(0.0f));
    }

    // 创建 Mesh 并一次性 LoadFromModel（这是你 Mesh 唯一对外 API）
    auto mesh = std::make_shared<Mesh>();
    mesh->LoadFromModel(std::move(verts), std::move(idx));

    // 材质
    std::shared_ptr<PBRMaterial> mat = nullptr;
    if (materialIndex >= 0 && materialIndex < (int)m_materials.size())
        mat = m_materials[materialIndex];
    if (!mat) mat = std::make_shared<PBRMaterial>(); // 默认材质

    // 生成带 Mesh+Material 的 SceneNode
    auto node = std::make_shared<SceneNode>(mesh, mat);
    return node;
}

// ---------- attribute readers ----------
bool GLTFLoader::ReadVec3Attrib(const tinygltf::Model& model, int accessorIndex,
                                std::vector<glm::vec3>& out) const
{
    if (accessorIndex < 0 || accessorIndex >= (int)model.accessors.size()) return false;
    const auto& acc = model.accessors[accessorIndex];
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf  = model.buffers[view.buffer];

    // 仅支持 float vec3（常见情况）
    if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || acc.type != TINYGLTF_TYPE_VEC3)
        return false;

    size_t stride = acc.ByteStride(view);
    if (stride == 0) stride = sizeof(float) * 3;

    const unsigned char* data = buf.data.data() + view.byteOffset + acc.byteOffset;
    out.resize(acc.count);
    for (size_t i = 0; i < acc.count; ++i) {
        const float* f = reinterpret_cast<const float*>(data + i * stride);
        out[i] = glm::vec3(f[0], f[1], f[2]);
    }
    return true;
}

bool GLTFLoader::ReadVec2Attrib(const tinygltf::Model& model, int accessorIndex,
                                std::vector<glm::vec2>& out) const
{
    if (accessorIndex < 0 || accessorIndex >= (int)model.accessors.size()) return false;
    const auto& acc = model.accessors[accessorIndex];
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf  = model.buffers[view.buffer];

    if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || acc.type != TINYGLTF_TYPE_VEC2)
        return false;

    size_t stride = acc.ByteStride(view);
    if (stride == 0) stride = sizeof(float) * 2;

    const unsigned char* data = buf.data.data() + view.byteOffset + acc.byteOffset;
    out.resize(acc.count);
    for (size_t i = 0; i < acc.count; ++i) {
        const float* f = reinterpret_cast<const float*>(data + i * stride);
        out[i] = glm::vec2(f[0], f[1]);
    }
    return true;
}

bool GLTFLoader::ReadIndices(const tinygltf::Model& model, int accessorIndex,
                             std::vector<unsigned int>& out) const
{
    if (accessorIndex < 0 || accessorIndex >= (int)model.accessors.size()) return false;
    const auto& acc = model.accessors[accessorIndex];
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf  = model.buffers[view.buffer];

    const unsigned char* data = buf.data.data() + view.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(view);
    if (stride == 0) {
        // 根据 componentType 默认字节数
        if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)   stride = 4;
        else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) stride = 2;
        else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)  stride = 1;
        else return false;
    }

    out.resize(acc.count);
    switch (acc.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            for (size_t i = 0; i < acc.count; ++i) {
                out[i] = *(reinterpret_cast<const uint32_t*>(data + i * stride));
            }
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            for (size_t i = 0; i < acc.count; ++i) {
                out[i] = *(reinterpret_cast<const uint16_t*>(data + i * stride));
            }
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            for (size_t i = 0; i < acc.count; ++i) {
                out[i] = *(reinterpret_cast<const uint8_t*>(data + i * stride));
            }
        } break;
        default: return false;
    }
    return true;
}

// ---------- transform ----------
void GLTFLoader::ApplyNodeTransform(const tinygltf::Node& n, const std::shared_ptr<SceneNode>& node) const {
    // 优先使用 matrix
    if (n.matrix.size() == 16) {
        glm::mat4 M(1.0f);
        // glTF 是列主序；tinygltf 提供 double 数组，直接按列填
        M[0][0] = (float)n.matrix[0];  M[1][0] = (float)n.matrix[1];  M[2][0] = (float)n.matrix[2];  M[3][0] = (float)n.matrix[3];
        M[0][1] = (float)n.matrix[4];  M[1][1] = (float)n.matrix[5];  M[2][1] = (float)n.matrix[6];  M[3][1] = (float)n.matrix[7];
        M[0][2] = (float)n.matrix[8];  M[1][2] = (float)n.matrix[9];  M[2][2] = (float)n.matrix[10]; M[3][2] = (float)n.matrix[11];
        M[0][3] = (float)n.matrix[12]; M[1][3] = (float)n.matrix[13]; M[2][3] = (float)n.matrix[14]; M[3][3] = (float)n.matrix[15];
        node->SetLocalTransformMatrix(M);
        return;
    }

    glm::vec3 T(0.0f), S(1.0f);
    glm::quat R(1,0,0,0);

    if (n.translation.size() == 3) {
        T = glm::vec3((float)n.translation[0], (float)n.translation[1], (float)n.translation[2]);
    }
    if (n.scale.size() == 3) {
        S = glm::vec3((float)n.scale[0], (float)n.scale[1], (float)n.scale[2]);
    }
    if (n.rotation.size() == 4) {
        // glTF rotation 是 (x,y,z,w)
        R = glm::quat((float)n.rotation[3], (float)n.rotation[0], (float)n.rotation[1], (float)n.rotation[2]);
    }

    glm::mat4 M(1.0f);
    M = glm::translate(M, T) * glm::mat4_cast(R) * glm::scale(glm::mat4(1.0f), S);
    node->SetLocalTransformMatrix(M);
}
