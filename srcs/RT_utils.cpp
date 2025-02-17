/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT_utils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 23:21:09 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/12 23:21:09 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void				setupScreenTriangle(GLuint *VAO)
{
	GLuint VBO;

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	size_t size = sizeof(vertices) / sizeof(Vertex) / 3; // size 1

    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);
	
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void				drawScreenTriangle(GLuint VAO, GLuint output_texture, GLuint program)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, output_texture);
	glUniform1i(glGetUniformLocation(program, "screenTexture"), 0);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 1 * 3); // size 1
}

//0 output
//1 output_accumulation
//2 denoising
//3 normal
//4 position
//5 light
//6 light_accum
//7 color
std::vector<GLuint> generateTextures(unsigned int textures_count)
{
	std::vector<GLuint> textures(textures_count);

	glGenTextures(textures_count, textures.data());
	for (unsigned int i = 0; i < textures_count; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(i, textures[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}
	return (textures);
}

std::vector<Buffer *>	createDataOnGPU(Scene &scene)
{
	GLint max_gpu_size;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_gpu_size);

	const std::vector<GPUObject> &object_data = scene.getObjectData();
	const std::vector<GPUTriangle> &triangle_data = scene.getTriangleData();
	const std::vector<GPUBvh> &bvh_nodes = scene.getBvh();
	const std::vector<GPUBvhData> &bvh_data = scene.getBvhData();
	const std::vector<GPUMaterial> &material_data = scene.getMaterialData();

	std::cout << "Sending " << object_data.size() << " objects for " << \
				object_data.size() * sizeof(GPUObject) + \
				triangle_data.size() * sizeof(GPUTriangle) + \
				bvh_nodes.size() * sizeof(GPUBvh) + \
				material_data.size() * sizeof(GPUMaterial) \
				<< " / " << max_gpu_size << " bytes" << std::endl;

	std::vector<Buffer *> buffers;

	buffers.push_back(new Buffer(Buffer::Type::SSBO, 1, sizeof(GPUObject) * object_data.size(), object_data.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 2, sizeof(GPUTriangle) * triangle_data.size(), triangle_data.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 3, sizeof(GPUBvhData) * bvh_data.size(), bvh_data.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 4, sizeof(GPUBvh) * bvh_nodes.size(), bvh_nodes.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 5, sizeof(GPUMaterial) * material_data.size(), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 6, scene.getGPULights().size() * sizeof(int), nullptr));

	buffers.push_back(new Buffer(Buffer::Type::UBO, 0, sizeof(GPUCamera), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::UBO, 1, sizeof(GPUVolume), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::UBO, 2, sizeof(GPUDebug), nullptr));

	return (buffers);
}

void	updateDataOnGPU(Scene &scene, std::vector<Buffer *> buffers)
{
	const std::vector<GPUMaterial> &material_data = scene.getMaterialData();
	const std::set<int> &gpu_lights = scene.getGPULights();
	std::vector<int> gpu_lights_array(gpu_lights.begin(), gpu_lights.end());

	buffers[4]->update(material_data.data(), sizeof(GPUMaterial) * material_data.size());
	buffers[5]->update(gpu_lights_array.data(), gpu_lights.size() * sizeof(int));

	GPUCamera camera_data = scene.getCamera()->getGPUData();
	buffers[6]->update(&camera_data, sizeof(GPUCamera));

	buffers[7]->update(&scene.getVolume(), sizeof(GPUVolume));
	buffers[8]->update(&scene.getDebug(), sizeof(GPUDebug));
}


void	shaderDenoise(ShaderProgram &denoising_program, GPUDenoise &denoise, std::vector<GLuint> textures)
{
	denoising_program.use();

	denoising_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
	denoising_program.set_int("u_pass_count", denoise.pass);
	denoising_program.set_float("u_c_phi", denoise.c_phi);
	denoising_program.set_float("u_p_phi", denoise.p_phi);
	denoising_program.set_float("u_n_phi", denoise.n_phi);

	int output_texture = 5;
	int denoising_texture = 2;

	for (int pass = 0; pass < denoise.pass ; ++pass)
	{
		glBindImageTexture(5, textures[output_texture], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, textures[denoising_texture], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		
		denoising_program.set_int("u_pass", pass);
		denoising_program.dispathCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);

		std::swap(output_texture, denoising_texture);
	}

	glBindImageTexture(5, textures[output_texture], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}