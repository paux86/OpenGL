#pragma once

#include "glm/glm.hpp"

class Renderer
{
public:
	static void Init();
	static void Shutdown();
	
	static void BeginBatch();
	static void EndBatch();
	static void Flush();

	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t textureID);

	static void DrawBox(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color, const glm::vec3& facing);

	struct Stats
	{
		uint32_t DrawCount = 0;
		uint32_t QuadCount = 0;
	};

	static const Stats& GetStats();
	static void ResetStats();
};