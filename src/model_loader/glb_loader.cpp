#include "model_loader/glb_loader.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

// If your Texture2D already flips V on load, keep this 0 to avoid double flip
#ifndef GLTF_FLIP_TEXCOORD_V
#define GLTF_FLIP_TEXCOORD_V 0
#endif

// ---------- small utils ----------
static std::string to_lower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}
static bool file_exists(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return (bool)f;
}
static bool is_glb_magic(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    char magic[4] = {};
    f.read(magic, 4);
    return f.gcount()==4 && magic[0]=='g' && magic[1]=='l' && magic[2]=='T' && magic[3]=='F';
}
std::string GlbLoader::DirOf(const std::string& p) {
    auto pos = p.find_last_of("/\\");
    return (pos == std::string::npos) ? std::string(".") : p.substr(0, pos);
}
static glm::mat4 MakeTRS(const glm::vec3& T, const glm::quat& R, const glm::vec3& S) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, T);
    M *= glm::mat4_cast(R);
    M = glm::scale(M, S);
    return M;
}

// ---------- accessor element pointer ----------
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

// ---------- bitangent from normal x tangent * w ----------
void GlbLoader::FillBitangentsFromTangentW(std::vector<Vertex>& vertices, float defaultW) {
    for (auto& v : vertices) {
        const float n2 = glm::dot(v.normals,  v.normals);
        const float t2 = glm::dot(v.tangent,  v.tangent);
        const float w  = defaultW;
        if (n2 > 0.0f && t2 > 0.0f) {
            v.bitangent = glm::normalize(glm::cross(v.normals, v.tangent) * w);
        } else {
            v.bitangent = glm::vec3(0.0f);
        }
    }
}

// ---------- LoadFile ----------
bool GlbLoader::LoadFile(const std::string& path, const std::shared_ptr<SceneNode>& parent) {
    if (!parent) {
        std::cerr << "[GlbLoader] parent is null\n";
        return false;
    }
    if (!file_exists(path)) {
        std::cerr << "[GlbLoader] file not found: " << path << "\n";
        return false;
    }

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // Decide ASCII/binary
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
    if (expect_glb) { ok = try_binary(); if (!ok) ok = try_ascii(); }
    else            { ok = try_ascii();  if (!ok) ok = try_binary(); }

    if (!ok) {
        std::cerr << "[GlbLoader] failed: " << err << "\n";
        return false;
    }

    parent->UpdateLocalTransform();
    parent->UpdateWorldTransform();

    int sceneIndex = model.defaultScene >= 0 ? model.defaultScene : 0;

    auto attach_roots = [&](const tinygltf::Scene& s){
        for (int root : s.nodes) {
            auto node = BuildNodeRecursive(model, root, path);
            if (node) parent->AddChild(node);
        }
    };

    if (sceneIndex >= 0 && sceneIndex < (int)model.scenes.size()) {
        attach_roots(model.scenes[sceneIndex]);
    } else {
        // No scene: attach all nodes
        tinygltf::Scene fake;
        fake.nodes.resize(model.nodes.size());
        for (int i = 0; i < (int)model.nodes.size(); ++i) fake.nodes[i] = i;
        attach_roots(fake);
    }
    return true;
}

