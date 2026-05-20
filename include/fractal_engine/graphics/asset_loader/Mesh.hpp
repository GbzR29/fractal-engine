/**
 * @file Mesh.hpp
 * @brief Single draw-call submesh: CPU vertex/index data and the corresponding GPU buffers.
 *
 * Meshes are not copyable because they own OpenGL VAO/VBO/EBO handles.
 * Call @ref uploadToGPU() on the GL thread before the first @ref draw() call.
 */
#pragma once

#include <glad/glad.h>
#include <vector>
#include "Vertex.hpp"
#include "Material.hpp"

/// @brief One indexed submesh — owns its own VAO, VBO, and EBO.
class Mesh {
public:
    std::vector<Vertex>       vertices;      ///< CPU-side vertex data (may be cleared after GPU upload).
    std::vector<unsigned int> indices;       ///< Triangle index list.
    int                       materialIndex = 0; ///< Index into the parent @ref Model::materials array.

    GLuint VAO = 0; ///< OpenGL Vertex Array Object.
    GLuint VBO = 0; ///< Vertex Buffer Object.
    GLuint EBO = 0; ///< Element (index) Buffer Object.

    Mesh() = default;

    /**
     * @brief Constructs a mesh from pre-built vertex and index arrays.
     * @param verts   Vertex data.
     * @param idx     Index data.
     * @param matIdx  Material index within the parent model.
     */
    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> idx, int matIdx = 0);
    ~Mesh();

    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;
    Mesh& operator=(Mesh&&) noexcept;

    /// Issues a @c glDrawElements call.  Requires @ref uploadToGPU() to have been called first.
    void draw() const;

    /// Uploads vertex and index data to the GPU.  Must be called on the OpenGL context thread.
    void uploadToGPU();

private:
    bool m_uploaded = false;
    void setup();
    void destroy();
};