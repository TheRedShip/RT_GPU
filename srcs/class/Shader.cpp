/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:21:13 by ycontre           #+#    #+#             */
/*   Updated: 2024/10/14 19:52:40 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Shader.hpp"

char* load_file(char const* path)
{
	char* buffer = 0;
	long length = 0;
	FILE * f = fopen (path, "rb");

	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = (char*)malloc ((length+1)*sizeof(char));
		if (buffer)
		{
		fread (buffer, sizeof(char), length, f);
		}
		fclose (f);
	}
	else
		return (NULL);
	buffer[length] = '\0';

	return buffer;
}

Shader::Shader(std::string vertexPath, std::string fragmentPath, std::string computePath)
{
	const char *vertexCode = load_file(vertexPath.c_str());
	const char *fragmentCode = load_file(fragmentPath.c_str());
	const char *computeCode = load_file(computePath.c_str());
	
	_vertex = glCreateShader(GL_VERTEX_SHADER);
	
	glShaderSource(_vertex, 1, &vertexCode, NULL);
	glCompileShader(_vertex);

	checkCompileErrors(_vertex);
	
	_fragment = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(_fragment, 1, &fragmentCode, NULL);
	glCompileShader(_fragment);

	checkCompileErrors(_fragment);

	_compute = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(_compute, 1, &computeCode, NULL);
	glCompileShader(_compute);

	checkCompileErrors(_compute);
}

Shader::~Shader(void)
{
	glDeleteShader(_vertex);
	glDeleteShader(_fragment);
	glDeleteShader(_compute);
	glDeleteProgram(_program);
	glDeleteProgram(_program_compute);
}

void Shader::attach(void)
{
	_program = glCreateProgram();
	_program_compute = glCreateProgram();

	
	glAttachShader(_program, _vertex);
	glAttachShader(_program, _fragment);
	glAttachShader(_program_compute, _compute);

	glLinkProgram(_program);
	glLinkProgram(_program_compute);

	glGenTextures(1, &_outputTexture);
	glBindTexture(GL_TEXTURE_2D, _outputTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, _outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void Shader::checkCompileErrors(GLuint shader)
{
	GLint success;
	GLchar infoLog[512];
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void Shader::setupVertexBuffer(const Vertex* vertices, size_t size)
{
    glGenVertexArrays(1, &_screen_VAO);
    glGenBuffers(1, &_screen_VBO);
    
    glBindVertexArray(_screen_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _screen_VBO);
    glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void	Shader::drawTriangles(size_t size)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _outputTexture);
	glUniform1i(glGetUniformLocation(_program, "screenTexture"), 0);
	
	glBindVertexArray(_screen_VAO);
	glDrawArrays(GL_TRIANGLES, 0, size * 3);
}

void	Shader::set_int(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(_program_compute, name.c_str()), value);
}
void	Shader::set_vec2(const std::string &name, const glm::vec2 &value) const
{
	glUniform2fv(glGetUniformLocation(_program_compute, name.c_str()), 1, glm::value_ptr(value));
}
void	Shader::set_vec3(const std::string &name, const glm::vec3 &value) const
{
	glUniform3fv(glGetUniformLocation(_program_compute, name.c_str()), 1, glm::value_ptr(value));
}
void	Shader::set_mat4(const std::string &name, const glm::mat4 &value) const
{
	glUniformMatrix4fv(glGetUniformLocation(_program_compute, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

GLuint	Shader::getProgram(void) const
{
	return (_program);
}

GLuint	Shader::getProgramCompute(void) const
{
	return (_program_compute);
}