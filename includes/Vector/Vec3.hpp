/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vec3.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 17:17:04 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 20:32:13 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_VEC3__HPP
# define RT_VEC3__HPP

# include "Vector/Vector.hpp"
# include <cmath>

namespace RT
{
	template<typename T>
	class Vec3 : public Vector<Vec3<T>, T, 3>
	{
		public:
			using Vector<Vec3<T>, T, 3>::Vector;

			constexpr Vec3(T x, T y, T z)
			{
				this->data[0] = x;
				this->data[1] = y;
				this->data[2] = z;
			}
			
			constexpr T Distance(const Vec3& other) const
			{
				T dx = this->data[0] - other.data[0];
				T dy = this->data[1] - other.data[1];
				T dz = this->data[2] - other.data[2];

				return std::sqrt(dx * dx + dy * dy + dz * dz);
			}
	};

	using Vec3f = Vec3<float>;
	using Vec3d = Vec3<double>;
	using Vec3i = Vec3<int>;
}

#endif