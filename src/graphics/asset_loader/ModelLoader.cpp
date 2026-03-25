#include "ModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ──────────────────────────────────────────────────────────────────────────
//  Public: load
// ──────────────────────────────────────────────────────────────────────────
std::shared_ptr<Model> ModelLoader::load(const std::string& path) {
    Assimp::Importer importer;

    constexpr unsigned int flags =
        aiProcess_Triangulate            |
        aiProcess_GenSmoothNormals       |
        aiProcess_CalcTangentSpace       |
        aiProcess_FlipUVs                |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_LimitBoneWeights       |   // cap to MAX_BONE_INFLUENCE
        aiProcess_ImproveCacheLocality   |
        aiProcess_RemoveRedundantMaterials;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "[ModelLoader] Assimp error: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    auto model        = std::make_shared<Model>();
    model->sourcePath = path;
    model->name       = fs::path(path).stem().string();

    fs::path dir = fs::path(path).parent_path();

    // ── Pre-load all materials ─────────────────────────────
    model->materials.reserve(scene->mNumMaterials);
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
        model->materials.push_back(processMaterial(scene->mMaterials[i], dir));

    // ── Skeleton root ──────────────────────────────────────
    model->skeleton = std::make_unique<Skeleton>();
    model->skeleton->globalInverseTransform =
        glm::inverse(toGLM(scene->mRootNode->mTransformation));

    buildSkeletonNode(scene->mRootNode, model->skeleton->rootNode);

    // ── Meshes (recursively) ───────────────────────────────
    processNode(scene->mRootNode, scene, *model, dir);

    // ── Animations ────────────────────────────────────────
    if (scene->HasAnimations())
        extractAnimations(scene, *model);

    // ── AABB ─────────────────────────────────────────────
    model->boundsMin = glm::vec3(std::numeric_limits<float>::max());
    model->boundsMax = glm::vec3(std::numeric_limits<float>::lowest());
    for (const auto& mesh : model->meshes)
        for (const auto& v : mesh.vertices) {
            model->boundsMin = glm::min(model->boundsMin, v.position);
            model->boundsMax = glm::max(model->boundsMax, v.position);
        }

    std::cout << "[ModelLoader] Loaded: " << path
              << "  meshes=" << model->meshes.size()
              << "  bones="  << model->skeleton->boneCount
              << "  anims="  << model->animations.size() << "\n";

    return model;
}

// ──────────────────────────────────────────────────────────────────────────
//  processNode
// ──────────────────────────────────────────────────────────────────────────
void ModelLoader::processNode(const aiNode* node, const aiScene* scene,
                               Model& model, const fs::path& dir)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.push_back(processMesh(mesh, scene, model, dir));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, model, dir);
}

// ──────────────────────────────────────────────────────────────────────────
//  processMesh
// ──────────────────────────────────────────────────────────────────────────
Mesh ModelLoader::processMesh(const aiMesh* mesh, const aiScene* /*scene*/,
                               Model& model, const fs::path& /*dir*/)
{
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v;
        v.position = toGLM(mesh->mVertices[i]);
        v.normal   = mesh->HasNormals() ? toGLM(mesh->mNormals[i]) : glm::vec3(0,1,0);

        if (mesh->mTextureCoords[0])
            v.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

        if (mesh->HasTangentsAndBitangents()) {
            v.tangent   = toGLM(mesh->mTangents[i]);
            v.bitangent = toGLM(mesh->mBitangents[i]);
        }
        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    // Bone weights
    extractBones(mesh, *model.skeleton, vertices);

    int matIdx = (int)mesh->mMaterialIndex;
    return Mesh(std::move(vertices), std::move(indices), matIdx);
}

// ──────────────────────────────────────────────────────────────────────────
//  processMaterial
// ──────────────────────────────────────────────────────────────────────────
std::shared_ptr<Material> ModelLoader::processMaterial(const aiMaterial* mat,
                                                        const fs::path& dir)
{
    auto m = std::make_shared<Material>();

    aiString aiName;
    mat->Get(AI_MATKEY_NAME, aiName);
    m->name = aiName.C_Str();

    // PBR textures
    m->albedoMap    = loadMaterialTexture(mat, aiTextureType_DIFFUSE,         TextureType::Albedo,    dir, true);
    m->normalMap    = loadMaterialTexture(mat, aiTextureType_NORMALS,          TextureType::Normal,    dir);
    m->metallicMap  = loadMaterialTexture(mat, aiTextureType_METALNESS,        TextureType::Metallic,  dir);
    m->roughnessMap = loadMaterialTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS,TextureType::Roughness, dir);
    m->aoMap        = loadMaterialTexture(mat, aiTextureType_AMBIENT_OCCLUSION,TextureType::AO,        dir);
    m->emissiveMap  = loadMaterialTexture(mat, aiTextureType_EMISSIVE,         TextureType::Emissive,  dir, true);

    // Scalar factors
    aiColor4D col;
    if (mat->Get(AI_MATKEY_BASE_COLOR, col) == AI_SUCCESS)
        m->albedoFactor = { col.r, col.g, col.b, col.a };

    float val;
    if (mat->Get(AI_MATKEY_METALLIC_FACTOR,  val) == AI_SUCCESS) m->metallicFactor  = val;
    if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, val) == AI_SUCCESS) m->roughnessFactor = val;

    int twoSided = 0;
    mat->Get(AI_MATKEY_TWOSIDED, twoSided);
    m->doubleSided = twoSided != 0;

    return m;
}

