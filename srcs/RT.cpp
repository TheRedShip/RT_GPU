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
	Window		window(WIDTH, HEIGHT, "RT_GPU", 0);
	Shader		shader("shaders/vertex.vert", "shaders/frag.frag");
	
	shader.attach();

	// RT::Vec2f vertices[6] = {
	// 		{ -1.0f, -1.0f }, { 1.0f, -1.0f }, { -1.0f, 1.0f },
	// 		{ 1.0f, -1.0f }, { 1.0f,  1.0f }, { -1.0f, 1.0f }
	// };
	RT::Vec2f vertices[3] = {
		{-1.0f, -1.0f}, {3.0f, -1.0f}, {-1.0f,  3.0f}
	};
	size_t size = sizeof(vertices) / sizeof(RT::Vec2f) / 3;

	shader.setupVertexBuffer(vertices, size);
	
	while (!window.shouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		shader.setVec2f("u_resolution", RT::Vec2f(WIDTH, HEIGHT));
		glUseProgram(shader.getProgram());
		shader.drawTriangles(size);

		std::cout << "\rFPS: " << int(window.getFps()) << "        " << std::flush;

		window.display();
		window.pollEvents();
	}
	
	return (0);
}