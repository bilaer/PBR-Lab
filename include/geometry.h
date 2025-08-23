#pragma once
#include <vector>
#include "stb_image.h"
#include "shader.h"

// ========================Gemoetry==========================
// Provide concise data strcture for storing baisc object for
// drawing cubemap and quad for framebuffer drawing
// No support for UV, tangent bitangent
class Geometry {
    public: 
        Geometry();
        ~Geometry();
        virtual void SetupBuffers() = 0;
        void Draw();
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


// ========================Mesh========================================
// Provide more extensive support for UV, normal map for procedurally 
// generated or loaded model
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

        const std::vector<struct Vertex>& GetVertices() const { return this->vertices; };
        const std::vector<unsigned int>& GetIndices() const { return this->indices; };
        void SetVertices(std::vector<struct Vertex> vertices) { this->vertices = vertices; };
        void SetIndices(std::vector<unsigned int> indices) { this->indices = indices; };
        
        void Init();
        virtual void Draw();
        // Initialize VBO VAO and EBO buffers based on vertices and indices data 
        void SetupBuffers();

        // API for model loader to bypass tangent/bitangent calculation
        void LoadFromModel(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
        
    protected:
        bool initialized;

        std::vector<struct Vertex> vertices{};
        std::vector<unsigned int> indices{};

        unsigned int VBO = 0;
        unsigned int VAO = 0;
        unsigned int EBO = 0;

        virtual void GenerateVertices() {};
        virtual void GenerateIndices() {};
};

//========================================
//               Sphere
//========================================
/*
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

};

//========================================
//              Plane
//========================================

class Plane: public Mesh
{
    public:
        Plane(float size);
        float GetSize() { return this->size; };
        void SetSize(float size) { this->size = size; };

        virtual void GenerateVertices() override;
        virtual void GenerateIndices() override;
    private:
        float size;
    
};

//=========================================
//                  Ray
//=========================================
// Use for ray intersection and etc
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& origin, const glm::vec3& dir)
        : origin(origin), direction(glm::normalize(dir)) {}
};


