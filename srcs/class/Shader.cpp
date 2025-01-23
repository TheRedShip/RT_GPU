/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:21:13 by ycontre           #+#    #+#             */
/*   Updated: 2025/01/19 18:58:42 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Shader.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *loadFileWithIncludes(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }

    std::stringstream fileContent;
    std::string line;

    while (std::getline(file, line))
	{
        if (line.rfind("#include", 0) == 0)
		{
            size_t start = line.find_first_of("\"<");
            size_t end = line.find_last_of("\">");
            if (start != std::string::npos && end != std::string::npos && end > start)
			{
                std::string includePath = line.substr(start + 1, end - start - 1);
                std::string includedContent = loadFileWithIncludes(includePath);
                fileContent << includedContent << "\n";
            }
        }
		else
            fileContent << line << "\n";
    }

    return strdup(fileContent.str().c_str());
}


void printWithLineNumbers(const char *str)
{
    if (!str)
        return;

    std::istringstream stream(str);
    std::string line;
    int lineNumber = 1;

    while (std::getline(stream, line))
        std::cout << lineNumber++ << ": " << line << std::endl;
}

Shader::Shader(std::string vertexPath, std::string fragmentPath, std::string computePath)
{
	const char *vertexCode = loadFileWithIncludes(vertexPath);
	const char *fragmentCode = loadFileWithIncludes(fragmentPath);
	const char *computeCode = loadFileWithIncludes(computePath);

	// printWithLineNumbers(computeCode);

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

	glGenTextures(1, &_output_texture);
	glBindTexture(GL_TEXTURE_2D, _output_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, _output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glGenTextures(1, &_accumulation_texture);
    glBindTexture(GL_TEXTURE_2D, _accumulation_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(1, _accumulation_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
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
	glBindTexture(GL_TEXTURE_2D, _output_texture);
	glUniform1i(glGetUniformLocation(_program, "screenTexture"), 0);
	
	glBindVertexArray(_screen_VAO);
	glDrawArrays(GL_TRIANGLES, 0, size * 3);
}

void	Shader::set_int(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(_program_compute, name.c_str()), value);
}
void	Shader::set_float(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(_program_compute, name.c_str()), value);
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