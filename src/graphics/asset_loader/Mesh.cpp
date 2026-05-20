#include "Mesh.hpp"

#include <cstddef>

Mesh::Mesh(std::vector<Vertex> verts, std::vector<unsigned int> idx, int matIdx)
    : vertices(std::move(verts)), indices(std::move(idx)), materialIndex(matIdx) {}

Mesh::~Mesh() { destroy(); }

Mesh::Mesh(Mesh&& o) noexcept
    : vertices(std::move(o.vertices)), indices(std::move(o.indices)),
      materialIndex(o.materialIndex), VAO(o.VAO), VBO(o.VBO), EBO(o.EBO),
      m_uploaded(o.m_uploaded)
{
    o.VAO = o.VBO = o.EBO = 0;
    o.m_uploaded          = false;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept
{
    if (this == &o) return *this;
    destroy();
    vertices       = std::move(o.vertices);
    indices        = std::move(o.indices);
    materialIndex  = o.materialIndex;
    VAO            = o.VAO;
    VBO            = o.VBO;
    EBO            = o.EBO;
    m_uploaded     = o.m_uploaded;
    o.VAO = o.VBO = o.EBO = 0;
    o.m_uploaded          = false;
    return *this;
}

void Mesh::destroy()
{
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    VAO = VBO = EBO = 0;
    m_uploaded      = false;
}

void Mesh::setup()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizei>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizei>(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = sizeof(Vertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, texCoords)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, tangent)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, bitangent)));

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, stride,
                           reinterpret_cast<void*>(offsetof(Vertex, boneIDs)));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, boneWeights)));

    glBindVertexArray(0);
}

void Mesh::uploadToGPU()
{
    if (m_uploaded || vertices.empty() || indices.empty())
        return;
    setup();
    m_uploaded = true;
}

void Mesh::draw() const
{
    if (!m_uploaded || !VAO || indices.empty())
        return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
