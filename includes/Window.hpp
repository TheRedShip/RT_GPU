/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:15:41 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 20:48:46 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_WINDOW__HPP
# define RT_WINDOW__HPP

# include "RT.hpp"

class Window
{
	public:
		Window(void);
		Window(Window const &src);
		~Window(void);

		Window	&operator=(Window const &rhs);

		GLFWwindow	*getWindow(void) const;
		RT::Vec2i	getMousePos(void) const;


		void		display();
		void		pollEvents();
		bool		shouldClose();
		
		static void	keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static void	mouseMoveCallback(GLFWwindow *window, double xpos, double ypos);
		static void	mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	
	private:
		GLFWwindow	*_window;
		RT::Vec2i	_mousePos;

};

#endif