/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Buffer.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 23:45:57 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/12 23:45:57 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUFFER_HPP
# define BUFFER_HPP

# include "RT.hpp"

class Buffer
{
	public:
		enum Type
		{
			SSBO,
			UBO
		};
	
		Buffer(Type type, GLuint binding_point, GLuint size, const void *data)
			: _type(type), _binding_point(binding_point)
		{
			glGenBuffers(1, &_buffer_id);
			glBindBuffer(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, _buffer_id);
			glBufferData(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
			glBindBufferBase(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, _binding_point, _buffer_id);
			glBindBuffer(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, 0);
		}
	
		~Buffer() { glDeleteBuffers(1, &_buffer_id); }
	
		void update(const void *data, GLuint size)
		{
			glBindBuffer(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, _buffer_id);
			glBufferSubData(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, 0, size, data);
			glBindBuffer(_type == SSBO ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER, 0);
		}
	
		GLuint getID() const { return _buffer_id; }
	
	private:
		Type	_type;
		GLuint	_buffer_id;
		GLuint	_binding_point;
};

#endif