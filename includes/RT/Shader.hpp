/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 18:10:10 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/13 19:10:11 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SHADER__HPP
# define RT_SHADER__HPP

# include "RT.hpp"

class Shader
{
	public:
		Shader(GLenum type, const std::string &file_path);
		~Shader(void);

		
		void	compile(void);
		void	reload();

		bool    hasChanged();
		
		void	setDefine(const std::string &name, const std::string &value);
		
		GLuint				getShader(void) const;
		const std::string	&getFilePath(void) const;

	private:
		void	checkCompileErrors();

		std::map<std::string, std::string>	_defines;

		GLenum						_type;
		GLuint						_shader_id;
		std::string					_file_path;

		std::unordered_map<std::string, std::filesystem::file_time_type> _files_timestamps;
};

#endif
