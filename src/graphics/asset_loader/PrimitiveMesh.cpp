#include "PrimitiveMesh.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Skeleton.hpp"
#include "Vertex.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <vector>

static Vertex makeVert(glm::vec3 pos, glm::vec3 nrm, glm::vec2 uv,
                       glm::vec3 tan, glm::vec3 bit)
{
    Vertex v;
    v.position  = pos;
    v.normal    = nrm;
    v.texCoords = uv;
    v.tangent   = tan;
    v.bitangent = bit;
    return v;
}

// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Model> PrimitiveMesh::cube()
{
    struct Face {
        glm::vec3 n, t, b;
        glm::vec3 p[4]; // CCW: BL BR TR TL
    };

    const Face faces[6] = {
        // +Z front
        {{0,0,1},{1,0,0},{0,1,0},
         {{-0.5f,-0.5f, 0.5f},{ 0.5f,-0.5f, 0.5f},{ 0.5f, 0.5f, 0.5f},{-0.5f, 0.5f, 0.5f}}},
        // -Z back
        {{0,0,-1},{-1,0,0},{0,1,0},
         {{ 0.5f,-0.5f,-0.5f},{-0.5f,-0.5f,-0.5f},{-0.5f, 0.5f,-0.5f},{ 0.5f, 0.5f,-0.5f}}},
        // +X right
        {{1,0,0},{0,0,-1},{0,1,0},
         {{ 0.5f,-0.5f, 0.5f},{ 0.5f,-0.5f,-0.5f},{ 0.5f, 0.5f,-0.5f},{ 0.5f, 0.5f, 0.5f}}},
        // -X left
        {{-1,0,0},{0,0,1},{0,1,0},
         {{-0.5f,-0.5f,-0.5f},{-0.5f,-0.5f, 0.5f},{-0.5f, 0.5f, 0.5f},{-0.5f, 0.5f,-0.5f}}},
        // +Y top
        {{0,1,0},{1,0,0},{0,0,-1},
         {{-0.5f, 0.5f, 0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.5f, 0.5f,-0.5f},{-0.5f, 0.5f,-0.5f}}},
        // -Y bottom
        {{0,-1,0},{1,0,0},{0,0,1},
         {{-0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f, 0.5f},{-0.5f,-0.5f, 0.5f}}},
    };

    const glm::vec2 uvs[4] = {{0,0},{1,0},{1,1},{0,1}};

    std::vector<Vertex>       verts;
    std::vector<unsigned int> idx;
    verts.reserve(24);
    idx.reserve(36);

    for (const auto& f : faces) {
        unsigned int base = static_cast<unsigned int>(verts.size());
        for (int v = 0; v < 4; ++v)
            verts.push_back(makeVert(f.p[v], f.n, uvs[v], f.t, f.b));
        idx.insert(idx.end(), {base,base+1,base+2, base,base+2,base+3});
    }

    auto model = std::make_shared<Model>();
    model->name     = "Cube";
    model->skeleton = std::make_unique<Skeleton>();
    model->materials.push_back(std::make_shared<Material>());
    model->meshes.emplace_back(std::move(verts), std::move(idx), 0);
    model->boundsMin = glm::vec3(-0.5f);
    model->boundsMax = glm::vec3( 0.5f);
    model->uploadToGPU();
    return model;
}

// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Model> PrimitiveMesh::sphere(int stacks, int slices)
{
    const float PI = glm::pi<float>();

    std::vector<Vertex>       verts;
    std::vector<unsigned int> idx;
    verts.reserve((stacks + 1) * (slices + 1));
    idx.reserve(stacks * slices * 6);

    for (int lat = 0; lat <= stacks; ++lat) {
        float theta    = static_cast<float>(lat) / stacks * PI;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= slices; ++lon) {
            float phi    = static_cast<float>(lon) / slices * 2.0f * PI;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 nrm = {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
            glm::vec3 tan = {-sinPhi, 0.0f, cosPhi};
            glm::vec3 bit = glm::cross(nrm, tan);
            glm::vec2 uv  = {static_cast<float>(lon) / slices,
                              static_cast<float>(lat) / stacks};
            verts.push_back(makeVert(nrm * 0.5f, nrm, uv, tan, bit));
        }
    }

    for (int lat = 0; lat < stacks; ++lat) {
        for (int lon = 0; lon < slices; ++lon) {
            unsigned int a = static_cast<unsigned int>(lat * (slices + 1) + lon);
            unsigned int b = a + static_cast<unsigned int>(slices + 1);
            idx.insert(idx.end(), {a, b, a+1, b, b+1, a+1});
        }
    }

    auto model = std::make_shared<Model>();
    model->name     = "Sphere";
    model->skeleton = std::make_unique<Skeleton>();
    model->materials.push_back(std::make_shared<Material>());
    model->meshes.emplace_back(std::move(verts), std::move(idx), 0);
    model->boundsMin = glm::vec3(-0.5f);
    model->boundsMax = glm::vec3( 0.5f);
    model->uploadToGPU();
    return model;
}

// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Model> PrimitiveMesh::plane(float size)
{
    float h = size * 0.5f;
    const glm::vec3 n  = {0, 1, 0};
    const glm::vec3 t  = {1, 0, 0};
    const glm::vec3 bt = {0, 0,-1};

    std::vector<Vertex> verts = {
        makeVert({-h, 0,  h}, n, {0, 0}, t, bt),
        makeVert({ h, 0,  h}, n, {1, 0}, t, bt),
        makeVert({ h, 0, -h}, n, {1, 1}, t, bt),
        makeVert({-h, 0, -h}, n, {0, 1}, t, bt),
    };
    std::vector<unsigned int> idx = {0,1,2, 0,2,3};

    auto model = std::make_shared<Model>();
    model->name     = "Plane";
    model->skeleton = std::make_unique<Skeleton>();
    model->materials.push_back(std::make_shared<Material>());
    model->meshes.emplace_back(std::move(verts), std::move(idx), 0);
    model->boundsMin = {-h, 0.0f, -h};
    model->boundsMax = { h, 0.0f,  h};
    model->uploadToGPU();
    return model;
}
