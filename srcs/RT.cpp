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

	// glm::vec2 vertices[6] = {
	// 		{ -1.0f, -1.0f }, { 1.0f, -1.0f }, { -1.0f, 1.0f },
	// 		{ 1.0f, -1.0f }, { 1.0f,  1.0f }, { -1.0f, 1.0f }
	// };
	glm::vec2 vertices[3] = {
		{-1.0f, -1.0f}, {3.0f, -1.0f}, {-1.0f,  3.0f}
	};
	size_t size = sizeof(vertices) / sizeof(glm::vec2) / 3;

	shader.setupVertexBuffer(vertices, size);
	
	while (!window.shouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glm::mat4 view = window.get_camera()->get_view_matrix();
    	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

		shader.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		shader.set_vec3("u_cameraPosition", window.get_camera()->get_position());
		shader.set_mat4("u_viewMatrix", view);
		shader.set_mat4("u_projectionMatrix", projection);

		glUseProgram(shader.getProgram());
		shader.drawTriangles(size);

		std::cout << "\rFPS: " << int(window.getFps()) << "        " << std::flush;

		window.display();
		window.pollEvents();
	}
	
	return (0);
}