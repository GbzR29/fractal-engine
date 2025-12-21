#pragma once

#include <memory>
#include <map>
#include <iostream>
#include "Chunk.h"

class World{

    public:
        // posição(wx,wy,wz), Chunk
        std::map<glm::ivec3, Chunk> chunks;

    private:

        int worldX, worldY, worldZ;
        bool isBlockSolid(int wx, int wy, int wz);

};