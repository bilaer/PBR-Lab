// gltf_loader.cpp
#include "model_loader/gltf_loader.h"   
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cctype>
#include <stb_image_write.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // translate / scale
#include <glm/gtc/quaternion.hpp>        // glm::quat, glm::mat4_cast
#include <glm/gtc/type_ptr.hpp>          // glm::make_mat4

// ---------------- small utils ----------------
static std::string to_lower(std::string s){
    for (auto &c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

static bool file_exists(const std::string& p){
    std::ifstream f(p, std::ios::binary);
    return (bool)f;
}

static bool is_glb_magic(const std::string& p){
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    char magic[4] = {};
    f.read(magic, 4);
    // GLB magic = "glTF"
    return f.gcount()==4 && magic[0]=='g' && magic[1]=='l' && magic[2]=='T' && magic[3]=='F';
}

std::string GlbLoader::DirOf(const std::string& p) {
    auto pos = p.find_last_of("/\\");
    return (pos == std::string::npos) ? std::string(".") : p.substr(0, pos);
}

static glm::mat4 MakeTRS(const glm::vec3& T, const glm::quat& R, const glm::vec3& S) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, T);     // T
    M *= glm::mat4_cast(R);       // R (quat → mat4)
    M = glm::scale(M, S);         // S
    return M;                     // M = T * R * S
}

// Access a single element pointer with proper offsets/stride
const unsigned char* GlbLoader::AccessorElemPtr(const tinygltf::Model& model,
                                                const tinygltf::Accessor& acc,
                                                size_t elemIndex) {
    const auto& bv  = model.bufferViews[acc.bufferView];
    const auto& buf = model.buffers[bv.buffer];

    size_t compSize  = tinygltf::GetComponentSizeInBytes(acc.componentType);
    size_t numComps  = tinygltf::GetNumComponentsInType(acc.type);
    size_t defStride = compSize * numComps;
    size_t stride    = bv.byteStride ? bv.byteStride : defStride;

    size_t offset = (size_t)bv.byteOffset + (size_t)acc.byteOffset + elemIndex * stride;
    return &buf.data[offset];
}

// Save embedded (in-memory) image to a temp .png and return its path
std::string GlbLoader::ResolveImagePathOrTemp(const std::string& baseDir,
                                              const tinygltf::Image& img,
                                              const std::string& hintFileName) {
    // 1) URI case (.gltf external or .glb kept uri)
    if (!img.uri.empty()) {
        std::filesystem::path p1 = std::filesystem::path(baseDir) / img.uri;
        if (std::filesystem::exists(p1)) return p1.string();

        // common layout fallback: baseDir/textures/xxx.png
        std::filesystem::path p2 = std::filesystem::path(baseDir) / "textures" / img.uri;
        if (std::filesystem::exists(p2)) return p2.string();
        // fallthrough to embedded
    }

    // 2) Embedded case (.glb, uri empty but pixels present)
    if (!img.image.empty()) {
        if (img.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            std::cerr << "[GlbLoader] Unsupported pixel type for temp write: " << img.pixel_type << "\n";
            return {};
        }
        int w = img.width, h = img.height, comp = img.component;
        if (w <= 0 || h <= 0 || (comp != 3 && comp != 4)) {
            std::cerr << "[GlbLoader] Bad embedded image size/channels\n";
            return {};
        }

        // temp file
        std::filesystem::path out = std::filesystem::temp_directory_path()
            / (std::string("glb_embed_") + hintFileName + (comp==4?".rgba.png":".rgb.png"));

        int stride = comp * w;
        int ok = stbi_write_png(out.string().c_str(), w, h, comp, img.image.data(), stride);
        if (!ok) {
            std::cerr << "[GlbLoader] stbi_write_png failed\n";
            return {};
        }
        return out.string();
    }

    // 3) no uri, no pixels
    return {};
}

