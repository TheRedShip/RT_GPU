/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vec2.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 17:16:11 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 17:16:11 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_VEC2__HPP
#define RT_VEC2__HPP

# include "Vector/Vector.hpp"

namespace RT
{
	template<typename T>
	class Vec2 : public Vector<Vec2<T>, T, 2>
	{
		public:
			using Vector<Vec2<T>, T, 2>::Vector;

			constexpr Vec2(T x, T y)
			{
				this->data[0] = x;
				this->data[1] = y;
			}
	};

	using Vec2f = Vec2<float>;
	using Vec2d = Vec2<double>;
	using Vec2i = Vec2<int>;
}

#endif