/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/06 18:02:18 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void					setupScreenTriangle(GLuint *VAO);
void					drawScreenTriangle(GLuint VAO, GLuint output_texture, GLuint program);
std::vector<GLuint>		generateTextures(unsigned int textures_count);
std::vector<Buffer *>	createDataOnGPU(Scene &scene);
void					updateDataOnGPU(Scene &scene, std::vector<Buffer *> buffers);

int main(int argc, char **argv)
{
	Arguments	args(argc, argv);
	if (args.error())
		return (1);

	Scene		scene(args.getSceneName());
	if (scene.fail())
		return (1);

	Window		window(&scene, WIDTH, HEIGHT, "RT_GPU", 0, args);

	GLuint VAO;
	setupScreenTriangle(&VAO);

	std::vector<GLuint> textures = generateTextures(2);
	GLuint output_texture = textures[0];

	ShaderProgram raytracing_program;
	Shader compute = Shader(GL_COMPUTE_SHADER, "shaders/compute.glsl");

	raytracing_program.attachShader(&compute);
	raytracing_program.link();

	raytracing_program.use();
	// raytracing_program.bindImageTexture(output_texture, 0, GL_READ_WRITE, GL_RGBA32F);
	// raytracing_program.bindImageTexture(textures[1], 1, GL_READ_WRITE, GL_RGBA32F);


	ShaderProgram render_program;
	Shader vertex = Shader(GL_VERTEX_SHADER, "shaders/vertex.vert");
	Shader frag = Shader(GL_FRAGMENT_SHADER, "shaders/frag.frag");

	render_program.attachShader(&vertex);
	render_program.attachShader(&frag);
	render_program.link();

	std::vector<Buffer *> buffers = createDataOnGPU(scene);

	while (!window.shouldClose())
	{
		window.updateDeltaTime();
		glClear(GL_COLOR_BUFFER_BIT);
		
		updateDataOnGPU(scene, buffers);
		
		raytracing_program.use();
		raytracing_program.set_int("u_frameCount", window.getFrameCount());
		raytracing_program.set_int("u_objectsNum", scene.getObjectData().size());
		raytracing_program.set_int("u_bvhNum", scene.getBvhData().size());
		raytracing_program.set_int("u_lightsNum", scene.getGPULights().size());
		raytracing_program.set_int("u_pixelisation", window.getPixelisation());
		raytracing_program.set_float("u_time", (float)(glfwGetTime()));
		raytracing_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		raytracing_program.dispathCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
		
		window.imGuiNewFrame();

		render_program.use();
		drawScreenTriangle(VAO, output_texture, render_program.getProgram());

		window.imGuiRender();

		window.display();
		window.pollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return (0);
}
