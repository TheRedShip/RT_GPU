/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 16:29:26 by tomoron           #+#    #+#             */
/*   Updated: 2025/01/23 19:41:40 by tomoron          ###   ########.fr       */
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
		void	update(void);

	private:
		void	addPoint(void);
		void	makeMovement(float timeFromStart, float curSplitTimeReset);

		int							_min;
		float						_sec;
		int							_samples;
		std::vector<t_pathPoint>	_path;
		Scene						*_scene;
		Window						*_win;

		int							_curPathIndex;
		int							_destPathIndex;
		double						_curSplitStart;
		int							_curSamples;
};

#endif
