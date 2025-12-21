#include "../headers/TextureLoader.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

TextureLoader::TextureLoader(GLuint texID, const char* path) {
    loadTexture(texID, path);
}

void TextureLoader::loadTexture(GLuint texID, const char* path) {

    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    int width, height, channel;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img = stbi_load(path, &width, &height, &channel, 0);

    if (!img) {
        std::cout << "Erro ao carregar imagem: " << path << std::endl;
        return;
    }

    GLenum format = (channel == 4) ? GL_RGBA : GL_RGB;
    GLenum internalFormat = (channel == 4) ? GL_RGBA8 : GL_RGB8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(img);
}
