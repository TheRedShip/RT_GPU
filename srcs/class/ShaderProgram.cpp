/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShaderProgram.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 22:21:46 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/13 19:16:44 by ycontre          ###   ########.fr       */
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

void	ShaderProgram::clearShaders()
{
	for (Shader *shader : _shaders)
		glDetachShader(_program, shader->getShader());
	
	_shaders.clear();
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

void	ShaderProgram::use()
{
	glUseProgram(_program);
	this->watchForChanges();
}

void	ShaderProgram::dispathCompute(GLuint x, GLuint y, GLuint z)
{
	this->use();
	glDispatchCompute(x, y, z);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void	ShaderProgram::bindImageTexture(GLuint texture_id, GLuint unit, GLenum access, GLenum format) const
{
	glBindImageTexture(unit, texture_id, 0, GL_FALSE, 0, access, format);
}

void	ShaderProgram::watchForChanges(void)
{
	for (Shader *shader : _shaders)
	{
		if (shader->hasChanged())
		{
			std::cout << "Shader " << shader->getFilePath() << " has changed" << std::endl;
			this->reloadShaders();
			break;
		}
	}
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

void	ShaderProgram::set_textures(std::map<std::string, std::vector<GLuint>> texture_ids)
{
	size_t	start_texture = 0;

	for (auto it = texture_ids.begin(); it != texture_ids.end(); it++)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			GLuint current_unit = start_texture + i;

			glActiveTexture(GL_TEXTURE0 + current_unit);
			glBindTexture(GL_TEXTURE_2D, it->second[i]);

			std::string uniform_name = it->first + "[" + std::to_string(i) + "]";
			// std::cout << "Loading texture " << uniform_name << " at unit " << i << std::endl;
			glUniform1i(glGetUniformLocation(_program, uniform_name.c_str()), current_unit);
		}
		start_texture = it->second.size();
	}
}

void	ShaderProgram::setDefine(const std::string &name, const std::string &value)
{
	for (Shader *shader : _shaders)
		shader->setDefine(name, value);
}

GLuint	ShaderProgram::getProgram(void) const
{
	return (_program);
}