/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:21:13 by ycontre           #+#    #+#             */
/*   Updated: 2025/02/13 18:59:18 by ycontre          ###   ########.fr       */
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

Shader::Shader(GLenum type, const std::string &file_path)
{
	_type = type;
	_file_path = file_path;
	_shader_id = 0;

	this->compile();
}

Shader::~Shader(void)
{
}

void	Shader::compile()
{
	_shader_id = glCreateShader(_type);
	
	const char *shader_code = loadFileWithIncludes(_file_path);
	// printWithLineNumbers(shader_code);
	
	glShaderSource(_shader_id, 1, &shader_code, NULL);
	glCompileShader(_shader_id);

	this->checkCompileErrors();
}

void Shader::reload()
{
	glDeleteShader(_shader_id);
	this->compile();
}

void Shader::checkCompileErrors()
{
	GLint success;
	GLchar infoLog[512];
	
	glGetShaderiv(_shader_id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(_shader_id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

GLuint	Shader::getShader(void) const
{
	return (_shader_id);
}

