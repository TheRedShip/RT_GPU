/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:52:10 by TheRed            #+#    #+#             */
/*   Updated: 2025/01/08 17:44:56 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT__HPP
# define RT__HPP

# define WIDTH 1920
# define HEIGHT 1080

#define GLM_ENABLE_EXPERIMENTAL

# include "glm/glm.hpp"
# include "glm/gtx/string_cast.hpp"
# include "glm/gtc/matrix_transform.hpp"
# include "glm/gtc/type_ptr.hpp"
# include "glm/gtx/euler_angles.hpp"

# include "glad/gl.h"
# include "GLFW/glfw3.h"

# include <iostream>
# include <fstream>
# include <sstream>
# include <vector>
# include <string>
# include <memory>
# include <map>

struct Vertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

# include "Object.hpp"
# include "objects/Sphere.hpp"
# include "objects/Plane.hpp"
# include "objects/Quad.hpp"
# include "objects/Triangle.hpp"
# include "objects/Cube.hpp"
# include "objects/Portal.hpp"
# include "objects/Cylinder.hpp"

# include "Camera.hpp"
# include "Window.hpp"
# include "Shader.hpp"
# include "Scene.hpp"
# include "SceneParser.hpp"



#endif