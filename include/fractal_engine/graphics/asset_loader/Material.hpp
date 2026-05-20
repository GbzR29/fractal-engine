/**
 * @file Material.hpp
 * @brief PBR material — six texture slots and scalar factor overrides.
 *
 * Texture slots always contain a valid @ref Texture (1×1 fallback created on first use).
 * Call @ref bind() before each draw call to upload textures and uniform factors to the
 * active shader.
 */
#pragma once

#include "Texture.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

/// @brief Texture slot binding points matching the PBR shader uniforms.
namespace PBRSlot {
    constexpr GLuint Albedo    = 0; ///< Base colour / albedo map.
    constexpr GLuint Normal    = 1; ///< Tangent-space normal map.
    constexpr GLuint Metallic  = 2; ///< Metallic map (R channel).
    constexpr GLuint Roughness = 3; ///< Roughness map (R channel).
    constexpr GLuint AO        = 4; ///< Ambient occlusion map (R channel).
    constexpr GLuint Emissive  = 5; ///< Emissive colour map.
}

class Shader;

/// @brief Physically-based rendering material with six texture maps and scalar factors.
class Material {
public:
    std::string name; ///< Material name from the source asset.

    /// @name PBR texture maps
    /// All maps always hold a valid shared_ptr — missing maps use 1×1 fallback textures.
    /// @{
    std::shared_ptr<Texture> albedoMap;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Texture> metallicMap;
    std::shared_ptr<Texture> roughnessMap;
    std::shared_ptr<Texture> aoMap;
    std::shared_ptr<Texture> emissiveMap;
    /// @}

    /// @name Scalar/colour factors  (multiplied with the sampled texture value in the shader)
    /// @{
    glm::vec4 albedoFactor    = glm::vec4(1.0f); ///< Base colour tint and opacity.
    float     metallicFactor  = 0.0f;            ///< Metallic multiplier [0, 1].
    float     roughnessFactor = 1.0f;            ///< Roughness multiplier [0, 1].
    glm::vec3 emissiveFactor  = glm::vec3(0.0f); ///< Emissive colour and brightness.
    float     alphaCutoff     = 0.5f;            ///< Alpha cutoff threshold for masked transparency.
    /// @}

    bool doubleSided = false; ///< Disable back-face culling when @c true.
    bool transparent = false; ///< Use alpha-blending when @c true (instead of opaque).

    Material() { setDefaults(); }

    /**
     * @brief Binds all texture maps to their PBR slots and sets shader uniforms.
     * @param shader  A bound PBR shader that matches the @ref PBRSlot binding points.
     */
    void bind(Shader& shader) const;

private:
    void setDefaults() {
        albedoMap    = Texture::white();
        normalMap    = Texture::defaultNormal();
        metallicMap  = Texture::black();
        roughnessMap = Texture::white();
        aoMap        = Texture::white();
        emissiveMap  = Texture::black();
    }
};