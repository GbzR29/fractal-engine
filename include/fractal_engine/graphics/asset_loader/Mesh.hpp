#pragma once

#include <glad/glad.h>
#include <vector>
#include "Vertex.hpp"
#include "Material.hpp"

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    int                       materialIndex = 0;

    GLuint VAO = 0, VBO = 0, EBO = 0;

    Mesh() = default;
    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> idx, int matIdx = 0);
    ~Mesh();

    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;
    Mesh& operator=(Mesh&&) noexcept;

    void draw() const;
    void uploadToGPU(); // call after construction on GL thread

private:
    bool m_uploaded = false;
    void setup();
    void destroy();
};