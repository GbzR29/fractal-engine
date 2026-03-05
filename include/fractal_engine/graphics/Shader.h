#pragma once

#include <third_party/glad/glad.h>
#include <string>
#include <third_party/glm/glm.hpp>

namespace fractal_engine::graphics {

class Shader {

	public:
		GLuint ID;
		Shader(const char* vertexPath, const char* fragmentPath);
		
		void use() const;

		// Uniform helpers
		void setBool(const std::string& name, bool value) const;
		void setInt(const std::string& name, int value) const;
		void setFloat(const std::string& name, float value) const;
		void setVec3(const std::string& name, const glm::vec3& value) const;
		void setMat4(const std::string& name, const glm::mat4& mat) const;

	private:
		std::string readFile(const char* path);
		void checkCompileErrors(GLuint shader, std::string type);
	
};

} // namespace fractal_engine::graphics