// Derive bitangent from normal, tangent.xyz and a default w
void GlbLoader::FillBitangentsFromTangentW(std::vector<Vertex>& vertices, float defaultW) {
    for (auto& v : vertices) {
        float Tw = defaultW; // we don't store tangent.w, assume +1
        if (glm::length(v.tangent) > 0.0f && glm::length(v.normals) > 0.0f) {
            v.bitangent = glm::normalize(glm::cross(v.normals, v.tangent) * Tw);
        }
    }
}

// ---------------- robust LoadFile ----------------
bool GlbLoader::LoadFile(const std::string& path, const std::shared_ptr<Scene>& scene) {
    if (!file_exists(path)) {
        std::cerr << "File not found: " << path << "\n";
        return false;
    }

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // guess by suffix + verify by magic
    std::string lower = to_lower(path);
    bool expect_glb = lower.size() >= 4 && lower.substr(lower.size()-4) == ".glb";
    if (is_glb_magic(path)) expect_glb = true;

    auto try_ascii = [&](){
        err.clear(); warn.clear();
        bool ok = loader.LoadASCIIFromFile(&model, &err, &warn, path);
        if (!warn.empty()) std::cerr << "[tinygltf][warn] " << warn << "\n";
        return ok;
    };
    auto try_binary = [&](){
        err.clear(); warn.clear();
        bool ok = loader.LoadBinaryFromFile(&model, &err, &warn, path);
        if (!warn.empty()) std::cerr << "[tinygltf][warn] " << warn << "\n";
        return ok;
    };

    bool ok = false;
    if (expect_glb) {
        ok = try_binary();
        if (!ok) ok = try_ascii();
    } else {
        ok = try_ascii();
        if (!ok) ok = try_binary();
    }

    if (!ok) {
        std::cerr << "Failed to load glTF: " << err << "\n";
        std::ifstream f(path, std::ios::binary);
        char head[8] = {};
        f.read(head, 8);
        std::cerr << "First bytes: ";
        for (int i=0;i<f.gcount();++i) std::cerr << std::hex << (0xff & (unsigned)head[i]) << " ";
        std::cerr << std::dec << "\n";
        return false;
    }

    int sceneIndex = model.defaultScene >= 0 ? model.defaultScene : 0;
    if (sceneIndex < 0 || sceneIndex >= (int)model.scenes.size()) {
        for (int i=0; i<(int)model.nodes.size(); ++i) {
            auto node = BuildNodeRecursive(model, i, path);
            if (node) scene->AddNode(node);
        }
        return true;
    }

    const auto& s = model.scenes[sceneIndex];
    for (int root : s.nodes) {
        auto node = BuildNodeRecursive(model, root, path);
        if (node) scene->AddNode(node);
    }
    return true;
}

// ---------------- scene graph ----------------
std::shared_ptr<SceneNode> GlbLoader::BuildNodeRecursive(const tinygltf::Model& model,
                                                         int nodeIndex,
                                                         const std::string& gltfPath) {
    const auto& n = model.nodes[nodeIndex];

    std::shared_ptr<SceneNode> parentNode;

    auto applyNodeTransform = [&](const std::shared_ptr<SceneNode>& node){
        // glTF: if matrix exists, ignore T/R/S
        if (n.matrix.size() == 16) {
            float m[16];
            for (int i = 0; i < 16; ++i) m[i] = static_cast<float>(n.matrix[i]);
            glm::mat4 M = glm::make_mat4(m);  // column-major (glTF & GLM both)
            node->SetLocalTransformMatrix(M);
        } else {
            glm::vec3 T(0.0f), S(1.0f);
            glm::quat R(1,0,0,0);
            if (n.translation.size()==3) T = {(float)n.translation[0], (float)n.translation[1], (float)n.translation[2]};
            if (n.scale.size()==3)       S = {(float)n.scale[0],       (float)n.scale[1],       (float)n.scale[2]};
            if (n.rotation.size()==4)    R = glm::quat((float)n.rotation[3], (float)n.rotation[0],
                                                       (float)n.rotation[1], (float)n.rotation[2]);
            glm::mat4 M = MakeTRS(T, R, S);
            node->SetLocalTransformMatrix(M);
        }
    };

    if (n.mesh >= 0 && n.mesh < (int)model.meshes.size()) {
        const auto& gltfMesh = model.meshes[n.mesh];

        if (gltfMesh.primitives.size() > 1) {
            // parent carries transform; each primitive becomes a child
            parentNode = std::make_shared<SceneNode>(nullptr, nullptr);
            applyNodeTransform(parentNode);
        }

        for (const auto& prim : gltfMesh.primitives) {
            auto mesh = LoadMesh(model, prim);
            auto mat  = LoadMaterial(model, prim.material, gltfPath);
            auto node = std::make_shared<SceneNode>(mesh, mat);

            if (!parentNode) {
                // single primitive: apply transform directly on this node
                applyNodeTransform(node);
                parentNode = node;
            } else {
                // multi-primitive: child stays identity; parent has TRS
                node->SetLocalTransformMatrix(glm::mat4(1.0f));
                parentNode->AddChild(node);
            }
        }
    } else {
        // pure transform node
        parentNode = std::make_shared<SceneNode>(nullptr, nullptr);
        applyNodeTransform(parentNode);
    }

    // recurse children
    for (int ci : n.children) {
        auto child = BuildNodeRecursive(model, ci, gltfPath);
        if (child) parentNode->AddChild(child);
    }

    return parentNode;
}

