/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 18:10:10 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 20:57:49 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SHADER__HPP
# define RT_SHADER__HPP

# include "RT.hpp"

class Shader
{
	public:
		Shader(char *vertexPath, char *fragmentPath);
		Shader(Shader const &src);
		~Shader(void);

		Shader	&operator=(Shader const &rhs);

		// void	compile(const char *vertexSource, const char *fragmentSource);
		void	attach(void);

		// void	setBool(const std::string &name, bool value) const;
		// void	setInt(const std::string &name, int value) const;
		// void	setFloat(const std::string &name, float value) const;
		// void	setVec2(const std::string &name, const RT::Vec2f &value) const;
		// void	setVec3(const std::string &name, const RT::Vec3f &value) const;
		// void	setVec4(const std::string &name, const RT::Vec4f &value) const;
		// void	setMat4(const std::string &name, const RT::Mat4f &value) const;

		GLuint	getProgram(void) const;

	private:
		GLuint	_program;

		GLuint	_vertex;
		GLuint	_fragment;

		void	checkCompileErrors(unsigned int shader);
};

#endif