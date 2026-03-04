#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

// ===============================
// Construtor
// ===============================
Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::cout << "\n=========== SHADER LOAD ===========\n";
    std::cout << "Vertex  : " << vertexPath << std::endl;
    std::cout << "Fragment: " << fragmentPath << std::endl;
    std::cout << "===================================\n";

    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    if (vertexCode.empty())
        std::cout << "ERRO: Vertex shader vazio ou não encontrado.\n";

    if (fragmentCode.empty())
        std::cout << "ERRO: Fragment shader vazio ou não encontrado.\n";

    const char* vShader = vertexCode.c_str();
    const char* fShader = fragmentCode.c_str();

    GLuint vertex, fragment;

    // ===============================
    // Compilar Vertex Shader
    // ===============================
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShader, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // ===============================
    // Compilar Fragment Shader
    // ===============================
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShader, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // ===============================
    // Criar Programa
    // ===============================
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// ===============================
// Usar shader
// ===============================
void Shader::use() const
{
    glUseProgram(ID);
}

// ===============================
// Ler arquivo shader
// ===============================
std::string Shader::readFile(const char* filepath)
{
    std::ifstream file(filepath);

    if (!file.is_open())
    {
        std::cout << "ERRO: Não foi possível abrir o shader: "
                  << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ===============================
// Verificação de erros
// ===============================
void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[4096];

    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader, 4096, nullptr, infoLog);

            std::cout << "\n========== SHADER ERROR ==========\n";
            std::cout << "Tipo : " << type << std::endl;
            std::cout << "----------------------------------\n";
            std::cout << infoLog << std::endl;
            std::cout << "==================================\n";
        }
        else
        {
            std::cout << "Shader compilado com sucesso: "
                      << type << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(shader, 4096, nullptr, infoLog);

            std::cout << "\n========== LINK ERROR ==========\n";
            std::cout << infoLog << std::endl;
            std::cout << "================================\n";
        }
        else
        {
            std::cout << "Shader program linkado com sucesso.\n";
        }
    }
}

// ===============================
// Uniform helpers
// ===============================
void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(ID, name.c_str()),
        1,
        GL_FALSE,
        &mat[0][0]
    );
}