// ---------------- geometry ----------------
std::shared_ptr<Mesh> GlbLoader::LoadMesh(const tinygltf::Model& model,
                                          const tinygltf::Primitive& primitive) {
    auto mesh = std::make_shared<Mesh>();

    // POSITION (required)
    if (!primitive.attributes.count("POSITION")) {
        std::cerr << "[GlbLoader] Primitive missing POSITION\n";
        return mesh;
    }
    const auto& posAcc = model.accessors[primitive.attributes.at("POSITION")];

    std::vector<Vertex> vertices(posAcc.count);
    for (size_t i=0; i<posAcc.count; ++i) {
        const float* p = reinterpret_cast<const float*>(AccessorElemPtr(model, posAcc, i));
        vertices[i].position = glm::vec3(p[0], p[1], p[2]);
    }

    // NORMAL (optional)
    if (primitive.attributes.count("NORMAL")) {
        const auto& nAcc = model.accessors[primitive.attributes.at("NORMAL")];
        for (size_t i=0; i<nAcc.count && i<vertices.size(); ++i) {
            const float* n = reinterpret_cast<const float*>(AccessorElemPtr(model, nAcc, i));
            vertices[i].normals = glm::vec3(n[0], n[1], n[2]);
        }
    } else {
        for (auto& v : vertices) v.normals = glm::vec3(0.0f);
    }

    // TEXCOORD_0 (optional)
    if (primitive.attributes.count("TEXCOORD_0")) {
        const auto& tAcc = model.accessors[primitive.attributes.at("TEXCOORD_0")];
        for (size_t i=0; i<tAcc.count && i<vertices.size(); ++i) {
            const float* uv = reinterpret_cast<const float*>(AccessorElemPtr(model, tAcc, i));
            vertices[i].texCoord = glm::vec2(uv[0], uv[1]);
        }
    } else {
        for (auto& v : vertices) v.texCoord = glm::vec2(0.0f);
    }

    // TANGENT (vec4) → tangent.xyz + bitangent from w (assume +1)
    bool hasTangent = false;
    if (primitive.attributes.count("TANGENT")) {
        const auto& tanAcc = model.accessors[primitive.attributes.at("TANGENT")];
        for (size_t i=0; i<tanAcc.count && i<vertices.size(); ++i) {
            const float* t = reinterpret_cast<const float*>(AccessorElemPtr(model, tanAcc, i));
            vertices[i].tangent = glm::vec3(t[0], t[1], t[2]);
        }
        hasTangent = true;
    } else {
        for (auto& v : vertices) v.tangent = glm::vec3(0.0f);
    }

    if (hasTangent) {
        FillBitangentsFromTangentW(vertices, /*defaultW=*/1.0f);
    } else {
        for (auto& v : vertices) v.bitangent = glm::vec3(0.0f);
    }

    mesh->SetVertices(std::move(vertices));

    // Indices
    std::vector<unsigned int> indices;
    if (primitive.indices >= 0) {
        const auto& idxAcc = model.accessors[primitive.indices];
        const auto& idxBV  = model.bufferViews[idxAcc.bufferView];
        const auto& idxBuf = model.buffers[idxBV.buffer];
        const unsigned char* base = &idxBuf.data[idxBV.byteOffset + idxAcc.byteOffset];

        indices.resize(idxAcc.count);
        switch (idxAcc.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                auto s = reinterpret_cast<const uint8_t*>(base);
                for (size_t i=0;i<idxAcc.count;++i) indices[i] = s[i];
            } break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                auto s = reinterpret_cast<const uint16_t*>(base);
                for (size_t i=0;i<idxAcc.count;++i) indices[i] = s[i];
            } break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                auto s = reinterpret_cast<const uint32_t*>(base);
                for (size_t i=0;i<idxAcc.count;++i) indices[i] = s[i];
            } break;
            default:
                std::cerr << "[GlbLoader] Unsupported index component type\n";
                indices.clear();
                break;
        }
    }
    mesh->SetIndices(std::move(indices));
    mesh->SetupBuffers(); // init VAO/VBO/EBO

    return mesh;
}

