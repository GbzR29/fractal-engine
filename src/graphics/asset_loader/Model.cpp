#include "Model.hpp"

#include "Shader.hpp"

namespace {

void uploadMaterialTextures(const std::shared_ptr<Material>& m)
{
    if (!m) return;
    if (m->albedoMap && m->albedoMap->needsGpuUpload()) m->albedoMap->uploadToGPU();
    if (m->normalMap && m->normalMap->needsGpuUpload()) m->normalMap->uploadToGPU();
    if (m->metallicMap && m->metallicMap->needsGpuUpload()) m->metallicMap->uploadToGPU();
    if (m->roughnessMap && m->roughnessMap->needsGpuUpload()) m->roughnessMap->uploadToGPU();
    if (m->aoMap && m->aoMap->needsGpuUpload()) m->aoMap->uploadToGPU();
    if (m->emissiveMap && m->emissiveMap->needsGpuUpload()) m->emissiveMap->uploadToGPU();
}

} // namespace

void Model::uploadToGPU()
{
    if (m_uploaded) return;

    for (auto& mat : materials)
        uploadMaterialTextures(mat);

    for (auto& mesh : meshes)
        mesh.uploadToGPU();

    m_uploaded = true;
}

void Model::draw(Shader& shader) const
{
    for (const auto& mesh : meshes) {
        if (mesh.materialIndex >= 0
            && static_cast<size_t>(mesh.materialIndex) < materials.size()) {
            const auto& mat = materials[static_cast<size_t>(mesh.materialIndex)];
            if (mat)
                mat->bind(shader);
        }
        mesh.draw();
    }
}

void Model::drawAnimated(Shader& shader, const Animator& animator) const
{
    if (hasSkeleton()) {
        const auto& mats = animator.getBoneTransforms();
        shader.SetMat4Array("uBoneMatrices", mats.data(),
                            static_cast<int>(mats.size()));
    }
    draw(shader);
}
