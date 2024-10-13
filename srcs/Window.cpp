/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:16:24 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 20:52:00 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Window.hpp"

Window::Window(void)
{
	if (!glfwInit())
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow(WIDTH, HEIGHT, "RT_GPU", NULL, NULL);
	if (!_window )
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
	glfwSwapInterval(1);
}
Window::Window(Window const &src)
{
	*this = src;
}
Window	&Window::operator=(Window const &rhs)
{
	if (this != &rhs)
		_window = rhs._window;
	return (*this);
}
Window::~Window(void)
{
	glfwTerminate();
}


void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) key; (void) scancode; (void) mods;
	if (action == GLFW_PRESS)
	{
	
    }
}
void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) xpos; (void) ypos;

	win->_mousePos = RT::Vec2i(xpos, ypos);
}
void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) button; (void) mods;
	
    if (action == GLFW_PRESS)
	{

    }
}


void Window::display()
{
    glfwSwapBuffers(_window);
}
void Window::pollEvents()
{
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
RT::Vec2i	Window::getMousePos(void) const
{
	return (_mousePos);
}
