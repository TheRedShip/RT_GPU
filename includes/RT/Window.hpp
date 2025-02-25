/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:15:41 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/25 01:50:04 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_WINDOW__HPP
# define RT_WINDOW__HPP

# include "RT.hpp"

class Scene;
class ShaderProgram;
class Clusterizer;

class Window
{
	public:
		Window(Scene *scene, int width, int height, const char *title, int sleep, Arguments &args);
		~Window(void);

		void		updateDeltaTime();
		void		display();
		void		pollEvents();
		bool		shouldClose();

		void		process_input();
		
		static void	keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static void	mouseMoveCallback(GLFWwindow *window, double xpos, double ypos);
		static void	mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

		void		imGuiNewFrame();
		void		imGuiRender(ShaderProgram &raytracing_program);

		GLFWwindow	*getWindow(void) const;
		float		getFps(void) const;
		int			getFrameCount(void) const;

		int			getPixelisation(void);

		bool		&getAccumulate(void);

		void		setFrameCount(int nb);
		bool		isRendering();

		void		rendererUpdate(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
		void		clusterizerUpdate(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
	private:
		GLFWwindow	*_window;
		Scene		*_scene;
		Renderer	*_renderer;
		Clusterizer *_clusterizer;
		
		float		_fps;
		float		_delta;
		int			_frameCount;
		int			_pixelisation;

		bool		accumulate = true;
};

#endif
