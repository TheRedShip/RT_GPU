/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/18 21:10:23 by ycontre          ###   ########.fr       */
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
	// Shader		shader("shaders/vertex.vert", "shaders/frag.frag", "shaders/debug.glsl");

	GLint max_gpu_size;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_gpu_size);

	const std::vector<GPUObject> &object_data = scene.getObjectData();
	const std::vector<GPUTriangle> &triangle_data = scene.getTriangleData();

	const std::vector<GPUMaterial> &material_data = scene.getMaterialData();

	std::cout << "Sending " << object_data.size() << " objects for " << \
				object_data.size() * sizeof(GPUObject) + \
				triangle_data.size() * sizeof(GPUTriangle) + \
				material_data.size() * sizeof(GPUMaterial) \
				<< " / " << max_gpu_size << " bytes" << std::endl;

	GLuint objectSSBO;
    glGenBuffers(1, &objectSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * object_data.size(), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectSSBO);

	GLuint trianglesSSBO;
    glGenBuffers(1, &trianglesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, trianglesSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUTriangle) * triangle_data.size(), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, trianglesSSBO);

	GLuint materialSSBO;
    glGenBuffers(1, &materialSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUMaterial) * material_data.size(), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, materialSSBO);
	
	GLuint lightSSBO;
	glGenBuffers(1, &lightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, scene.getGPULights().size() * sizeof(int), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightSSBO);

	GLuint bvhSSBO;
	glGenBuffers(1, &bvhSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, scene.getBVH().size() * sizeof(GPUBvh), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bvhSSBO);


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

	GLuint debugUBO;
	glGenBuffers(1, &debugUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, debugUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUDebug), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, debugUBO);


	shader.attach();

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	size_t size = sizeof(vertices) / sizeof(Vertex) / 3;
	shader.setupVertexBuffer(vertices, size);

	std::vector<int>	recorded_fps;

	while (!window.shouldClose())
	{

		glUseProgram(shader.getProgramCompute());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, object_data.size() * sizeof(GPUObject), object_data.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, trianglesSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, triangle_data.size() * sizeof(GPUTriangle), triangle_data.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material_data.size() * sizeof(GPUMaterial), material_data.data());

		std::set<int> gpu_lights = scene.getGPULights();
		std::vector<int> gpu_lights_array(gpu_lights.begin(), gpu_lights.end());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gpu_lights_array.size() * sizeof(int), gpu_lights_array.data());
		
		std::vector<GPUBvh> gpu_bvh = scene.getBVH();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gpu_bvh.size() * sizeof(GPUBvh), gpu_bvh.data());
		

		Camera *camera = scene.getCamera();

		// performance profiling
		if (false)
		{
			float time = (float)(glfwGetTime()) ;

			recorded_fps.push_back((int)window.getFps());

			float y_offset = 0;
			float dist_to_obj = 2;
			float speed = 0.5;

			camera->setPosition(glm::vec3(
								cos((time + 6.28) * speed) * dist_to_obj,
								y_offset,
								sin((time + 6.28) * speed) * dist_to_obj
								));

			glm::vec3 direction = glm::normalize(camera->getPosition());
			float yaw = glm::degrees(atan2(direction.z, direction.x));
			
			if ((int)yaw == 179)
				break;

			camera->setDirection(0, yaw - 180);
			camera->updateCameraVectors();
		}
		//

		GPUCamera camera_data = camera->getGPUData();
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUCamera), &camera_data);

		glBindBuffer(GL_UNIFORM_BUFFER, volumeUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUVolume), &scene.getVolume());

		glBindBuffer(GL_UNIFORM_BUFFER, debugUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUDebug), &scene.getDebug());

		shader.set_int("u_frameCount", window.getFrameCount());
		shader.set_int("u_objectsNum", object_data.size());
		shader.set_int("u_bvhNum", gpu_bvh.size());
		shader.set_int("u_lightsNum", gpu_lights.size());
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
	
	// performance profiling
	std::ofstream file("fps.txt");
	for (int i = 0; i < (int) recorded_fps.size(); i++)
		file << recorded_fps[i] << std::endl;
	file.close();
	//

	return (0);
}