// ---------- BuildNodeRecursive ----------
std::shared_ptr<SceneNode> GlbLoader::BuildNodeRecursive(const tinygltf::Model& model,
                                                         int nodeIndex,
                                                         const std::string& gltfPath) {
    const auto& n = model.nodes[nodeIndex];

    auto applyNodeTransform = [&](const std::shared_ptr<SceneNode>& node){
        if (n.matrix.size() == 16) {
            float m[16];
            for (int i = 0; i < 16; ++i) m[i] = (float)n.matrix[i];
            node->SetLocalTransformMatrix(glm::make_mat4(m)); // column-major
        } else {
            glm::vec3 T(0.0f), S(1.0f);
            glm::quat R(1,0,0,0);
            if (n.translation.size()==3) T = {(float)n.translation[0], (float)n.translation[1], (float)n.translation[2]};
            if (n.scale.size()==3)       S = {(float)n.scale[0],       (float)n.scale[1],       (float)n.scale[2]};
            if (n.rotation.size()==4)    R = glm::quat((float)n.rotation[3], (float)n.rotation[0],
                                                       (float)n.rotation[1], (float)n.rotation[2]); // (w,x,y,z)
            node->SetLocalTransformMatrix(MakeTRS(T, R, S));
        }
    };

    std::shared_ptr<SceneNode> parentNode;

    if (n.mesh >= 0 && n.mesh < (int)model.meshes.size()) {
        const auto& gltfMesh = model.meshes[n.mesh];

        // If this glTF mesh has multiple primitives, we create a transform parent and attach children.
        if (gltfMesh.primitives.size() > 1) {
            parentNode = std::make_shared<SceneNode>(nullptr, nullptr);
            applyNodeTransform(parentNode);
        }

        for (const auto& prim : gltfMesh.primitives) {
            if (prim.mode != TINYGLTF_MODE_TRIANGLES &&
                prim.mode != TINYGLTF_MODE_TRIANGLE_STRIP &&
                prim.mode != TINYGLTF_MODE_TRIANGLE_FAN) {
                std::cerr << "[GlbLoader] skip non-triangle primitive mode=" << prim.mode << "\n";
                continue;
            }

            auto mesh = LoadMesh(model, prim);
            auto mat  = LoadMaterial(model, prim.material, gltfPath);
            auto node = std::make_shared<SceneNode>(mesh, mat);

            // IMPORTANT: material toggles whether to use vertex tangents.
            if (mat) {
                bool hasTangent = prim.attributes.count("TANGENT") > 0;
                mat->SetUseVertexTangent(hasTangent);
            }

            if (!parentNode) {
                applyNodeTransform(node);
                parentNode = node;
            } else {
                node->SetLocalTransformMatrix(glm::mat4(1.0f));
                parentNode->AddChild(node);
            }
        }
    } else {
        // Pure transform node (no mesh)
        parentNode = std::make_shared<SceneNode>(nullptr, nullptr);
        applyNodeTransform(parentNode);
    }

    // Recurse children
    for (int ci : n.children) {
        auto child = BuildNodeRecursive(model, ci, gltfPath);
        if (child) parentNode->AddChild(child);
    }
    return parentNode;
}

