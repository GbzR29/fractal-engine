#define STB_IMAGE_IMPLEMENTATION

#include "Program.h"

#include <third_party/glm/glm.hpp>
#include <third_party/glm/gtc/matrix_transform.hpp>
#include <third_party/glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "fractal_engine/scene/Player.h"
#include "fractal_engine/graphics/Shader.h"
#include "fractal_engine/world/World.h"
#include "fractal_engine/input/Input.h"
#include "../objects/Cube.h"

using namespace fractal_engine::graphics;
using namespace fractal_engine::world;
using namespace fractal_engine::input;
using namespace fractal_engine::core;



/**
 * @brief Variáveis de controle de cena e tempo.
 */
static int current_scene = 0;
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

/**
 * @brief UI rendering buffers.
 */
static GLuint vao, vbo, ebo;

void CreateGameUI()
{
    float vertex[] = {
        -0.01f, -0.01f, 0.0f,
         0.01f, -0.01f, 0.0f,
         0.01f,  0.01f, 0.0f,
        -0.01f,  0.01f, 0.0f,
    };

    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void drawGameUI(Shader &ui_shader)
{
    ui_shader.use();
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

/**
 * @brief Renders the main game scene.
 * Note que agora passamos o World em vez do vector de Chunks.
 */
void game_scene(Shader &shader, World &world, Player &player, Shader &ui_shader, Input &input, Window &window)
{
    glClearColor(0.38f, 0.76f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Atualizar lógica (Movimentação + Colisão)
    input.update(window.getNativeWindow());
    player.update(deltaTime, input, world); // Aqui a mágica da colisão acontece

    // 2. Renderizar Câmera
    player.renderCamera(shader, 1080, 720);

    // 3. Renderizar Mundo
    shader.use();
    world.render(shader); // O World agora cuida do loop de desenho dos chunks

    // 4. Renderizar UI
    drawGameUI(ui_shader);
}

void menu_scene(Shader &ui_shader)
{
    drawGameUI(ui_shader);
}

void Init_Program()
{
    Window window(1080, 720, "Fractal Engine");

    if (!window.init())
    {
        std::cerr << "Failed to initialize window system." << std::endl;
        return;
    }

    // Instanciamos o Player e o World
    Player player;
    World world;

    Shader shader("shaders/vertex_shader.vert", "shaders/fragment_shader.frag");
    Shader ui_shader("shaders/ui_vShader.vert", "shaders/ui_fShader.frag");

    CreateGameUI();

    // 1. Gerar os Chunks dentro do World
    for (int i = 0; i < 8; i++)
    {
        // Usamos addChunk que organiza tudo no std::map
        world.addChunk(glm::ivec3(i * 16, 0, 0), shader);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    while (!window.shouldClose())
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (current_scene == 0)
        {
            // Passamos a referência do world aqui
            game_scene(shader, world, player, ui_shader, window.getInput(), window);
        }

        if (current_scene == 1)
        {
            menu_scene(ui_shader);
        }

        window.swapBuffers();
        window.pollEvents();
    }
}