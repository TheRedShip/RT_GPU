/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:00:38 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/15 14:00:38 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Camera.hpp"

Camera::Camera(void)
{
}

Camera::Camera(Camera const &src)
{
	*this = src;
}

Camera::~Camera(void)
{
}

Camera	&Camera::operator=(Camera const &rhs)
{
	if (this != &rhs)
	{
	}
	return (*this);
}