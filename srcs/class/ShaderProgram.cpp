/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShaderProgram.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 22:21:46 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/12 22:21:46 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram()
{
	_program = glCreateProgram();
}

ShaderProgram::~ShaderProgram(void)
{
	glDeleteProgram(_program);
}

void	ShaderProgram::attachShader(Shader *shader)
{
	_shaders.push_back(shader);
	glAttachShader(_program, shader->getShader());
}

void	ShaderProgram::link()
{
	glLinkProgram(_program);

	GLint success;
	glGetProgramiv(_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(_program, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
}

void	ShaderProgram::use() const
{
	glUseProgram(_program);
}

void	ShaderProgram::dispathCompute(GLuint x, GLuint y, GLuint z) const
{
	this->use();
	glDispatchCompute(x, y, z);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void	ShaderProgram::bindImageTexture(GLuint texture_id, GLuint unit, GLenum access, GLenum format) const
{
	glBindImageTexture(unit, texture_id, 0, GL_FALSE, 0, access, format);
}

void	ShaderProgram::reloadShaders(void)
{
	std::cout << "Reloading shaders" << std::endl;

	for (Shader *shader : _shaders)
	{
		glDetachShader(_program, shader->getShader());
		shader->reload();
		glAttachShader(_program, shader->getShader());
	}

	this->link();
}

void	ShaderProgram::set_int(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(_program, name.c_str()), value);
}
void	ShaderProgram::set_float(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(_program, name.c_str()), value);
}
void	ShaderProgram::set_vec2(const std::string &name, const glm::vec2 &value) const
{
	glUniform2fv(glGetUniformLocation(_program, name.c_str()), 1, glm::value_ptr(value));
}

GLuint	ShaderProgram::getProgram(void) const
{
	return (_program);
}