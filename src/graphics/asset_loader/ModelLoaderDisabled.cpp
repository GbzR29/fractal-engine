#include "ModelLoader.hpp"
#include "Model.hpp"

#include <iostream>

std::shared_ptr<Model> ModelLoader::load(const std::string& path)
{
    std::cerr << "[ModelLoader] Assimp não está habilitado ou não foi compilado; ignorando: "
              << path << "\n";
    return nullptr;
}
