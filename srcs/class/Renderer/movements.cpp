/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   movements.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 16:01:59 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 16:02:05 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

glm::vec3 Renderer::hermiteInterpolate(glm::vec3 points[4], double alpha)
{
	double tension;
	double bias;
	glm::vec3 tang[2];
	double alphaSqr[2];
	glm::vec3 coef[4];
	
	tension = 0;
	bias = 0;

	alphaSqr[0] = alpha * alpha;
	alphaSqr[1] = alphaSqr[0] * alpha;

	tang[0]  = (points[1] - points[0]) * glm::vec3(1 + bias) * glm::vec3(1 - tension) / glm::vec3(2);
	tang[0] += (points[2] - points[1]) * glm::vec3(1 - bias) * glm::vec3(1 - tension) / glm::vec3(2);
	tang[1]  = (points[2] - points[1]) * glm::vec3(1 + bias) * glm::vec3(1 - tension) / glm::vec3(2);
	tang[1] += (points[3] - points[2]) * glm::vec3(1 - bias) * glm::vec3(1 - tension) / glm::vec3(2);

	coef[0] = glm::vec3(2 * alphaSqr[1] - 3 * alphaSqr[0] + 1);
	coef[1] = glm::vec3(alphaSqr[1] - 2 * alphaSqr[0] + alpha);
	coef[2] = glm::vec3(alphaSqr[1] -   alphaSqr[0]);
	coef[3] = glm::vec3(-2 * alphaSqr[1] + 3 * alphaSqr[0]);
	
	return(coef[0] * points[1] + coef[1] * tang[0] + coef[2] * tang[1] + coef[3] * points[2]);
}

glm::vec2 Renderer::bezierSphereInterpolate(glm::vec4 control, glm::vec2 from, glm::vec2 to, float time)
{
	glm::vec2 delta;
	glm::vec2 p1, p2;
	float t;

	p1 = glm::vec2(control.x, control.y);
	p2 = glm::vec2(control.z, control.w);
	t = time;
    for(int i = 0; i < 5; i++) {
        float currentX = 3.0f * ((1 - t) * (1 - t)) * t * p1.x + 3.0f * (1 - t) * (t * t) * p2.x + (t * t * t);
        
        if(abs(currentX - time) < 0.00001f) {
            break;
        }
        
        glm::vec2 derivative = glm::vec2(
	        3.0f * (1 - t) * (1 - t) * p1.x +
	        6.0f * (1.0f - t) * t * (p2.x - p1.x) +
	        3.0f * t * t * (1.0f - p2.x),
	        
	        3.0f * (1 - t) * (1 - t) * p1.y +
	        6.0f * (1.0f - t) * t * (p2.y - p1.y) +
	        3.0f * t * t * (1.0f - p2.y)
	    );
        
        if(abs(derivative.x) > 0.00001f){
            t = t - (currentX - time) / derivative.x;
            t = glm::clamp(t, 0.0f, 1.0f);
        }
    }
	t = 3.0f * ((1 - t) * (1 - t)) * t * p1.y + 3.0f * (1 - t) * (t * t) * p2.y + (t * t * t);

	delta = to - from;
	return(from + glm::vec2(delta.x * t, delta.y * t));
}

t_pathPoint Renderer::createNextPoint(t_pathPoint from, t_pathPoint to)
{
	t_pathPoint res;

	res.pos = to.pos + (to.pos - from.pos);
	res.dir = to.dir + (to.dir - from.dir);
	res.time = to.time + (to.time - from.time);
	return (res);
}

void Renderer::getInterpolationPoints(t_pathPoint &prev, t_pathPoint &from, t_pathPoint &to, t_pathPoint &next)
{
	from = _path[_curPathIndex];
	to = _path[_curPathIndex + 1];

	if(!_curPathIndex)
		prev = from;
	else if (_curPathIndex && _path[_curPathIndex - 1].time == _path[_curPathIndex].time)
		prev = createNextPoint(to, from);
	else
		prev = _path[_curPathIndex - 1];

	if((size_t)_curPathIndex + 2 >= _path.size())
		next = to;
	else if ((size_t)_curPathIndex + 2 < _path.size() && _path[_curPathIndex + 2].time == _path[_curPathIndex + 1].time)
		next = createNextPoint(from, to);
	else
		next = _path[_curPathIndex + 2];
}


void Renderer::makeMovement(float timeFromStart, float curSplitTimeReset)
{
	t_pathPoint		prev;
	t_pathPoint		from;
	t_pathPoint		to;
	t_pathPoint		next;

	float			pathTime;
	Camera			*cam;
	glm::vec3		pos;
	glm::vec2		dir;	
	float			normalTime;
	bool			smallDistPrev;
	bool			smallDistNext;
	glm::vec4		bezierControl;

	getInterpolationPoints(prev, from, to, next);

	cam = _scene->getCamera();
	pathTime = (to.time - from.time) * 60;
	normalTime = 1 - ((pathTime - timeFromStart) / pathTime);
	
	glm::vec3 points[4] = {prev.pos, from.pos, to.pos, next.pos};
	pos = hermiteInterpolate(points, normalTime);

	smallDistPrev = glm::distance((to.dir - from.dir) / glm::vec2(pathTime), (from.dir - prev.dir) / glm::vec2((from.time - prev.time) * 60)) < 40;
	smallDistNext = glm::distance((to.dir - from.dir) / glm::vec2(pathTime), (next.dir - to.dir) / glm::vec2((next.time - to.time) * 60)) < 40;
	bezierControl.x = 0.2f;
	bezierControl.y = !_curPathIndex || smallDistPrev ? .1f : .0f;
	bezierControl.z = 0.8f;
	bezierControl.w = (size_t)_curPathIndex + 2 >= _path.size() ||  smallDistNext ? .9f : 1.0f;

	dir = bezierSphereInterpolate(bezierControl, from.dir, to.dir, normalTime);
	if(std::isnan(dir.x) || std::isnan(dir.y))
		dir = from.dir;
	if(timeFromStart >= pathTime)
	{
		pos = to.pos;
		dir = to.dir;
		_curSplitStart = curSplitTimeReset;
		_curPathIndex++;
		while( _curPathIndex < _destPathIndex && _path[_curPathIndex].time == _path[_curPathIndex + 1].time)
		{
			_curPathIndex++;
			std::cout << "skip tp" << std::endl;
		}
	}
	cam->setPosition(pos);
	cam->setDirection(dir.x, dir.y);
	_win->setFrameCount(0);
	if(_curPathIndex == _destPathIndex)
	{
		_destPathIndex = 0;
		if(!_testMode)
			endRender();
	}
}
