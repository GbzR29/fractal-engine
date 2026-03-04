#pragma once

#include <glad/glad.h>

class TextureLoader {

    public:
        TextureLoader(GLuint texID, const char* path);

    private:
        void loadTexture(GLuint texID, const char* path);
};
