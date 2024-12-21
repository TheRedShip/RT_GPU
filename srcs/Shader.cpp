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

Shader::Shader(std::string vertexPath, std::string fragmentPath)
{
	const char *vertexCode = load_file(vertexPath.c_str());
	const char *fragmentCode = load_file(fragmentPath.c_str());
	
	_vertex = glCreateShader(GL_VERTEX_SHADER);
	
	glShaderSource(_vertex, 1, &vertexCode, NULL);
	glCompileShader(_vertex);

	checkCompileErrors(_vertex);
	
	_fragment = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(_fragment, 1, &fragmentCode, NULL);
	glCompileShader(_fragment);

	checkCompileErrors(_fragment);
}

Shader::Shader(Shader const &src)
{
	*this = src;
}

Shader	&Shader::operator=(Shader const &rhs)
{
	if (this != &rhs)
	{
		_program = rhs._program;
		_vertex = rhs._vertex;
		_fragment = rhs._fragment;
	}
	return (*this);
}

Shader::~Shader(void)
{
	glDeleteShader(_vertex);
	glDeleteShader(_fragment);
	glDeleteProgram(_program);
}

void Shader::attach(void)
{
	_program = glCreateProgram();
	
	glAttachShader(_program, _vertex);
	glAttachShader(_program, _fragment);
	glLinkProgram(_program);
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

void Shader::setupVertexBuffer(const glm::vec2* vertices, size_t size)
{
	glGenVertexArrays(1, &_screen_VAO);
    glGenBuffers(1, &_screen_VBO);
	
    glBindVertexArray(_screen_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _screen_VBO);
    glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(glm::vec2), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void	Shader::drawTriangles(size_t size)
{
	glBindVertexArray(_screen_VAO);
	glDrawArrays(GL_TRIANGLES, 0, size * 3);
}


void	Shader::setVec2f(const std::string &name, const glm::vec2 &value) const
{
	glUniform2f(glGetUniformLocation(_program, name.c_str()), value[0], value[1]);
}

GLuint	Shader::getProgram(void) const
{
	return (_program);
}