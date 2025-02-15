/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:16:24 by TheRed            #+#    #+#             */
/*   Updated: 2025/02/15 22:54:33 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Window.hpp"

void GLFWErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

Window::Window(Scene *scene, int width, int height, const char *title, int sleep, Arguments &args)
{
	_scene = scene;
	_fps = 0;
	_frameCount = 0;
	_pixelisation = 0;
	_renderer = new Renderer(scene, this, args);
	glfwSetErrorCallback(GLFWErrorCallback);
	if (!glfwInit())
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!_window)
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(-1);
	}
	
	glfwMakeContextCurrent(_window);
	glfwSetWindowUserPointer(_window, this);

	glfwSetKeyCallback(_window, keyCallback);
	glfwSetCursorPosCallback(_window, mouseMoveCallback);
	glfwSetMouseButtonCallback(_window, mouseButtonCallback);

	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(sleep);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

Window::~Window(void)
{
	glfwTerminate();
}


void Window::process_input()
{
	bool forward = glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS;
	bool backward = glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS;
	bool left = glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS;
	bool right = glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS;
	bool up = glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS;
	bool down = glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

	if(_renderer->rendering())
		return ;
	if (forward || backward || left || right || up || down)
		_frameCount = 0;

	_scene->getCamera()->processKeyboard(forward, backward, left, right, up, down);
}

void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	(void) win; (void) xpos; (void) ypos;

	static double lastX = 0;
	static double lastY = 0;

	if (lastX == 0 && lastY == 0)
	{
		lastX = xpos;
		lastY = ypos;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
	{
		win->_scene->getCamera()->processMouse(xoffset, yoffset, true);
		win->_frameCount = 0;
	}

	lastX = xpos;
	lastY = ypos;
}
void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) button; (void) mods;
	
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		win->_frameCount = 0;
}
void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) key; (void) scancode; (void) action; (void) mods;
	
	if (key == 67 && action == GLFW_PRESS)
	{
		glm::vec3 pos = win->_scene->getCamera()->getPosition();
		glm::vec2 dir = win->_scene->getCamera()->getDirection();
		float aperture = win->_scene->getCamera()->getAperture();
		float focus = win->_scene->getCamera()->getFocus();
		float fov = win->_scene->getCamera()->getFov();
		int	bounce = win->_scene->getCamera()->getBounce();

		std::cout << "\nCAM\t" << pos.x << " " << pos.y << " " << pos.z << "\t"
				<< dir.x << " " << dir.y << " " << "\t"
				<< aperture << " " << focus << " " << fov << "\t" << bounce
				<< std::endl;
	}
}

void Window::updateDeltaTime()
{
	static double	lastTime = glfwGetTime();
	double			currentTime = glfwGetTime();
	
	_delta = currentTime - lastTime;

	lastTime = currentTime;
	_fps = 1.0f / _delta;
}

void Window::display()
{
	if (accumulate)
		_frameCount++;

	if (_scene->getCamera()->getVelocity() > 0.0f)
		_frameCount = 0;

    glfwSwapBuffers(_window);
}

void Window::pollEvents()
{
	this->process_input();
	_scene->getCamera()->update(_scene, _delta);
	
    glfwPollEvents();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(_window) || _renderer->shouldClose();
}

bool Window::isRendering()
{
	return (_renderer->rendering());
}

void		Window::rendererUpdate(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram)
{
	_renderer->update(textures, denoisingProgram);
}

void Window::imGuiNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Window::imGuiRender(ShaderProgram &raytracing_program)
{
	bool has_changed = false;
	
	ImGui::Begin("Settings");

	ImGui::Text("Fps: %d", int(_fps));
	ImGui::Text("Frame: %d", _frameCount);
	ImGui::Text("Objects: %lu", _scene->getObjectData().size() + _scene->getTriangleData().size());
	
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Camera"))
	{

		if (ImGui::Checkbox("Accumulate", &accumulate))
			_frameCount = 0;

		has_changed |= ImGui::SliderInt("Bounce", &_scene->getCamera()->getBounce(), 0, 20);
		has_changed |= ImGui::SliderFloat("FOV", &_scene->getCamera()->getFov(), 1.0f, 180.0f);
		has_changed |= ImGui::SliderFloat("Aperture", &_scene->getCamera()->getAperture(), 0.0f, 1.0f);
		has_changed |= ImGui::SliderFloat("Focus", &_scene->getCamera()->getFocus(), 0.0f, 150.0f);
	}


	if (ImGui::CollapsingHeader("Material"))
	{

		ImGui::BeginChild("Header", ImVec2(0, 400), true, 0);

		for (unsigned int i = 0; i < _scene->getMaterialData().size(); i++)
		{
			GPUMaterial &mat = _scene->getMaterialData()[i];

			ImGui::PushID(i);
			
			ImGui::Text("Material %d", i);
			has_changed |= ImGui::ColorEdit3("Color", &mat.color[0]);
			if (ImGui::SliderFloat("Emission", &mat.emission, 0.0f, 10.0f))
			{
				has_changed = 1;
				_scene->updateLightAndObjects(i);
			}
			
			if (mat.type == 0)
			{
				has_changed |= ImGui::SliderFloat("Roughness", &mat.roughness, 0.0f, 1.0f);
				has_changed |= ImGui::SliderFloat("Metallic", &mat.metallic, 0.0f, 1.0f);
			}
			else if (mat.type == 1)
				has_changed |= ImGui::SliderFloat("Refraction", &mat.refraction, 1.0f, 5.0f);
			else if (mat.type == 2)
			{
				has_changed |= ImGui::SliderFloat("Transparency", &mat.roughness, 0.0f, 1.0f);
				has_changed |= ImGui::SliderFloat("Refraction", &mat.refraction, 1.0f, 2.0f);
				has_changed |= ImGui::SliderFloat("Proba", &mat.metallic, 0., 1.);
			}
			has_changed |= ImGui::SliderInt("Type", &mat.type, 0, 2);

			ImGui::PopID();

			ImGui::Separator();
		}
		ImGui::EndChild();

	}
	
	if (ImGui::CollapsingHeader("Fog"))
	{
		if (ImGui::Checkbox("Enable##0", (bool *)(&_scene->getVolume().enabled)))
		{
			raytracing_program.set_define("FOG", std::to_string(_scene->getVolume().enabled));
			raytracing_program.reloadShaders();
			has_changed = true;
		}
		ImGui::Separator();
		
		if (ImGui::SliderFloat("Absorption", &_scene->getVolume().sigma_a.x, 0., 0.1))
		{
			_scene->getVolume().sigma_a = glm::vec3(_scene->getVolume().sigma_a.x);
			_scene->getVolume().sigma_t = _scene->getVolume().sigma_a + _scene->getVolume().sigma_s;
			has_changed = true;
		}
		if (ImGui::SliderFloat("Scattering", &_scene->getVolume().sigma_s.x, 0., 0.5))
		{
			_scene->getVolume().sigma_s = glm::vec3(_scene->getVolume().sigma_s.x);
			_scene->getVolume().sigma_t = _scene->getVolume().sigma_a + _scene->getVolume().sigma_s;
			has_changed = true;
		}
		if (ImGui::SliderFloat("G", &_scene->getVolume().g, 0., 1.))
			has_changed = true;
	}

	if (ImGui::CollapsingHeader("Denoiser"))
	{
		ImGui::Checkbox("Enable##1", (bool *)(&_scene->getDenoise().enabled));
		ImGui::Separator();
		if (ImGui::SliderInt("Pass", &_scene->getDenoise().pass, 0, 8))
			_scene->getDenoise().pass = (_scene->getDenoise().pass / 2) * 2; // make sure it's even
		
		ImGui::SliderFloat("Color diff", &_scene->getDenoise().c_phi, 0.0f, 1.0f);
		ImGui::SliderFloat("Position diff", &_scene->getDenoise().p_phi, 0.0f, 1.0f);
		ImGui::SliderFloat("Normal diff", &_scene->getDenoise().n_phi, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Debug"))
	{
		if (ImGui::Checkbox("Enable##2", (bool *)(&_scene->getDebug().enabled)))
		{
			raytracing_program.set_define("DEBUG", std::to_string(_scene->getDebug().enabled));
			raytracing_program.reloadShaders();
			has_changed = true;
		}
		ImGui::Separator();
		has_changed |= ImGui::SliderInt("Debug mode", &_scene->getDebug().mode, 0, 2);
		has_changed |= ImGui::SliderInt("Box treshold", &_scene->getDebug().box_treshold, 1, 2000);
		has_changed |= ImGui::SliderInt("Triangle treshold", &_scene->getDebug().triangle_treshold, 1, 2000);
	}


	_renderer->renderImgui();
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	if (has_changed)
		_frameCount = (accumulate == 0) - 1;
}

GLFWwindow	*Window::getWindow(void) const
{
	return (_window);
}

float		Window::getFps(void) const
{
	return (_fps);
}

int			Window::getFrameCount(void) const
{
	return (_frameCount);
}

void		Window::setFrameCount(int nb)
{
	_frameCount = nb;
}

bool		&Window::getAccumulate(void)
{
	return (accumulate);
}

int			Window::getPixelisation(void)
{
	bool mouse = glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	bool movement = _scene->getCamera()->getVelocity() > 0.0f;

	if (mouse || movement)
	{
		if(_fps < 30 && _pixelisation < 16)	
			_pixelisation++;
		if(_fps > 60 && _pixelisation > 0)
			_pixelisation--;
	}
	else if(_pixelisation)
		_pixelisation = 0;
	return (_pixelisation + 1);
}