// ---------------- materials ----------------
std::shared_ptr<PBRMaterial> GlbLoader::LoadMaterial(const tinygltf::Model& model,
                                                     int materialIndex,
                                                     const std::string& gltfPath) {
    auto mat = std::make_shared<PBRMaterial>();
    if (materialIndex < 0 || materialIndex >= (int)model.materials.size()) return mat;

    const auto& m   = model.materials[materialIndex];
    const auto& pmr = m.pbrMetallicRoughness;
    const std::string baseDir = DirOf(gltfPath);

    if (pmr.baseColorFactor.size()==4) {
        mat->SetBaseColor(glm::vec3((float)pmr.baseColorFactor[0],
                                    (float)pmr.baseColorFactor[1],
                                    (float)pmr.baseColorFactor[2]));
    }
    mat->SetRoughness((float)pmr.roughnessFactor);
    mat->SetMetalness((float)pmr.metallicFactor);

    auto loadTexPath = [&](int texIndex, void (PBRMaterial::*fn)(const std::string&), const char* hint){
        if (texIndex < 0) return;
        const auto& tex = model.textures[texIndex];
        if (tex.source < 0 || tex.source >= (int)model.images.size()) return;
        const auto& img = model.images[tex.source];
        std::string path = ResolveImagePathOrTemp(baseDir, img, hint ? hint : "tex");
        if (path.empty()) {
            std::cerr << "❌ Failed to resolve texture for " << (hint?hint:"tex") << "\n";
            return;
        }
        (mat.get()->*fn)(path);
    };

    // Albedo (sRGB in your Texture2D)
    loadTexPath(pmr.baseColorTexture.index, &PBRMaterial::LoadAlbedoMap, "baseColor");

    // Normal (linear)
    loadTexPath(m.normalTexture.index,      &PBRMaterial::LoadNormalMap, "normal");

    // glTF metallicRoughness packed texture (G=roughness, B=metallic)
    loadTexPath(pmr.metallicRoughnessTexture.index, &PBRMaterial::LoadRoughnessMetalMap, "metalRough");

    // AO (optional, linear)
    loadTexPath(m.occlusionTexture.index,   &PBRMaterial::LoadAoMap, "ao");

    // Emissive (factor + texture）
    if (m.emissiveFactor.size() == 3) {
        mat->SetEmissive(glm::vec3(
            (float)m.emissiveFactor[0],
            (float)m.emissiveFactor[1],
            (float)m.emissiveFactor[2]
        ));
    }
    loadTexPath(m.emissiveTexture.index,    &PBRMaterial::LoadEmissiveMap, "emissive");

    return mat;
}
