/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:16:24 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/09 16:34:48 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Window.hpp"

Window::Window(Scene *scene, int width, int height, const char *title, int sleep)
{
	_scene = scene;
	_frameCount = 0;
	
	if (!glfwInit())
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!_window)
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(-1);
	}
	
	glfwMakeContextCurrent(_window);
	glfwSetWindowUserPointer(_window, this);

	glfwSetKeyCallback(_window, keyCallback);
	glfwSetCursorPosCallback(_window, mouseMoveCallback);
	glfwSetMouseButtonCallback(_window, mouseButtonCallback);

	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(sleep);
}

Window::~Window(void)
{
	glfwTerminate();
}


void Window::process_input()
{

	bool forward = glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS;
	bool backward = glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS;
	bool left = glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS;
	bool right = glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS;
	bool up = glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS;
	bool down = glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

	if (forward || backward || left || right || up || down)
		_frameCount = 0;

	_scene->getCamera()->processKeyboard(forward, backward, left, right, up, down);
}

void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	(void) win; (void) xpos; (void) ypos;

	static double lastX = 0;
	static double lastY = 0;

	if (lastX == 0 && lastY == 0)
	{
		lastX = xpos;
		lastY = ypos;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
	{
		win->_scene->getCamera()->processMouse(xoffset, yoffset, true);
		win->_frameCount = 0;
	}

	lastX = xpos;
	lastY = ypos;
}
void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) button; (void) mods;
	
    if (action == GLFW_RELEASE)
	{
		win->_frameCount = 0;
    }
}
void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) key; (void) scancode; (void) action; (void) mods;
	
	if (key == 67 && action == GLFW_PRESS)
	{
		glm::vec3 pos = win->_scene->getCamera()->getPosition();
		glm::vec2 dir = win->_scene->getCamera()->getDirection();
		glm::vec2 dov = win->_scene->getCamera()->getDOV();

		std::cout << "\nCAM\t" << pos.x << " " << pos.y << " " << pos.z << "\t"
				<< dir.x << " " << dir.y << " " << "\t"
				<< dov.x << " " << dov.y << " " << "\t"
				<< std::endl;
	}
	if (action == GLFW_RELEASE)
		win->_frameCount = 0;
}

void Window::display()
{
	static double	lastTime = glfwGetTime();
	double			currentTime = glfwGetTime();
	
	_delta = currentTime - lastTime;

	lastTime = currentTime;
	_fps = 1.0f / _delta;

	_frameCount++;

    glfwSwapBuffers(_window);
}
void Window::pollEvents()
{
	this->process_input();
	_scene->getCamera()->update(_delta);
	
    glfwPollEvents();
}
bool Window::shouldClose()
{
    return glfwWindowShouldClose(_window);
}

GLFWwindow	*Window::getWindow(void) const
{
	return (_window);
}

float		Window::getFps(void) const
{
	return (_fps);
}

int			Window::getFrameCount(void) const
{
	return (_frameCount);
}

int			Window::getPixelisation(void)
{
	bool mouse = glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	bool movement = _scene->getCamera()->getVelocity() > 0.0f;

	if (mouse || movement)
		return (1);
	return (0);
}