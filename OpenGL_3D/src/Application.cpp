#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 540, "OpenGL Stuff", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Shader shader("res/shaders/Basic.shader");

		shader.Bind();
		int samplers[32];
		for (int i = 0; i < 32; i++)
			samplers[i] = i;
		shader.SetUniform1iv("u_Textures", 32, samplers);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		Renderer::Init();

		Texture texture1("res/textures/hero_dash_icon.png");
		Texture texture2("res/textures/shield_with_cross_icon.png");

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT);
			shader.Bind();
			glm::mat4 viewProj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
			shader.SetUniformMat4f("u_ViewProj", viewProj);
			shader.SetUniformMat4f("u_Transform", transform);

			Renderer::ResetStats();
			Renderer::BeginBatch();

			float scale = 50.0f;
			for (float y = -10.f; y < 10.0f; y += 0.25f)
			{
				for (float x = -10.f; x < 10.0f; x += 0.25f)
				{
					glm::vec4 color = { (x + 10) / 20.0f, 0.2f, (y + 10) / 20.0f, 1.0f };
					Renderer::DrawQuad({ x*scale, y*scale }, { 0.25f*scale, 0.25f*scale }, color);
				}
			}

			for (int y = 0; y < 5; y++)
			{
				for (int x = 0; x < 5; x++)
				{
					GLuint tex = (x + y) % 2 == 0 ? texture1.GetId() : texture2.GetId();
					Renderer::DrawQuad({ x*scale, y*scale }, { 1.0f*scale, 1.0f*scale }, tex);
				}
			}

			Renderer::EndBatch();

			Renderer::Flush();

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
		
		Renderer::Shutdown();
	}
	glfwTerminate();
	return 0;
}