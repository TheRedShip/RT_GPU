/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2024/10/13 20:58:06 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

int main(void)
{
	Window		window;
	Shader		shader("shaders/vertex.glsl", "shaders/frag.glsl");
	
	shader.attach();

	RT::Vec2f vertices[6] = {
			{ -0.5f, -1.0f }, { 1.0f, -1.0f }, { -1.0f, 1.0f },
			{ 1.0f, -1.0f }, { 1.0f,  1.0f }, { -1.0f, 1.0f }
	};

	unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind the VAO
    glBindVertexArray(VAO);

    // Bind the VBO and upload the vertex data to it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	while (!window.shouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader.getProgram());
		glBindVertexArray(VAO);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		window.display();
		window.pollEvents();
	}
	
	return (0);
}