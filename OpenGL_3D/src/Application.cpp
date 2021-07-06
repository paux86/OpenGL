#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

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

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		Shader shader("res/shaders/Basic.shader");

		shader.Bind();
		int samplers[32];
		for (int i = 0; i < 32; i++)
			samplers[i] = i;
		shader.SetUniform1iv("u_Textures", 32, samplers);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		Renderer::Init();

		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, true);
		ImGui::StyleColorsDark();

		Texture texture1("res/textures/hero_dash_icon.png");
		Texture texture2("res/textures/shield_with_cross_icon.png");

		float zoom = 0.0f;
		glm::vec3 camera(0, 0, 0);

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ImGui_ImplGlfwGL3_NewFrame();
			ImGui::Begin("Test");

			shader.Bind();
			
			ImGui::SliderFloat3("Camera", &camera.x, -1000.0f, 1000.0f);

			glm::mat4 viewProj;
			const float radius = 100.0f;
			float camX = sin(glfwGetTime()) * radius;
			float camZ = cos(glfwGetTime()) * radius;
			glm::mat4 view;
			view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			viewProj = glm::perspectiveFov(90.0f, 960.0f, 540.0f, 0.1f, 1000.0f) * view;
			
			shader.SetUniformMat4f("u_ViewProj", viewProj);

			Renderer::ResetStats();
			Renderer::BeginBatch();

			/*
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
			*/

			glm::vec3 boxPosition = { 0, 0, 0 };
			glm::vec3 boxDimensions = { 50, 50, 50 };
			glm::vec4 boxColor = { 0.1f, 0.2f, 0.8f, 1.0f };
			glm::vec3 boxFacing = { 0, 0, 1 };
			Renderer::DrawBox(boxPosition, boxDimensions, boxColor, boxFacing);

			Renderer::EndBatch();
			Renderer::Flush();

			ImGui::End();
			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
		
		Renderer::Shutdown();
	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}