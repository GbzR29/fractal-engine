#define STB_IMAGE_IMPLEMENTATION

#include "../headers/Chunk.h"
#include "../headers/Player.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void drawGameUI(Shader &ui_shader);
void CreateGameUI();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

GLFWwindow* window;

Input input;
Player player;

const int win_width = 1080;
const int win_height = 720;

int current_scene = 0;

// ================ timing =================
// time between current frame and last frame
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{	
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){

        glfwSetWindowShouldClose(window, true);
    }  

	//testing a simple scene changer
	if(key == GLFW_KEY_Y && action == GLFW_PRESS){
        std::cout << current_scene << std::endl;

        if (current_scene == 0)
		{
			current_scene = 1;

		}else if(current_scene == 1){

			current_scene = 0;
		}
    }
}

void centerWindow(GLFWwindow* window, GLFWmonitor* monitor) {

    //Returns the monitor's current video mode, which includes the width and height of the resolution in use.
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);   
    int monitorX, monitorY;    

    //Retrieves the monitor's position in screen coordinates, which is important in setups with multiple monitors.
    glfwGetMonitorPos(monitor, &monitorX, &monitorY); 

    //Calculate the central position.
    int posX = monitorX + (mode->width - win_width) / 2;
    int posY = monitorY + (mode->height - win_height) / 2;

    glfwSetWindowPos(window, posX, posY);
}

//Here, all the game scene will be drawed
void game_scene(GLFWwindow* window, Shader &shader, std::vector<Chunk> &chunks, Player &player, Shader &ui_shader, Chunk &chunk){	

	glClearColor(0.38f, 0.76f, 0.9f, 1.0f);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	player.renderCamera(shader, win_width, win_height);

	input.update(window);
	player.update(deltaTime, input);

	for (auto& chunk : chunks)
	{
		//chunk.Draw(shader);
	}
	chunk.Draw(shader);

	drawGameUI(ui_shader);
}

void menu_scene(Shader &ui_shader){
	drawGameUI(ui_shader);
}

//Initialize GLFW window and GLAD
void initWindow(){

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(win_width, win_height, "Minecraft 2", NULL, NULL);

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    centerWindow(window, primaryMonitor);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

		std::cerr << "GLAD initialization failed." << std::endl;
	}

	glEnable(GL_DEPTH_TEST);
}

GLuint vao, vbo, ebo;

//simple crosshair 
void CreateGameUI(){

	float vertex[] = {
	   -0.05f, -0.05f,  0.0f, //bottom left
		0.05f, -0.05f,  0.0f, //bottom right
		0.05f,  0.05f,  0.0f, //top right
	   -0.05f,  0.05f,  0.0f, //top left
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

//draw simple crosshair
void drawGameUI(Shader &ui_shader){
	ui_shader.use();

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main() {
		
	initWindow();

	Shader shader("../src/shaders/vertex_shader.vert", "../src/shaders/fragment_shader.frag");
	Shader ui_shader("../src/shaders/ui_vShader.vert", "../src/shaders/ui_fShader.frag");

	CreateGameUI();

	Chunk chunk(glm::vec3(0.0f, 0.0f, 0.0f), shader);

	std::vector<Chunk> chunks;

	float x_pos = 0.0f;
	float z_pos = 0.0f;

	for (size_t i = 0; i < 8; i++)
	{
		chunks.emplace_back(glm::vec3(x_pos, 0.0f, 0.0f), shader);
		x_pos += 16.0f;
	}
	
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);	

	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (current_scene == 0)	{			
			game_scene(window, shader, chunks, player, ui_shader, chunk);				
		}	

		if(current_scene == 1) menu_scene(ui_shader);

		glfwSwapBuffers(window);
		glfwPollEvents();		
	}
		
	glfwTerminate();
	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    input.onMouseMove(xpos, ypos);
}