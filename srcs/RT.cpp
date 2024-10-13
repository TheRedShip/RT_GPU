/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 20:58:06 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

int main(void)
{
	Window		window;

	Shader		shader("shaders/vertex.glsl", "shaders/frag.glsl");

	shader.attach();

	

	while (!window.shouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader.getProgram());

		window.display();
		window.pollEvents();
	}
	
	return (0);
}