#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../headers/Shader.h"
#include "../headers/TextureLoader.h"

#include <vector>

class Cube
{
private:

    GLuint vao, vbo;
    GLuint textures[4];

    glm::vec3 position;
    glm::vec3 halfSize = glm::vec3(0.5f); // como seu cubo é 1x1x1

    std::vector<float> faces;

    //0 = grass top, 1 = grass bottom, 2 = grass side
        
    float face_frontal[36] = {

        //face frontal
       -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  2,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  2,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  2,

		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  2,
	   -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  2,
	   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  2,
    };

    float face_traseira[36] = {

        //face traseira
       -0.5f, -0.5f,  -0.5f,  0.0f,  0.0f,  2,
		0.5f, -0.5f,  -0.5f,  1.0f,  0.0f,  2,
		0.5f,  0.5f,  -0.5f,  1.0f,  1.0f,  2,

		0.5f,  0.5f,  -0.5f,  1.0f,  1.0f,  2,
	   -0.5f,  0.5f,  -0.5f,  0.0f,  1.0f,  2, 
	   -0.5f, -0.5f,  -0.5f,  0.0f,  0.0f,  2,

    };

    float face_esquerda[36] = {

        //face esquerda
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2, 
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2,

		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2,
    };

    float face_direita[36] = {

        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  2,
		0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  2,
		0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  2,

		0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  2,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  2,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  2,

    };

    float face_inferior[36] = {
        
        //face inferior
       -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  1,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  1,

		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  1,
	   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1,
	   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1,
    };

    float face_superior[36] = {
    
        //face superior
       -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0,
	   -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0,
	   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0,
            
    };    

public:
    
    Cube(glm::vec3 pos, Shader &shader) : position(pos)
    {
        faces.insert(faces.end(), face_frontal, face_frontal + 36);
        faces.insert(faces.end(), face_traseira, face_traseira + 36);
        faces.insert(faces.end(), face_esquerda, face_esquerda + 36);
        faces.insert(faces.end(), face_direita, face_direita + 36);
        faces.insert(faces.end(), face_inferior, face_inferior + 36);
        faces.insert(faces.end(), face_superior, face_superior + 36);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(float), faces.data(), GL_STATIC_DRAW);

        // posição
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // texcoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // texID
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        //Adiciona texturas
        glGenTextures(4, textures);
        
        TextureLoader (textures[0], "../src/assets/grass-top.jpg");
        TextureLoader (textures[1], "../src/assets/grass-bottom.jpg");
        TextureLoader (textures[2], "../src/assets/grass_side.png");
        TextureLoader (textures[3], "../src/assets/wall.jpg");

        shader.use();
        shader.setInt("texture1", 0);
        shader.setInt("texture2", 1);
        shader.setInt("texture3", 2);
        shader.setInt("texture4", 3);
    }

    ~Cube()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    void Draw(Shader &shader)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures[2]);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures[3]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        shader.setMat4("model", model);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

	void setPosition(const glm::vec3& pos)
    {
        position = pos;
    }

    glm::vec3 getPosition(){

        return position;
    }

    glm::vec3 GetMin() const {
        return position + glm::vec3(-0.5f, -0.5f, -0.5f);
    }

    glm::vec3 GetMax() const {
        return position + glm::vec3(0.5f, 0.5f, 0.5f);
    }

    bool CollidesWith(const Cube& other) const {
        
        glm::vec3 aMin = GetMin();
        glm::vec3 aMax = GetMax();
        glm::vec3 bMin = other.GetMin();
        glm::vec3 bMax = other.GetMax();

        return (aMin.x <= bMax.x && aMax.x >= bMin.x) && (aMin.y <= bMax.y && aMax.y >= bMin.y) && (aMin.z <= bMax.z && aMax.z >= bMin.z);
    }
};