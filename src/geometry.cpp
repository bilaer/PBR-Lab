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

void Geometry::Draw(const std::shared_ptr<Shader>& shader) {
    shader->Use();
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

void Mesh::Init() {
    if (this->initialized) return; // Make sure don't repeatedly initialize buffers

    if (this->vertices.empty())
        this->GenerateVertices();   //< Must be implemented in derived class
    if (this->indices.empty())
        this->GenerateIndices();    //< Must be implemented in derived class
    this->GenerateTBN();            //< Generate tangent and bitagnet to calculate TBN matrix
    this->SetupBuffers();           //< Create and bind OpenGL buffers
    
    this->initialized = true;
}

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

    // tangent in TBN matrix
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(3);

    // bitangent in TBN matrix
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0); // Unbind VAO
}


// Calculate the tangent for TBN matrix
// Refer https://learnopengl.com/Advanced-Lighting/Normal-Mapping
glm::vec3 Mesh::CalTangent(const glm::vec2& deltaUV1, const glm::vec2& deltaUV2, const glm::vec3& edge1, const glm::vec3& edge2) {
    float d = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    float tangentX = (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x) * d;
    float tangentY = (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y) * d;
    float tangentZ = (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z) * d;
    glm::vec3 tangent = glm::normalize(glm::vec3(tangentX, tangentY, tangentZ));
    //std::cerr << "Ta: " << tangent.x << " " << tangent.y << " " << tangent.z << "\n" << std::endl;
    return tangent;

}

// Calculate the bitangent for TBN matrix
glm::vec3 Mesh::CalBitangent(const glm::vec2& deltaUV1, const glm::vec2& deltaUV2, const glm::vec3& edge1, const glm::vec3& edge2) {
    float d = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    float bitangentX = (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x) * d;
    float bitangentY = (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y) * d;
    float bitangentZ = (-deltaUV2.x * edge1.z - deltaUV1.x * edge2.z) * d;
    glm::vec3 bitangent = glm::normalize(glm::vec3(bitangentX, bitangentY, bitangentZ));
    return bitangent;
}


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
        // latitude(stacks) belongs to [pi/2, -pi/2]. i/stacks maps value to [0，1] for u (which will be used for uv)
        // -0.5 to maps value to [-1/2, 1/2]
        float stacks_angle = ((float)i / this->stacks - 0.5f) * PI;
        float y = this->radius * sin(stacks_angle);
        float zr = this->radius * cos(stacks_angle);
        for (int j = 0; j <= this->slices; ++j) {
            // longtitude(sector) belongs to [0, 2pi], again i/slices mapes t value to [0, 1] for v
            float slice_angle = 2 * PI * (float)j / this->slices;
            float x = zr * cos(slice_angle);
            float z = zr * sin(slice_angle);

            // Add vertex
            Vertex v;
            v.position = glm::vec3(x, y, z);

            // Calculate vertex normals
            v.normals = glm::normalize(glm::vec3(x, y, z));

            // Calculate texture coordinate
            float TexU = (float)j / this->slices;              
            float TexV = (float)i / this->stacks; 
            v.texCoord = glm::vec2(TexU, TexV);

            this->vertices.push_back(v);
        }
    };
}
/**
* Generate Indices for sphere, refers：https://www.songho.ca/opengl/gl_sphere.html
*             first+1  
* first    ●---●---●---●---●
*          |  /|  /|  /|  /|
*          | / | / | / | / |
* second   ●---●---●---●---●
*             second+1
* first->second->first+1, first+1->second->second+1 forms two triangles
*
*/ 
void Sphere::GenerateIndices() {
    for (int i = 0; i < this->stacks; ++i) {
        for (int j = 0; j < this->slices; ++j) {
            // 注意在生成vertices的时候每一圈纬度生成 slices + 1个点 （for (int j = 0; i <= slices; ++j），而不是 i < slices）
            // Notice
            // 比如 slices = 4：
            //     j = 0, 1, 2, 3, 4 → 生成了 5 个点，0 和 4 在空间位置上是重合的。
            // 只有生成 slices + 1个点才能使球体闭合. 
            // 在球体贴图时，让经度方向形成一个闭环，必须把 第一个点和最后一个点重复，这样纹理坐标或几何是连续的
            int first = i * (this->slices + 1) + j;
            // first 绕着球体转一圈才到达 second
            int second = first + this->slices + 1;
            
            // First Triagnle
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

// Generate TBN matrix for normal mapping
void Sphere::GenerateTBN() {
    std::vector<glm::vec3> tangents(this->vertices.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> bitangents(this->vertices.size(), glm::vec3(0.0f));

    for(size_t i = 0; i + 2 < this->indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        // Get points of the triangle
        glm::vec3 p1 = this->vertices[i0].position;
        glm::vec3 p2 = this->vertices[i1].position;
        glm::vec3 p3 = this->vertices[i2].position;
        
        // UV of the triangle
        glm::vec2 uv1 = this->vertices[i0].texCoord;
        glm::vec2 uv2 = this->vertices[i1].texCoord;
        glm::vec2 uv3 = this->vertices[i2].texCoord;

        glm::vec3 edge1 = p1 - p2;
        glm::vec3 edge2 = p3 - p2;

        // Calculate delta uv1 and delta uv2
        glm::vec2 deltaUV1 = uv1 - uv2;
        glm::vec2 deltaUV2 = uv3 - uv2;

        // Calculate the tangent and bitangent 
        glm::vec3 tangent = this->CalTangent(deltaUV1, deltaUV2, edge1, edge2);
        glm::vec3 bitangent = this->CalBitangent(deltaUV1, deltaUV2, edge1, edge2);

        // add to the vertices;
        tangents[i0] += tangent;
        tangents[i1] += tangent;
        tangents[i2] += tangent;

        bitangents[i0] += bitangent;
        bitangents[i1] += bitangent;
        bitangents[i2] += bitangent;
    }


    // normalize the tangent and bitangent
    for(size_t i = 0; i < this->vertices.size(); i ++) {
        // Gram-Schmdit process to make sure tangent, bitangent and normal are orthonormal basis
        glm::vec3 T = tangents[i];
        glm::vec3 N = vertices[i].normals;
        T = glm::normalize(T - N * glm::dot(N, T));
        glm::vec3 B = glm::normalize(glm::cross(N, T));
        
        this->vertices[i].tangent = T;
        this->vertices[i].bitangent = B;
    }

    // Remove texture seam
    for (int i = 0; i <= stacks; ++i) {
        int left  = i * (slices + 1) + 0;
        int right = i * (slices + 1) + slices;

        vertices[right].tangent = vertices[left].tangent;
        vertices[right].bitangent = vertices[left].bitangent;
        vertices[right].normals = vertices[left].normals;
    }
}
