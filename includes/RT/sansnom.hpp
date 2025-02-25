/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sansnom.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 18:04:15 by ycontre           #+#    #+#             */
/*   Updated: 2025/02/19 18:19:47 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ashf
# define ashf

class RT
{
	public:
		RT(Arguments &args, Window &window);
		~RT();

	private:
		Window			&window;
		GLuint			VAO;
		std::vector<GLuint>	&textures;

		Scene			*scene;

		ShaderProgram		raytracing_program;
};

#endif
