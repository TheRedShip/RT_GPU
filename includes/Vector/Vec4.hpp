/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vec4.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 17:18:35 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 17:18:35 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_VEC4__HPP
# define RT_VEC4__HPP

# include "Vector/Vector.hpp"

namespace RT
{
	template<typename T>
	class Vec4 : public Vector<Vec4<T>, T, 4>
	{
		public:
			using Vector<Vec4<T>, T, 4>::Vector;
			
			constexpr Vec4(T r, T g, T b, T a)
			{
				this->data[0] = r;
				this->data[1] = g;
				this->data[2] = b;
				this->data[3] = a;
			}
	};

	using Vec4f = Vec4<float>;
	using Vec4d = Vec4<double>;
	using Vec4i = Vec4<int>;
}

#endif