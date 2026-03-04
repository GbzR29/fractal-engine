#define STB_IMAGE_IMPLEMENTATION

#include "Program.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "system/player/Player.h"
#include "system/shader/Shader.h"
#include "system/environment/Chunk.h"

/**
 * @brief Current active scene.
 */
static int current_scene = 0;

/**
 * @brief Frame timing variables.
 */
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

/**
 * @brief UI rendering buffers.
 */
static GLuint vao, vbo, ebo;

/**
 * @brief Creates the in-game UI geometry.
 */
void CreateGameUI()
{
    float vertex[] = {
        -0.05f, -0.05f, 0.0f,
         0.05f, -0.05f, 0.0f,
         0.05f,  0.05f, 0.0f,
        -0.05f,  0.05f, 0.0f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

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

/**
 * @brief Draws the UI elements.
 */
void drawGameUI(Shader &ui_shader)
{
    ui_shader.use();
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

/**
 * @brief Renders the main game scene.
 */
void game_scene(Shader &shader, std::vector<Chunk> &chunks, Player &player, Shader &ui_shader, Chunk &chunk, Input &input,
                Window &window)
{
    glClearColor(0.38f, 0.76f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    player.renderCamera(shader, 1080, 720);

    input.update(window.getNativeWindow());
    player.update(deltaTime, input);

    for (auto& c : chunks)
    {
        c.Draw(shader);
    }

    //chunk.Draw(shader);

    drawGameUI(ui_shader);
}

/**
 * @brief Renders the menu scene.
 */
void menu_scene(Shader &ui_shader)
{
    drawGameUI(ui_shader);
}

/**
 * @brief Initializes and runs the entire program.
 */
void Init_Program()
{
    Window window(1080, 720, "Fractal Engine");

    if (!window.init())
    {
        std::cerr << "Failed to initialize window system." << std::endl;
        return;
    }

    Player player;

    Shader shader("shaders/vertex_shader.vert", "shaders/fragment_shader.frag");

    Shader ui_shader("shaders/ui_vShader.vert", "shaders/ui_fShader.frag");

    CreateGameUI();

    Chunk chunk(glm::vec3(0.0f, 0.0f, 0.0f), shader);

    std::vector<Chunk> chunks;

    float x_pos = 0.0f;

    for (size_t i = 0; i < 8; i++)
    {
        chunks.emplace_back(glm::vec3(x_pos, 0.0f, 0.0f), shader);
        x_pos += 16.0f;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    while (!window.shouldClose())
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (current_scene == 0)
        {
            game_scene(shader, chunks, player, ui_shader, chunk, window.getInput(), window);
        }

        if (current_scene == 1)
        {
            menu_scene(ui_shader);
        }

        window.swapBuffers();
        window.pollEvents();
    }
}