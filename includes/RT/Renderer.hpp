/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:29:26 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/22 19:34:22 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RENDERER_HPP
# define RENDERER_HPP

# include "RT.hpp"

class Scene;
class Window;

typedef struct s_pathPoint
{
	glm::vec3	pos;
	glm::vec2	dir;
	float		time;
}	t_pathPoint;

class Renderer
{
	public:
		Renderer(Scene *scene, Window *win);
		void	renderImgui(void);

	private:
		void	addPoint(void);

		int							_min;
		float						_sec;
		int							_samples;
		std::vector<t_pathPoint>	_path;
		Scene						*_scene;
		Window						*_win;
};

#endif
