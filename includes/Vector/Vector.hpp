/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vector.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 17:13:03 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 17:13:03 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_VECTOR__HPP
#define RT_VECTOR__HPP

#include <iostream>
#include <array>

namespace RT
{
	template<typename Derived, typename T, std::size_t N>
	class Vector
	{
		public:
			std::array<T, N> data;

			constexpr Vector() { data.fill(0); }

			constexpr Vector(std::initializer_list<T> values) { std::copy(values.begin(), values.end(), data.begin()); }

			constexpr T& operator[](std::size_t i) { return data[i]; }
			constexpr const T& operator[](std::size_t i) const { return data[i]; }

			constexpr Derived operator+(const Vector& vec) const
			{
				Derived result;
				for (std::size_t i = 0; i < N; ++i)
					result[i] = data[i] + vec[i];
				return result;
			}
			constexpr Derived operator-(const Vector& vec) const
			{
				Derived result;
				for (std::size_t i = 0; i < N; ++i)
					result[i] = data[i] - vec[i];
				return result;
			}
			constexpr Derived operator*(T scalar) const
			{
				Derived result;
				for (std::size_t i = 0; i < N; ++i)
					result[i] = data[i] * scalar;
				return result;
			}
			constexpr Derived operator/(T scalar) const
			{
				Derived result;
				for (std::size_t i = 0; i < N; ++i)
					result[i] = data[i] / scalar;
				return result;
			}

			constexpr Derived& operator+=(const Vector& vec)
			{
				*this = *this + vec;
				return static_cast<Derived&>(*this);
			}
			constexpr Derived& operator-=(const Vector& vec)
			{
				*this = *this - vec;
				return static_cast<Derived&>(*this);
			}
			constexpr Derived& operator*=(const Vector& vec)
			{
				*this = *this * vec;
				return static_cast<Derived&>(*this);
			}
			constexpr Derived& operator/=(const Vector& vec)
			{
				*this = *this / vec;
				return static_cast<Derived&>(*this);
			}

			constexpr Derived& operator*=(T scalar)
			{
				*this = *this * scalar;
				return static_cast<Derived&>(*this);
			}
			constexpr Derived& operator/=(T scalar)
			{
				*this = *this / scalar;
				return static_cast<Derived&>(*this);
			}

			void print() const
			{
				std::cout << "Vec" << N << "(";
				for (std::size_t i = 0; i < N; ++i) {
					std::cout << data[i];
					if (i < N - 1) std::cout << ", ";
				}
				std::cout << ")" << std::endl;
			}
	};
}

#include "Vector/Vec2.hpp"
#include "Vector/Vec3.hpp"
#include "Vector/Vec4.hpp"

#endif