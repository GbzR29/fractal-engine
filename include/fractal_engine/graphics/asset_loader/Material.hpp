#pragma once

#include "Texture.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

// Slot bindings — match your PBR shader uniforms
namespace PBRSlot {
    constexpr GLuint Albedo    = 0;
    constexpr GLuint Normal    = 1;
    constexpr GLuint Metallic  = 2;
    constexpr GLuint Roughness = 3;
    constexpr GLuint AO        = 4;
    constexpr GLuint Emissive  = 5;
}

class Shader; // forward declare

class Material {
public:
    std::string name;

    // PBR textures (always valid — fallback to 1x1 if missing)
    std::shared_ptr<Texture> albedoMap;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Texture> metallicMap;
    std::shared_ptr<Texture> roughnessMap;
    std::shared_ptr<Texture> aoMap;
    std::shared_ptr<Texture> emissiveMap;

    // Scalar / color factors (multiplied with texture in shader)
    glm::vec4 albedoFactor    = glm::vec4(1.0f);
    float     metallicFactor  = 0.0f;
    float     roughnessFactor = 1.0f;
    glm::vec3 emissiveFactor  = glm::vec3(0.0f);
    float     alphaCutoff     = 0.5f;

    bool doubleSided = false;
    bool transparent = false;

    Material() { setDefaults(); }

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