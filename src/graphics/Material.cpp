#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

namespace {

// Retorna a textura se válida, caso contrário usa o fallback.
// Sempre faz bind — nunca deixa o sampler apontando para lixo.
void bindSlot(const std::shared_ptr<Texture>& tex,
              const std::shared_ptr<Texture>& fallback,
              GLuint slot, const char* uniform, Shader& shader)
{
    const auto& t = (tex && tex->isValid()) ? tex : fallback;
    t->bind(slot);
    shader.SetInt(uniform, static_cast<int>(slot));
}

} // namespace

void Material::bind(Shader& shader) const
{
    bindSlot(albedoMap,    Texture::white(),         PBRSlot::Albedo,    "uAlbedoMap",    shader);
    bindSlot(normalMap,    Texture::defaultNormal(), PBRSlot::Normal,    "uNormalMap",    shader);
    bindSlot(metallicMap,  Texture::black(),         PBRSlot::Metallic,  "uMetallicMap",  shader);
    bindSlot(roughnessMap, Texture::white(),         PBRSlot::Roughness, "uRoughnessMap", shader);
    bindSlot(aoMap,        Texture::white(),         PBRSlot::AO,        "uAOMap",        shader);
    bindSlot(emissiveMap,  Texture::black(),         PBRSlot::Emissive,  "uEmissiveMap",  shader);

    shader.SetVec4("uAlbedoFactor",    albedoFactor);
    shader.SetFloat("uMetallicFactor",  metallicFactor);
    shader.SetFloat("uRoughnessFactor", roughnessFactor);
    shader.SetVec3("uEmissiveFactor",   emissiveFactor);
    shader.SetFloat("uAlphaCutoff",     alphaCutoff);
}
