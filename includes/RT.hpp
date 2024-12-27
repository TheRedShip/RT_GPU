/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:52:10 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/23 19:04:53 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT__HPP
# define RT__HPP

# define WIDTH 1920
# define HEIGHT 1080

# include "glm/glm.hpp"
# include "glm/gtc/matrix_transform.hpp"
# include "glm/gtc/type_ptr.hpp"

# include "glad/gl.h"
# include "GLFW/glfw3.h"

# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <memory>
# include <map>

struct Vertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

# include "Object.hpp"
# include "objects/Sphere.hpp"

# include "Camera.hpp"
# include "Window.hpp"
# include "Shader.hpp"
# include "Scene.hpp"
# include "SceneParser.hpp"



#endif