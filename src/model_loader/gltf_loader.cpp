#include "model_loader/gltf_loader.h"
#include "scene.h"
#include "geometry.h"
#include <iostream>

// Load the glTF file and convert it into SceneNode objects
bool GltfLoader::LoadFile(const std::string& path, std::shared_ptr<Scene>& scene) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // Load the glTF file
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    if (!ret) {
        std::cerr << "Failed to load glTF: " << err << std::endl;
        return false;
    }

    // Iterate through the meshes in the glTF model
    for (const auto& gltfMesh : model.meshes) {
        for (const auto& primitive : gltfMesh.primitives) {
            // Load mesh data
            std::shared_ptr<Mesh> mesh = LoadMesh(model, primitive);

            // Load material data
            std::shared_ptr<PBRMaterial> material = LoadMaterial(model, primitive.material);

            // Create a SceneNode and add it to the scene
            std::shared_ptr<SceneNode> node = std::make_shared<SceneNode>(mesh, material);
            scene->AddNode(node);
        }
    }

    return true;
}

// Load mesh data from glTF
std::shared_ptr<Mesh> GltfLoader::LoadMesh(const tinygltf::Model& model, const tinygltf::Primitive& primitive) {
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    // Load vertices
    const auto& positionsAccessor = model.accessors[primitive.attributes.at("POSITION")];
    const auto& positionBufferView = model.bufferViews[positionsAccessor.bufferView];
    const auto& positionData = model.buffers[positionBufferView.buffer];

    const float* positionPtr = reinterpret_cast<const float*>(&positionData.data[positionsAccessor.byteOffset]);

    std::vector<Vertex> vertices;
    for (size_t i = 0; i < positionsAccessor.count; ++i) {
        Vertex vertex;

        // Load position
        vertex.position = glm::vec3(positionPtr[i * 3], positionPtr[i * 3 + 1], positionPtr[i * 3 + 2]);

        // Load normals
        const auto& normalsAccessor = model.accessors[primitive.attributes.at("NORMAL")];
        const auto& normalBufferView = model.bufferViews[normalsAccessor.bufferView];
        const auto& normalData = model.buffers[normalBufferView.buffer];
        const float* normalPtr = reinterpret_cast<const float*>(&normalData.data[normalsAccessor.byteOffset]);
        vertex.normals = glm::vec3(normalPtr[i * 3], normalPtr[i * 3 + 1], normalPtr[i * 3 + 2]);

        // Load texture coordinates
        const auto& texCoordAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
        const auto& texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
        const auto& texCoordData = model.buffers[texCoordBufferView.buffer];
        const float* texCoordPtr = reinterpret_cast<const float*>(&texCoordData.data[texCoordAccessor.byteOffset]);
        vertex.texCoord = glm::vec2(texCoordPtr[i * 2], texCoordPtr[i * 2 + 1]);

        // Load tangents
        const auto& tangentAccessor = model.accessors[primitive.attributes.at("TANGENT")];
        const auto& tangentBufferView = model.bufferViews[tangentAccessor.bufferView];
        const auto& tangentData = model.buffers[tangentBufferView.buffer];
        const float* tangentPtr = reinterpret_cast<const float*>(&tangentData.data[tangentAccessor.byteOffset]);
        vertex.tangent = glm::vec3(tangentPtr[i * 3], tangentPtr[i * 3 + 1], tangentPtr[i * 3 + 2]);

        // Load bitangents (if available)
        if (primitive.attributes.find("BITANGENT") != primitive.attributes.end()) {
            const auto& bitangentAccessor = model.accessors[primitive.attributes.at("BITANGENT")];
            const auto& bitangentBufferView = model.bufferViews[bitangentAccessor.bufferView];
            const auto& bitangentData = model.buffers[bitangentBufferView.buffer];
            const float* bitangentPtr = reinterpret_cast<const float*>(&bitangentData.data[bitangentAccessor.byteOffset]);
            vertex.bitangent = glm::vec3(bitangentPtr[i * 3], bitangentPtr[i * 3 + 1], bitangentPtr[i * 3 + 2]);
        }

        vertices.push_back(vertex);
    }

    mesh->SetVertices(vertices);

    // Load indices
    const auto& indicesAccessor = model.accessors[primitive.indices];
    const auto& indexBufferView = model.bufferViews[indicesAccessor.bufferView];
    const auto& indexData = model.buffers[indexBufferView.buffer];
    const uint32_t* indexPtr = reinterpret_cast<const uint32_t*>(&indexData.data[indicesAccessor.byteOffset]);

    std::vector<unsigned int> indices;
    for (size_t i = 0; i < indicesAccessor.count; ++i) {
        indices.push_back(indexPtr[i]);
    }

    mesh->SetIndices(indices);

    return mesh;
}

// Load material data from glTF
std::shared_ptr<PBRMaterial> GltfLoader::LoadMaterial(const tinygltf::Model& model, int materialIndex) {
    const auto& gltfMaterial = model.materials[materialIndex];
    std::shared_ptr<PBRMaterial> material = std::make_shared<PBRMaterial>();

    // Set base color
    const auto& baseColorFactor = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
    material->SetBaseColor(glm::vec3(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2]));

    // Set roughness and metalness
    material->SetRoughness(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
    material->SetMetalness(gltfMaterial.pbrMetallicRoughness.metallicFactor);

    // Load textures (if available)
    if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const auto& textureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        const auto& texture = model.textures[textureIndex];
        const auto& image = model.images[texture.source];
        material->LoadAlbedoMap(image.uri);
    }

    return material;
}
