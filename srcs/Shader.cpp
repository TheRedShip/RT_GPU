/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:21:13 by ycontre           #+#    #+#             */
/*   Updated: 2024/10/13 20:58:02 by ycontre          ###   ########.fr       */
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

GLuint	Shader::getProgram(void) const
{
	return (_program);
}