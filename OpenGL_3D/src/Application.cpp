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

#include <glm/gtx/string_cast.hpp>

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

		glm::vec3 camera(0, 50, 0);
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

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
			//float camX = sin(glfwGetTime()) * radius;
			//float camZ = cos(glfwGetTime()) * radius;
			float camX = cos(camera.x) * radius;
			float camZ = sin(camera.x) * radius;
			
			glm::mat4 view;

			// lookAt manual implementation
			glm::vec3 camPosition = glm::vec3(camX, camera.y, camZ);
			glm::vec3 camTarget = glm::vec3(0.0, 0.0, 0.0);
			glm::vec3 camDirection = glm::normalize(camPosition - camTarget);
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 camRight = glm::cross(up, camDirection);
			glm::vec3 camUp = glm::cross(camDirection, camRight);
			view = glm::mat4(glm::vec4(camRight, 0.0), glm::vec4(camUp, 0.0), glm::vec4(camDirection, 0.0), glm::vec4(glm::vec3(0.0), 1.0));
			view = glm::transpose(view) * glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 1.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 1.0, 0.0), glm::vec4(-camPosition, 1.0));

			//view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			viewProj = glm::perspectiveFov(90.0f, 960.0f, 540.0f, 0.1f, 1000.0f) * view;
			
			shader.SetUniformMat4f("u_ViewProj", viewProj);
			shader.SetUniform3f("u_ViewPos", camPosition.x, camPosition.y, camPosition.z);

			Renderer::ResetStats();
			Renderer::BeginBatch();

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

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				camera.y++;
			}
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				camera.y--;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				camera.x -= 0.1f;
			}
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				camera.x += 0.1f;
			}

			float sensitivity = 0.05f;
			double newMouseX, newMouseY;
			glfwGetCursorPos(window, &newMouseX, &newMouseY);
			camera.y -= (newMouseY - mouseY);
			camera.x += (newMouseX - mouseX) * sensitivity;
			mouseX = newMouseX;
			mouseY = newMouseY;
		}
		
		Renderer::Shutdown();
	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}