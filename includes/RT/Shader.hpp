/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 18:10:10 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/14 19:51:46 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SHADER__HPP
# define RT_SHADER__HPP

# include "RT.hpp"

class Shader
{
	public:
		Shader(std::string vertexPath, std::string fragmentPath);
		Shader(Shader const &src);
		~Shader(void);

		Shader	&operator=(Shader const &rhs);

		// void	compile(const char *vertexSource, const char *fragmentSource);
		void	attach(void);
		void	setupVertexBuffer(const glm::vec2* vertices, size_t size);
		void	drawTriangles(size_t size);


		
		// void	setBool(const std::string &name, bool value) const;
		// void	setInt(const std::string &name, int value) const;
		// void	setFloat(const std::string &name, float value) const;
		void	set_vec2(const std::string &name, const glm::vec2 &value) const;
		void	set_vec3(const std::string &name, const glm::vec3 &value) const;
		// void	setVec4(const std::string &name, const RT::Vec4f &value) const;
		void	set_mat4(const std::string &name, const glm::mat4 &value) const;

		GLuint	getProgram(void) const;
		

	private:
		GLuint _screen_VAO, _screen_VBO;

		GLuint	_program;

		GLuint	_vertex;
		GLuint	_fragment;

		void	checkCompileErrors(unsigned int shader);
};

#endif