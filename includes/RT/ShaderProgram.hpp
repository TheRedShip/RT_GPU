/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShaderProgram.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 22:22:17 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/13 19:10:15 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHADERPROGRAM_HPP
# define SHADERPROGRAM_HPP

# include "RT.hpp"

class ShaderProgram
{
	public:
		ShaderProgram();
		~ShaderProgram(void);

		void	attachShader(Shader *shader);
		void	clearShaders();
		
		void	link(void);
		
		void	use(void);
		void	dispathCompute(GLuint x, GLuint y, GLuint z);
		
		void	bindImageTexture(GLuint texture_id, GLuint unit, GLenum access, GLenum format) const;

		void	watchForChanges(void);
		void	reloadShaders(void);

		void	set_int(const std::string &name, int value) const;
		void	set_float(const std::string &name, float value) const;
		void	set_vec2(const std::string &name, const glm::vec2 &value) const;

		void	set_textures(std::map<std::string, std::vector<GLuint>> texture_ids);

		void	set_define(const std::string &name, const std::string &value);

		GLuint	getProgram(void) const;

	private:
		std::vector<Shader *>	_shaders;
		GLuint					_program;
};

#endif
