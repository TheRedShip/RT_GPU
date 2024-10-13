/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 18:10:10 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 18:10:10 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SHADER__HPP
# define RT_SHADER__HPP

# include "RT.hpp"

class Shader
{
	public:
		Shader(void);
		Shader(Shader const &src);
		~Shader(void);

		Shader	&operator=(Shader const &rhs);

		// void	compile(const char *vertexSource, const char *fragmentSource);
		// void	use(void) const;

		// void	setBool(const std::string &name, bool value) const;
		// void	setInt(const std::string &name, int value) const;
		// void	setFloat(const std::string &name, float value) const;
		// void	setVec2(const std::string &name, const RT::Vec2f &value) const;
		// void	setVec3(const std::string &name, const RT::Vec3f &value) const;
		// void	setVec4(const std::string &name, const RT::Vec4f &value) const;
		// void	setMat4(const std::string &name, const RT::Mat4f &value) const;

	private:
		unsigned int	_id;

		// void	checkCompileErrors(unsigned int shader, std::string type);
};