// ---------- LoadMesh ----------
std::shared_ptr<Mesh> GlbLoader::LoadMesh(const tinygltf::Model& model,
                                          const tinygltf::Primitive& primitive) {
    auto mesh = std::make_shared<Mesh>();

    // POSITION (required, VEC3 float)
    if (!primitive.attributes.count("POSITION")) {
        std::cerr << "[GlbLoader] primitive missing POSITION\n";
        return mesh;
    }
    const auto& posAcc = model.accessors.at(primitive.attributes.at("POSITION"));
    if (posAcc.type != TINYGLTF_TYPE_VEC3 || posAcc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        std::cerr << "[GlbLoader] POSITION must be VEC3 float\n";
        return mesh;
    }

    // Prepare vertices
    std::vector<Vertex> vertices(posAcc.count);
    for (size_t i=0; i<posAcc.count; ++i) {
        const float* p = reinterpret_cast<const float*>(AccessorElemPtr(model, posAcc, i));
        vertices[i].position = glm::vec3(p[0], p[1], p[2]);
    }

    // NORMAL (optional)
    bool hasNormal = false;
    if (primitive.attributes.count("NORMAL")) {
        const auto& nAcc = model.accessors.at(primitive.attributes.at("NORMAL"));
        if (nAcc.type == TINYGLTF_TYPE_VEC3 && nAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            hasNormal = true;
            for (size_t i=0; i<nAcc.count && i<vertices.size(); ++i) {
                const float* n = reinterpret_cast<const float*>(AccessorElemPtr(model, nAcc, i));
                vertices[i].normals = glm::vec3(n[0], n[1], n[2]);
            }
        }
    }
    if (!hasNormal) { for (auto& v : vertices) v.normals = glm::vec3(0.0f); }

    // TEXCOORD_0 (optional; float/normalized ubyte/normalized ushort)
    if (primitive.attributes.count("TEXCOORD_0")) {
        const auto& tAcc = model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
        auto readUV = [&](size_t i)->glm::vec2 {
            switch (tAcc.componentType) {
                case TINYGLTF_COMPONENT_TYPE_FLOAT: {
                    const float* uv = reinterpret_cast<const float*>(AccessorElemPtr(model, tAcc, i));
                    return glm::vec2(uv[0], uv[1]);
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* uv = reinterpret_cast<const uint8_t*>(AccessorElemPtr(model, tAcc, i));
                    return glm::vec2(uv[0] / 255.0f, uv[1] / 255.0f);
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* uv = reinterpret_cast<const uint16_t*>(AccessorElemPtr(model, tAcc, i));
                    return glm::vec2(uv[0] / 65535.0f, uv[1] / 65535.0f);
                }
                default:
                    std::cerr << "[GlbLoader] TEXCOORD_0 unsupported component type: " << tAcc.componentType << "\n";
                    return glm::vec2(0.0f);
            }
        };
        for (size_t i=0; i<tAcc.count && i<vertices.size(); ++i) {
            glm::vec2 uv = readUV(i);
#if GLTF_FLIP_TEXCOORD_V
            uv.y = 1.0f - uv.y;
#endif
            vertices[i].texCoord = uv;
        }
    } else {
        for (auto& v : vertices) v.texCoord = glm::vec2(0.0f);
    }

    // TANGENT (optional, vec4 float) → tangent.xyz, bitangent = normalize(cross(N, T) * w)
    bool hasTangent = false;
    if (primitive.attributes.count("TANGENT")) {
        const auto& tanAcc = model.accessors.at(primitive.attributes.at("TANGENT"));
        if (tanAcc.type == TINYGLTF_TYPE_VEC4 && tanAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            hasTangent = true;
            for (size_t i=0; i<tanAcc.count && i<vertices.size(); ++i) {
                const float* t = reinterpret_cast<const float*>(AccessorElemPtr(model, tanAcc, i));
                glm::vec3 T(t[0], t[1], t[2]);
                float w = t[3];
                vertices[i].tangent = T;
                if (glm::dot(vertices[i].normals, vertices[i].normals) > 0.0f) {
                    vertices[i].bitangent = glm::normalize(glm::cross(vertices[i].normals, T) * w);
                } else {
                    vertices[i].bitangent = glm::vec3(0.0f);
                }
            }
        }
    }
    if (!hasTangent) {
        for (auto& v : vertices) { v.tangent = glm::vec3(0.0f); v.bitangent = glm::vec3(0.0f); }
    }

    // Indices (convert strip/fan to triangles if needed)
    std::vector<unsigned int> indices;
    if (primitive.indices >= 0) {
        const auto& idxAcc = model.accessors[primitive.indices];
        const auto& idxBV  = model.bufferViews[idxAcc.bufferView];
        const auto& idxBuf = model.buffers[idxBV.buffer];

        const unsigned char* base = &idxBuf.data[idxBV.byteOffset + idxAcc.byteOffset];
        indices.resize(idxAcc.count);

        switch (idxAcc.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                const uint8_t* s = reinterpret_cast<const uint8_t*>(base);
                for (size_t i=0; i<idxAcc.count; ++i) indices[i] = s[i];
            } break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                const uint16_t* s = reinterpret_cast<const uint16_t*>(base);
                for (size_t i=0; i<idxAcc.count; ++i) indices[i] = s[i];
            } break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                const uint32_t* s = reinterpret_cast<const uint32_t*>(base);
                for (size_t i=0; i<idxAcc.count; ++i) indices[i] = s[i];
            } break;
            default:
                std::cerr << "[GlbLoader] unsupported index component type\n";
                indices.clear();
                break;
        }

        if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP && indices.size() >= 3) {
            std::vector<unsigned int> tri;
            tri.reserve((indices.size() - 2) * 3);
            for (size_t i=0; i+2<indices.size(); ++i) {
                if (i & 1) { tri.push_back(indices[i]); tri.push_back(indices[i+2]); tri.push_back(indices[i+1]); }
                else       { tri.push_back(indices[i]); tri.push_back(indices[i+1]); tri.push_back(indices[i+2]); }
            }
            indices.swap(tri);
        } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN && indices.size() >= 3) {
            std::vector<unsigned int> tri;
            tri.reserve((indices.size() - 2) * 3);
            for (size_t i=1; i+1<indices.size(); ++i) {
                tri.push_back(indices[0]);
                tri.push_back(indices[i]);
                tri.push_back(indices[i+1]);
            }
            indices.swap(tri);
        }
    } else {
        // Non-indexed: expand strip/fan if needed
        if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
            std::vector<unsigned int> tri;
            size_t n = vertices.size();
            tri.reserve((n - 2) * 3);
            for (size_t i=0; i+2<n; ++i) {
                if (i & 1) { tri.push_back((unsigned)i); tri.push_back((unsigned)(i+2)); tri.push_back((unsigned)(i+1)); }
                else       { tri.push_back((unsigned)i); tri.push_back((unsigned)(i+1)); tri.push_back((unsigned)(i+2)); }
            }
            indices.swap(tri);
        } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN) {
            std::vector<unsigned int> tri;
            size_t n = vertices.size();
            tri.reserve((n - 2) * 3);
            for (size_t i=1; i+1<n; ++i) {
                tri.push_back(0u);
                tri.push_back((unsigned)i);
                tri.push_back((unsigned)(i+1));
            }
            indices.swap(tri);
        }
    }

    // Generate simple normals if missing
    if (!hasNormal) {
        auto& vtx = vertices;
        auto addFace = [&](uint32_t i0, uint32_t i1, uint32_t i2) {
            const glm::vec3& p0 = vtx[i0].position;
            const glm::vec3& p1 = vtx[i1].position;
            const glm::vec3& p2 = vtx[i2].position;
            glm::vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            vtx[i0].normals += n; vtx[i1].normals += n; vtx[i2].normals += n;
        };
        if (!indices.empty()) {
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
                addFace(indices[i], indices[i+1], indices[i+2]);
        } else {
            for (size_t i = 0; i + 2 < vtx.size(); i += 3)
                addFace((uint32_t)i, (uint32_t)i+1, (uint32_t)i+2);
        }
        for (auto& v : vtx) {
            if (glm::dot(v.normals, v.normals) > 0.0f) v.normals = glm::normalize(v.normals);
            else v.normals = glm::vec3(0,1,0);
        }
        if (hasTangent) FillBitangentsFromTangentW(vertices, 1.0f);
    }

    // Upload to your Mesh
    mesh->SetVertices(std::move(vertices));
    mesh->SetIndices(std::move(indices));
    mesh->SetupBuffers(); // Make sure you enabled layout 3/4 for tangent/bitangent in Mesh::SetupBuffers.

    return mesh;
}

