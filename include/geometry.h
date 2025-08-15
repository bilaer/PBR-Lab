#pragma once
#include <vector>
#include "stb_image.h"
#include "shader.h"

// ========================Gemoetry==========================
// only contain basic VAO and VBO
// No support for UV, tangent bitangent
class Geometry {
    public: 
        Geometry();
        ~Geometry();
        virtual void SetupBuffers() = 0;
        void Draw(const std::shared_ptr<Shader>& shader);
    protected:
        GLuint VAO;
        GLuint VBO;
        std::vector<float> vertices;
};

// Used for cubemap rendering
class UnitCube: public Geometry {
    public:
        UnitCube();
        virtual void SetupBuffers() override;
};

// Use for frame buffer rendering
class ScreenQuad: public Geometry {
    public:
        ScreenQuad();
        virtual void SetupBuffers() override;
};


// ==================Mesh=============================
// Provide full support for UV, normal map calculation
struct Vertex {
    glm::vec3 position;
    glm::vec3 normals;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class Mesh {
    public:
        Mesh();
        virtual ~Mesh();

        unsigned int GetVBO() const { return this->VBO; };
        unsigned int GetVAO() const { return this->VAO; };
        unsigned int GetEBO() const { return this->EBO; };
        void SetVBO(const unsigned int VBO) { this->VBO = VBO; };
        void SetVAO(const unsigned int VAO) { this->VAO = VAO; };
        void SetEBO(const unsigned int EBO) { this->EBO = EBO; };

        // 获取vertices和indices和为为vertices和indices赋值
        const std::vector<struct Vertex>& GetVertices() const { return this->vertices; };
        const std::vector<unsigned int>& GetIndices() const { return this->indices; };
        void SetVertices(std::vector<struct Vertex> vertices) { this->vertices = vertices; };
        void SetIndices(std::vector<unsigned int> indices) { this->indices = indices; };
        
        void Init();
        virtual void Draw();
        // Initialize VBO VAO and EBO buffers based on vertices and indices data 
        void SetupBuffers();

    protected:
        bool initialized;

        std::vector<struct Vertex> vertices{};
        std::vector<unsigned int> indices{};

        unsigned int VBO = 0;
        unsigned int VAO = 0;
        unsigned int EBO = 0;
 
        // Calculate Tangent and Bitangent vector for normal map 
        glm::vec3 CalTangent(const glm::vec2& deltaUV1, const glm::vec2& deltaUV2, const glm::vec3& p1, const glm::vec3& p2);
        glm::vec3 CalBitangent(const glm::vec2& deltaUV1, const glm::vec2& deltaUV2, const glm::vec3& p1, const glm::vec3& p2);
        // Generate tangent and bitangent for normal map
        virtual void GenerateTBN() {};
        virtual void GenerateVertices() {};
        virtual void GenerateIndices() {};
};

/**
 * Sphere
 * Represents a 3D sphere mesh with configurable radius, stack, and slice count.
 * 
 * Vertex and index generation is based on:
 * https://www.songho.ca/opengl/gl_sphere.html
 */
class Sphere: public Mesh
{
    public:
        /**
         * Constructs a Sphere object.
         * 
         * radius Radius of the sphere.
         * stacks Number of segments along the vertical axis (latitude).
         * slices Number of segments along the horizontal axis (longitude).
         */
        Sphere(float radius, int stacks, int slices);
    private:
        float radius; ///< Radius of the sphere.
        int stacks;   ///< Number of latitude divisions (vertical).
        int slices;   ///< Number of longitude divisions (horizontal).

        /**
         * Generates vertices, normals, and texture coordinates for the sphere.
         * Algorithm reference: https://www.songho.ca/opengl/gl_sphere.html
         */
        virtual void GenerateVertices() override;
        /**
         *  Generates indices for triangle rendering of the sphere.
         * Algorithm reference: https://www.songho.ca/opengl/gl_sphere.html
         */
        virtual void GenerateIndices() override;
        virtual void GenerateTBN() override;
};



