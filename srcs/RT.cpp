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
	Window		window(WIDTH, HEIGHT, "RT_GPU", 1);
	Shader		shader("shaders/vertex.vert", "shaders/frag.frag", "shaders/compute.glsl");
	
	Material redMaterial = {glm::vec3(1.0f, 0.2f, 0.2f), 1.0, 1.0};
    float radius = 30.0f;
	for (int i = 0; i < 150; i++) {
		float angle = (2.0f * M_PI * i) / 150.0f;

		float x = radius * cos(angle);
		float z = radius * sin(angle);
		
		float y = 2.0f * sin(angle * 3.0f);
		
		glm::vec3 position(x, y, z);
		float sphereSize = 0.8f + 0.4f * sin(angle * 2.0f);
		
		window.getScene()->addObject(new Sphere(position, sphereSize, &redMaterial));
	}

	GLuint objectSSBO;
    glGenBuffers(1, &objectSSBO);

	shader.attach();

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	size_t size = sizeof(vertices) / sizeof(Vertex) / 3;

	shader.setupVertexBuffer(vertices, size);
	
	while (!window.shouldClose())
	{
		glUseProgram(shader.getProgramCompute());
		
		const std::vector<GPUObject> &gpu_data = window.getScene()->getGPUData();

		window.getScene()->updateGPUData();

		// Update SSBO with latest object data
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, gpu_data.size() * sizeof(GPUObject), gpu_data.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectSSBO);

		shader.set_int("u_objectsNum", gpu_data.size());
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