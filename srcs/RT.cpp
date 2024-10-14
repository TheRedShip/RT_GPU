/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/14 19:54:42 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

int main(void)
{
	Window		window;
	Shader		shader("shaders/vertex.glsl", "shaders/frag.glsl");
	
	shader.attach();

	RT::Vec2f vertices[6] = {
			{ -1.0f, -1.0f }, { 1.0f, -1.0f }, { -1.0f, 1.0f },
			{ 1.0f, -1.0f }, { 1.0f,  1.0f }, { -1.0f, 1.0f }
	};

	shader.setupVertexBuffer(vertices, sizeof(vertices));

	while (!window.shouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		shader.setVec2f("u_resolution", RT::Vec2f(WIDTH, HEIGHT));

		glUseProgram(shader.getProgram());
		shader.drawTriangles();

		window.display();
		window.pollEvents();
	}
	
	return (0);
}