std::shared_ptr<Texture> ModelLoader::loadMaterialTexture(const aiMaterial* mat,
                                                           aiTextureType type,
                                                           TextureType engineType,
                                                           const fs::path& dir,
                                                           bool sRGB)
{
    if (mat->GetTextureCount(type) == 0) return nullptr;

    aiString texPath;
    mat->GetTexture(type, 0, &texPath);

    fs::path full = dir / texPath.C_Str();
    auto tex = Texture::load(full.string(), engineType, true, sRGB);
    return tex; // may be nullptr; Material::setDefaults fills in the fallback
}

// ──────────────────────────────────────────────────────────────────────────
//  extractBones
// ──────────────────────────────────────────────────────────────────────────
void ModelLoader::extractBones(const aiMesh* mesh, Skeleton& skeleton,
                                std::vector<Vertex>& vertices)
{
    for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
        const aiBone* bone   = mesh->mBones[b];
        std::string   name   = bone->mName.C_Str();
        glm::mat4     offset = toGLM(bone->mOffsetMatrix);

        int id = skeleton.addBone(name, offset);

        for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
            unsigned int vid = bone->mWeights[w].mVertexId;
            float        wt  = bone->mWeights[w].mWeight;
            if (vid < vertices.size())
                vertices[vid].addBoneData(id, wt);
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────
//  buildSkeletonNode  (recursive)
// ──────────────────────────────────────────────────────────────────────────
void ModelLoader::buildSkeletonNode(const aiNode* node, SkeletonNode& out) {
    out.name           = node->mName.C_Str();
    out.localTransform = toGLM(node->mTransformation);
    out.children.resize(node->mNumChildren);
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        buildSkeletonNode(node->mChildren[i], out.children[i]);
}

// ──────────────────────────────────────────────────────────────────────────
//  extractAnimations
// ──────────────────────────────────────────────────────────────────────────
void ModelLoader::extractAnimations(const aiScene* scene, Model& model) {
    model.animations.reserve(scene->mNumAnimations);

    for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {
        const aiAnimation* ai = scene->mAnimations[a];

        Animation anim;
        anim.name           = ai->mName.C_Str();
        anim.duration       = (float)ai->mDuration;
        anim.ticksPerSecond = ai->mTicksPerSecond > 0.0
                              ? (float)ai->mTicksPerSecond : 25.0f;

        for (unsigned int c = 0; c < ai->mNumChannels; ++c) {
            const aiNodeAnim* ch = ai->mChannels[c];
            BoneChannel bc;
            bc.boneName = ch->mNodeName.C_Str();

            bc.positions.reserve(ch->mNumPositionKeys);
            for (unsigned int k = 0; k < ch->mNumPositionKeys; ++k)
                bc.positions.push_back({ toGLM(ch->mPositionKeys[k].mValue),
                                         (float)ch->mPositionKeys[k].mTime });

            bc.rotations.reserve(ch->mNumRotationKeys);
            for (unsigned int k = 0; k < ch->mNumRotationKeys; ++k)
                bc.rotations.push_back({ toGLM(ch->mRotationKeys[k].mValue),
                                         (float)ch->mRotationKeys[k].mTime });

            bc.scales.reserve(ch->mNumScalingKeys);
            for (unsigned int k = 0; k < ch->mNumScalingKeys; ++k)
                bc.scales.push_back({ toGLM(ch->mScalingKeys[k].mValue),
                                      (float)ch->mScalingKeys[k].mTime });

            anim.channels[bc.boneName] = std::move(bc);
        }

        model.animations.push_back(std::move(anim));
    }
}

// ──────────────────────────────────────────────────────────────────────────
//  Conversion helpers
// ──────────────────────────────────────────────────────────────────────────
glm::mat4 ModelLoader::toGLM(const aiMatrix4x4& m) {
    return glm::transpose(glm::make_mat4(&m.a1));
}
glm::vec3 ModelLoader::toGLM(const aiVector3D& v) {
    return { v.x, v.y, v.z };
}
glm::quat ModelLoader::toGLM(const aiQuaternion& q) {
    return glm::quat(q.w, q.x, q.y, q.z);
}