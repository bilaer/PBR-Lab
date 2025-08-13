#pragma once
#include <vector>
#include "stb_image.h"
#include "shader.h"

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


class UnitCube: public Geometry {
    public:
        UnitCube();
        virtual void SetupBuffers() override;
};


class ScreenQuad: public Geometry {
    public:
        ScreenQuad();
        virtual void SetupBuffers() override;
};
