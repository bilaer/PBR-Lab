#include "model_loader.h"
#include <iostream>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static inline glm::vec3 toGLM(const aiVector3D& v) { return {v.x, v.y, v.z}; }
static inline glm::vec2 toGLM2(const aiVector3D& v) { return {v.x, v.y}; }

// Load PLY model into empty mesh class object
bool LoadPLYToMesh(const std::string& path, Mesh& mesh, bool flipUVs)
{
    Assimp::Importer importer;
    unsigned int flags =
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |   // use when PLY don't have normal data
        aiProcess_CalcTangentSpace |   // generate tangent/bitangent for normal map
        aiProcess_ImproveCacheLocality |
        aiProcess_SortByPType;
    if (flipUVs) flags |= aiProcess_FlipUVs;

    const aiScene* scene = importer.ReadFile(path, flags);
    if (!scene || !scene->mRootNode || scene->mNumMeshes == 0) {
        std::cerr << "[PLYLoader] Failed to load: " << path
                  << "  Error: " << importer.GetErrorString() << "\n";
        return false;
    }

    const aiMesh* aimesh = scene->mMeshes[0];

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.resize(aimesh->mNumVertices);
    for (unsigned i = 0; i < aimesh->mNumVertices; ++i) {
        Vertex v{};
        v.position = aimesh->HasPositions() ? toGLM(aimesh->mVertices[i]) : glm::vec3(0.0f);
        v.normals  = aimesh->HasNormals()   ? toGLM(aimesh->mNormals[i])  : glm::vec3(0,1,0);

        if (aimesh->HasTextureCoords(0)) v.texCoord = toGLM2(aimesh->mTextureCoords[0][i]);
        else                            v.texCoord = glm::vec2(0.0f);

        if (aimesh->HasTangentsAndBitangents()) {
            v.tangent   = toGLM(aimesh->mTangents[i]);
            v.bitangent = toGLM(aimesh->mBitangents[i]);
        } else {
            v.tangent = v.bitangent = glm::vec3(0.0f);
        }

        vertices[i] = v;
    }

    indices.reserve(aimesh->mNumFaces * 3);
    for (unsigned f = 0; f < aimesh->mNumFaces; ++f) {
        const aiFace& face = aimesh->mFaces[f];
        for (unsigned j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // Upload vertices and indices to mesh data
    mesh.LoadFromModel(vertices, indices);

    return true;
}
