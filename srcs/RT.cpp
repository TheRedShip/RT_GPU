/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/18 16:23:30 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void					setupScreenTriangle(GLuint *VAO);
void					drawScreenTriangle(GLuint VAO, GLuint output_texture, GLuint program);

std::vector<GLuint>		generateTextures(unsigned int textures_count);

void					updateDataOnGPU(Scene &scene, std::vector<Buffer *> buffers);

void					shaderDenoise(ShaderProgram &denoising_program, GPUDenoise &denoise, std::vector<GLuint> textures);

int main(int argc, char **argv)
{
	Arguments	args(argc, argv);
	if (args.error())
		return (1);

	Scene		scene(args.getSceneName());
	if (scene.error())
		return (1);

	Window		window(&scene, WIDTH, HEIGHT, "RT_GPU", 0, args);

	GLuint VAO;
	setupScreenTriangle(&VAO);

	std::vector<GLuint> textures = generateTextures(8);
	
	ShaderProgram raytracing_program;
	Shader compute = Shader(GL_COMPUTE_SHADER, "shaders/compute.glsl");
	int maxTextureUnits; glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	std::cout << "Max texture units: " << maxTextureUnits << std::endl;
	// compute.setDefine("TEXTURE_MAX", std::to_string(maxTextureUnits / 2));
	// compute.reload();
	raytracing_program.attachShader(&compute);
	raytracing_program.link();

	ShaderProgram denoising_program;
	Shader denoise = Shader(GL_COMPUTE_SHADER, "shaders/denoising.glsl");
	denoising_program.attachShader(&denoise);
	denoising_program.link();

	ShaderProgram render_program;
	Shader vertex = Shader(GL_VERTEX_SHADER, "shaders/vertex.vert");
	Shader frag = Shader(GL_FRAGMENT_SHADER, "shaders/frag.frag");
	render_program.attachShader(&vertex);
	render_program.attachShader(&frag);
	render_program.link();

	std::vector<Buffer *> buffers = scene.createDataOnGPU();

	if (!scene.loadTextures())
		return (-1);

	while (!window.shouldClose())
	{
		window.clusterizerUpdate(textures, denoising_program, buffers);
		window.updateDeltaTime();
		
		updateDataOnGPU(scene, buffers);
		window.rendererUpdate(textures, denoising_program);
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		raytracing_program.use();
		raytracing_program.set_int("u_frameCount", window.getFrameCount());
		raytracing_program.set_int("u_objectsNum", scene.getObjectData().size());
		raytracing_program.set_int("u_bvhNum", scene.getBvhData().size());
		raytracing_program.set_int("u_lightsNum", scene.getGPULights().size());
		raytracing_program.set_int("u_pixelisation", window.getPixelisation());
		raytracing_program.set_float("u_time", (float)(glfwGetTime()));
		raytracing_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));

		window.reduceTimeFrame();

		std::map<std::string, std::vector<GLuint>> object_textures;
		object_textures["textures"] = scene.getTextureIDs();
		object_textures["emissive_textures"] = scene.getEmissionTextureIDs();
		raytracing_program.set_textures(object_textures);
		
		raytracing_program.dispathCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);

		if (scene.getDenoise().enabled && !window.isRendering())
			shaderDenoise(denoising_program, scene.getDenoise(), textures);

		window.imGuiNewFrame();

		render_program.use();
		drawScreenTriangle(VAO, textures[window.getOutputTexture()], render_program.getProgram());

		window.imGuiRender(raytracing_program, textures);

		window.display();
		window.pollEvents();

		// glClearTexImage(textures[3], 0, GL_RGBA, GL_FLOAT, nullptr);
		// glClearTexImage(textures[4], 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return (0);
}
