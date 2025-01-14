/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/13 17:40:45 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

int main(int argc, char **argv)
{
	Scene		scene;

	if (argc <= 1 || !scene.parseScene(argv[1]))
		return (1);

	Window		window(&scene, WIDTH, HEIGHT, "RT_GPU", 0);
	Shader		shader("shaders/vertex.vert", "shaders/frag.frag", "shaders/compute.glsl");

	GLint max_gpu_size;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_gpu_size);

	const std::vector<GPUObject> &object_data = scene.getObjectData();
	const std::vector<GPUMaterial> &material_data = scene.getMaterialData();

	std::cout << "Sending " << object_data.size() << " objects for " << \
				object_data.size() * sizeof(GPUObject) + material_data.size() * sizeof(GPUMaterial) \
				<< " / " << max_gpu_size << " bytes" << std::endl;

	GLuint objectSSBO;
    glGenBuffers(1, &objectSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * object_data.size(), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectSSBO);

	GLuint materialSSBO;
    glGenBuffers(1, &materialSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUMaterial) * material_data.size(), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialSSBO);

	GLuint cameraUBO;
	glGenBuffers(1, &cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUCamera), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);

	GLuint volumeUBO;
	glGenBuffers(1, &volumeUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, volumeUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUVolume), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, volumeUBO);

	shader.attach();

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	size_t size = sizeof(vertices) / sizeof(Vertex) / 3;
	shader.setupVertexBuffer(vertices, size);

	while (!window.shouldClose())
	{
		glUseProgram(shader.getProgramCompute());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, object_data.size() * sizeof(GPUObject), object_data.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material_data.size() * sizeof(GPUMaterial), material_data.data());

		GPUCamera camera_data = scene.getCamera()->getGPUData();
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUCamera), &camera_data);

		glBindBuffer(GL_UNIFORM_BUFFER, volumeUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUVolume), &scene.getVolume());

		shader.set_int("u_frameCount", window.getFrameCount());
		shader.set_int("u_objectsNum", object_data.size());
		shader.set_int("u_pixelisation", window.getPixelisation());
		shader.set_float("u_time", (float)(glfwGetTime()));
		shader.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		
		glDispatchCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		glClear(GL_COLOR_BUFFER_BIT);

		window.imGuiNewFrame();

		glUseProgram(shader.getProgram());
		shader.drawTriangles(size);

		window.imGuiRender();

		window.display();
		window.pollEvents();		
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	return (0);
}