// ---------- LoadMaterial ----------
std::shared_ptr<PBRMaterial> GlbLoader::LoadMaterial(const tinygltf::Model& model,
                                                     int materialIndex,
                                                     const std::string& /*gltfPath*/) {
    auto mat = std::make_shared<PBRMaterial>();
    if (materialIndex < 0 || materialIndex >= (int)model.materials.size()) return mat;

    const auto& m   = model.materials[materialIndex];
    const auto& pmr = m.pbrMetallicRoughness;

    // ---- Factors ----
    // baseColorFactor (rgb) + alpha multiplier
    float baseAlpha = 1.0f;
    if (pmr.baseColorFactor.size() == 4) {
        mat->SetBaseColor(glm::vec3(
            (float)pmr.baseColorFactor[0],
            (float)pmr.baseColorFactor[1],
            (float)pmr.baseColorFactor[2]
        ));
        baseAlpha = (float)pmr.baseColorFactor[3];
    }
    mat->SetBaseAlpha(baseAlpha);

    mat->SetRoughness((float)pmr.roughnessFactor);
    mat->SetMetalness((float)pmr.metallicFactor);

    if (m.emissiveFactor.size() == 3) {
        mat->SetEmissive(glm::vec3(
            (float)m.emissiveFactor[0],
            (float)m.emissiveFactor[1],
            (float)m.emissiveFactor[2]
        ));
    }

    // ---- Helper: build Texture2D from embedded image ----
    auto makeTex = [&](int texIndex, bool isSRGB) -> std::shared_ptr<Texture2D> {
        if (texIndex < 0) return nullptr;
        const auto& tex = model.textures[texIndex];
        if (tex.source < 0 || tex.source >= (int)model.images.size()) return nullptr;
        const auto& img = model.images[tex.source];
        if (img.image.empty() || img.width <= 0 || img.height <= 0) return nullptr;
        if (img.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            std::cerr << "[GlbLoader] unsupported pixel_type=" << img.pixel_type << " (expect UBYTE)\n";
            return nullptr;
        }
        auto t = std::make_shared<Texture2D>();
        t->CreateFromPixels(img.image.data(), img.width, img.height, img.component, isSRGB);
        return t;
    };

    // ---- Textures (PBR color space conventions) ----
    if (auto t = makeTex(pmr.baseColorTexture.index,           /*sRGB*/ true )) mat->SetAlbedoMap(t);
    if (auto t = makeTex(m.normalTexture.index,                /*sRGB*/ false)) mat->SetNormalMap(t);
    if (auto t = makeTex(pmr.metallicRoughnessTexture.index,   /*sRGB*/ false)) mat->SetRoughnessMetalMap(t); // G: roughness, B: metallic
    if (auto t = makeTex(m.occlusionTexture.index,             /*sRGB*/ false)) mat->SetAOMap(t);
    if (auto t = makeTex(m.emissiveTexture.index,              /*sRGB*/ true )) mat->SetEmissiveMap(t);

    // glTF normalTexture.scale (default 1.0)
    mat->SetNormalScale(m.normalTexture.scale > 0.0 ? (float)m.normalTexture.scale : 1.0f);

    // ---- Alpha mode / cutoff / double-sided ----
    // alphaMode: "OPAQUE" (default), "MASK", "BLEND"
    PBRMaterial::AlphaMode mode = PBRMaterial::AlphaMode::Opaque;
    if (m.alphaMode == "MASK") {
        mode = PBRMaterial::AlphaMode::Mask;
        // alphaCutoff default = 0.5 if not specified by the asset
        float cutoff = (m.alphaCutoff > 0.0) ? (float)m.alphaCutoff : 0.5f;
        mat->SetAlphaCutoff(cutoff);
    } else if (m.alphaMode == "BLEND") {
        mode = PBRMaterial::AlphaMode::Blend;
    } // else Opaque
    mat->SetAlphaMode(mode);

    // doubleSided
    mat->SetDoubleSided(m.doubleSided);

    return mat;
}

