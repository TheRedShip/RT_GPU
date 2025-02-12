/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:21:13 by ycontre           #+#    #+#             */
/*   Updated: 2025/02/06 19:48:22 by ycontre          ###   ########.fr       */
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

void Shader::attach(void)
{
	_program = glCreateProgram();
	_program_compute = glCreateProgram();
	_program_denoising = glCreateProgram();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	glAttachShader(_program, _vertex);
	glAttachShader(_program, _fragment);

	glAttachShader(_program_compute, _compute);

	glAttachShader(_program_denoising, _denoising);

	glLinkProgram(_program);
	glLinkProgram(_program_compute);
	glLinkProgram(_program_denoising);

	glGenTextures(1, &_output_texture);
	glBindTexture(GL_TEXTURE_2D, _output_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, _output_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glGenTextures(1, &_accumulation_texture);
    glBindTexture(GL_TEXTURE_2D, _accumulation_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(1, _accumulation_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glGenTextures(1, &_denoising_texture);
    glBindTexture(GL_TEXTURE_2D, _denoising_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(2, _denoising_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glGenTextures(1, &_normal_texture);
    glBindTexture(GL_TEXTURE_2D, _normal_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(3, _normal_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glGenTextures(1, &_position_texture);
    glBindTexture(GL_TEXTURE_2D, _position_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(4, _position_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
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

void Shader::setupVertexBuffer()
{

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	_size = sizeof(vertices) / sizeof(Vertex) / 3;

    glGenVertexArrays(1, &_screen_VAO);
    glGenBuffers(1, &_screen_VBO);
    
    glBindVertexArray(_screen_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _screen_VBO);
    glBufferData(GL_ARRAY_BUFFER, _size * 3 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void	Shader::drawTriangles()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _output_texture);
	glUniform1i(glGetUniformLocation(_program, "screenTexture"), 0);
	
	glBindVertexArray(_screen_VAO);
	glDrawArrays(GL_TRIANGLES, 0, _size * 3);
}

void	Shader::flipOutputDenoising(bool pass)
{
	if (pass)
	{
		glBindImageTexture(0, _output_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);   
		glBindImageTexture(2, _denoising_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}
	else
	{
		glBindImageTexture(0, _denoising_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    	glBindImageTexture(2, _output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}
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

void	Shader::set_textures(std::vector<GLuint> texture_ids, std::vector<GLuint> emissive_texture_ids)
{
	for (size_t i = 0; i < texture_ids.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture_ids[i]);

		std::string uniform_name = "textures[" + std::to_string(i) + "]";
		// std::cout << "Loading texture " << uniform_name << " at unit " << i << std::endl;
		glUniform1i(glGetUniformLocation(_program_compute, uniform_name.c_str()), i);
	}

	size_t start_texture = texture_ids.size();

	for (size_t i = 0; i < emissive_texture_ids.size(); i++)
	{
		GLuint currentUnit = start_texture + i;

		glActiveTexture(GL_TEXTURE0 + currentUnit);
		glBindTexture(GL_TEXTURE_2D, emissive_texture_ids[i]);
		std::string uniform_name = "emissive_textures[" + std::to_string(i) + "]";
		// std::cout << "Loading emissive texture " << uniform_name << " (" << emissive_texture_ids[i] << ") at unit " << currentUnit << std::endl;
		glUniform1i(glGetUniformLocation(_program_compute, uniform_name.c_str()), currentUnit);
	}
}


GLuint	Shader::getShader(void) const
{
	return (_shader_id);
}

std::vector<float> Shader::getOutputImage(void)
{
	std::vector<float>	res(WIDTH * HEIGHT * 4);

	glBindTexture(GL_TEXTURE_2D, _output_texture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, res.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	return (res);
}	
