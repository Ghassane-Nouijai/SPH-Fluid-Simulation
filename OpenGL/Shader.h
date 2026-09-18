#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <unordered_map>

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	unsigned int m_RendererID;
	std::string m_FilePath;
	std::unordered_map<std::string, int> m_UniformLocationCache;
public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat2(const std::string& name, const glm::mat2& mat);
	void SetUniformMat3(const std::string& name, const glm::mat3& mat);
	void SetUniformMat4(const std::string& name, const glm::mat4& mat);
	void SetUniform1i(const std::string& name, int value);
	void SetUniformB(const std::string& name, bool value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform2fv(const std::string& name, const glm::vec2& value);
	void SetUniform2f(const std::string& name, float x, float y);
	void SetUniform3fv(const std::string& name, const glm::vec3& value);
	void SetUniform3f(const std::string& name, float x, float y, float z);
	void SetUniform4fv(const std::string& name, const glm::vec4& value);
private:
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int GetUniformLocation(const std::string& name);
};