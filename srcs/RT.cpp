/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/23 18:38:38 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"



int main(void)
{
	Window		window(WIDTH, HEIGHT, "RT_GPU", 0);
	Shader		shader("shaders/vertex.vert", "shaders/frag.frag", "shaders/compute.glsl");
	
	Vertex vertices[3] = {
		{{-1.0f, -1.0f}, {0.0f, 0.0f}},
		{{3.0f, -1.0f}, {2.0f, 0.0f}},
		{{-1.0f, 3.0f}, {0.0f, 2.0f}}
	};

	shader.attach();

	size_t size = sizeof(vertices) / sizeof(Vertex) / 3;

	shader.setupVertexBuffer(vertices, size);
	
	while (!window.shouldClose())
	{
		glUseProgram(shader.getProgramCompute());
		
		shader.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		shader.set_vec3("u_cameraPosition", window.getScene()->getCamera()->get_position());
		shader.set_mat4("u_viewMatrix", window.getScene()->getCamera()->get_view_matrix());
		
		glDispatchCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader.getProgram());
		
		shader.drawTriangles(size);

		std::cout << "\rFPS: " << int(window.getFps()) << "        " << std::flush;

		window.display();
		window.pollEvents();
	}
	
	return (0);
}