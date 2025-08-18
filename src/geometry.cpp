#include "geometry.h"
#include <vector>
#include <iostream>
#include <cmath>
#include "config.h"

Geometry::Geometry() {

}

Geometry::~Geometry() {
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
}

void Geometry::Draw() {
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(this->vertices.size() / 3));
    glBindVertexArray(0);
}

//====================Unit Cube====================
UnitCube::UnitCube() {
    this->vertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    this->SetupBuffers();
}

void UnitCube::SetupBuffers() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float), this->vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

//=================Screenquad======================
ScreenQuad::ScreenQuad() {
    this->vertices = {
        // pos        // texcoord
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f
    };
    this->SetupBuffers();
}

void ScreenQuad::SetupBuffers() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float),
                 this->vertices.data(), GL_STATIC_DRAW);

    // layout(location = 0) - position (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // layout(location = 1) - texcoord (vec2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

//===============Mesh======================
Mesh::Mesh():  
    VAO(0), 
    VBO(0), 
    EBO(0) {
}

Mesh::~Mesh() {
    // Check if buffers are generated before deleting to avoid invalid OpenGL calls.
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

// Used by derived class to procedurally generate vertices, indices and normal data
void Mesh::Init() {
    if (this->initialized) return; // Make sure don't repeatedly initialize buffers

    if (this->vertices.empty())
        this->GenerateVertices();   //< Must be implemented in derived class
    if (this->indices.empty())
        this->GenerateIndices();    //< Must be implemented in derived class
    this->SetupBuffers();           //< Create and bind OpenGL buffers
    
    this->initialized = true;
}

// API for model loader to bypass tangent/bitangent calculation
void Mesh::LoadFromModel(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
    this->vertices = vertices;
    this->indices = indices;
    this->SetupBuffers();
}

// Initialize VAO, VBO and EBO, enable location in shader
void Mesh::SetupBuffers() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);

    // Vertex position to layout = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Vertex norms to layout = 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normals));
    glEnableVertexAttribArray(1);

    // UV layout=2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO
}



// Draw the mesh
void Mesh::Draw(){
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// =================Sphere===================
// radius Radius of the sphere.
// stacks Number of latitude segments.
// slices Number of longitude segments.
Sphere::Sphere(float radius, int stacks, int slices): radius(radius), stacks(stacks), slices(slices) {
    // Init generate vertex and indice data and setup buffers
    this->Init();
}
// Calculation vertex, normals and texture coordinates，
// Vertex calculation refers https://www.songho.ca/opengl/gl_sphere.html
void Sphere::GenerateVertices() {
    for (int i = 0; i <= this->stacks; ++i) {
        float stacks_angle = ((float)i / this->stacks - 0.5f) * PI;  // Latitude angle
        float y = this->radius * sin(stacks_angle);  // Y position of the vertex
        float zr = this->radius * cos(stacks_angle); // Radius in the x-z plane
        
        for (int j = 0; j <= this->slices; ++j) {
            float slice_angle = 2 * PI * (float)j / this->slices;  // Longitude angle
            float x = zr * cos(slice_angle);
            float z = zr * sin(slice_angle);

            Vertex v;
            v.position = glm::vec3(x, y, z);
            v.normals = glm::normalize(glm::vec3(x, y, z)); // Normal vector (normalized)

            // Calculate texture coordinates
            float TexU = (float)j / this->slices;
            float TexV = (float)i / this->stacks;
            v.texCoord = glm::vec2(TexU, TexV);

            this->vertices.push_back(v);
        }
    }
}

void Sphere::GenerateIndices() {
    for (int i = 0; i < this->stacks; ++i) {
        for (int j = 0; j < this->slices; ++j) {
            int first = i * (this->slices + 1) + j;
            int second = first + this->slices + 1;
            
            // First Triangle
            this->indices.push_back(first);
            this->indices.push_back(second);
            this->indices.push_back(first + 1);

            // Second Triangle
            this->indices.push_back(first + 1);
            this->indices.push_back(second);
            this->indices.push_back(second + 1);
        }
    }
}


//====================================
//              Plane
//====================================
Plane::Plane(float size): size(size) {
    this->Init();
}

//Generates 4 vertices for a flat square plane on the XZ-plane.
void Plane::GenerateVertices() {
    float half = this->size / 2.0f;
    this->vertices = {
        {{-half, 0.0f, half}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 左下角
        {{ half, 0.0f, half}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 右下角
        {{ half, 0.0f, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 右上角
        {{-half, 0.0f, -half}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 左上角
    };
}

//Generates indices to form two triangles for the plane.
void Plane::GenerateIndices() {
    this->indices = {
        0, 1, 2,
        2, 3, 0
    };
}