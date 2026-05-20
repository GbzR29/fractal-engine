/**
 * @file ModelLoader.hpp
 * @brief Assimp-based 3-D model importer.
 *
 * Reads any format supported by Assimp (FBX, glTF, OBJ, …) and produces a fully
 * populated @ref Model with meshes, materials, skeleton, and animations.
 * Enable by defining @c FE_USE_ASSIMP in the CMake configuration; if disabled,
 * @ref load() returns @c nullptr.
 */
#pragma once

#include <memory>
#include <string>

class Model;

/// @brief Thin wrapper around the Assimp importer.
class ModelLoader {
public:
    /**
     * @brief Imports a model from an absolute file path.
     * @param path  Absolute filesystem path to the model file.
     * @return A fully-loaded @ref Model, or @c nullptr on failure.
     */
    static std::shared_ptr<Model> load(const std::string& path);
};
