/**
 * @file PrimitiveMesh.hpp
 * @brief Procedural mesh generators — cube, sphere, and plane, no Assimp required.
 *
 * Each function returns a fully GPU-uploaded @ref Model with one @ref Mesh and a default
 * @ref Material.  Results are not cached; call once and keep the @c shared_ptr.
 */
#pragma once
#include "Model.hpp"
#include <memory>

/// @brief Factory for simple procedural geometry primitives.
class PrimitiveMesh {
public:
    /// @return A unit cube (1×1×1) centred at the origin.
    static std::shared_ptr<Model> cube();

    /**
     * @brief Generates a UV sphere.
     * @param stacks  Latitude subdivisions (more = smoother).
     * @param slices  Longitude subdivisions (more = smoother).
     * @return A unit sphere centred at the origin.
     */
    static std::shared_ptr<Model> sphere(int stacks = 16, int slices = 16);

    /**
     * @brief Generates a flat horizontal plane.
     * @param size  Half-extent; the plane spans @c [-size, size] on X and Z.
     * @return A flat quad in the XZ plane at Y=0.
     */
    static std::shared_ptr<Model> plane(float size = 1.0f);
};
