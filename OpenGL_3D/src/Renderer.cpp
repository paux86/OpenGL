#include "Renderer.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

static const size_t MaxQuadCount = 10000;
static const size_t MaxVertexCount = MaxQuadCount * 4;
static const size_t MaxIndexCount = MaxQuadCount * 6;
static const size_t MaxTextures = 32;

struct Vertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoords;
	float TexIndex;
	glm::vec3 Normal;
};

struct RendererData
{
	GLuint QuadVA = 0;
	GLuint QuadVB = 0;
	GLuint QuadIB = 0;

	GLuint WhiteTexture = 0;
	uint32_t IndexCount = 0;

	Vertex* QuadBuffer = nullptr;
	Vertex* QuadBufferPtr = nullptr;

	std::array<uint32_t, MaxTextures> TextureSlots;
	uint32_t TextureSlotIndex = 1;

	Renderer::Stats RendererStats;
};

static RendererData s_Data;

void Renderer::Init()
{
	s_Data.QuadBuffer = new Vertex[MaxVertexCount];

	glCreateVertexArrays(1, &s_Data.QuadVA);
	glBindVertexArray(s_Data.QuadVA);

	glCreateBuffers(1, &s_Data.QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, s_Data.QuadVB);
	glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexArrayAttrib(s_Data.QuadVA, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	glEnableVertexArrayAttrib(s_Data.QuadVA, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	glEnableVertexArrayAttrib(s_Data.QuadVA, 2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));

	glEnableVertexArrayAttrib(s_Data.QuadVA, 3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexIndex));

	glEnableVertexArrayAttrib(s_Data.QuadVA, 4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Normal));

	uint32_t indices[MaxIndexCount];
	uint32_t offset = 0;
	for (int i = 0; i < MaxIndexCount; i += 6)
	{
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;

		offset += 4;
	}

	glCreateBuffers(1, &s_Data.QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data.QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//1x1 white texture
	glCreateTextures(GL_TEXTURE_2D, 1, &s_Data.WhiteTexture);
	glBindTexture(GL_TEXTURE_2D, s_Data.WhiteTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	uint32_t color = 0xffffffff;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

	s_Data.TextureSlots[0] = s_Data.WhiteTexture;
	for (size_t i = 1; i < MaxTextures; i++)
		s_Data.TextureSlots[i] = 0;
}

void Renderer::Shutdown()
{
	glDeleteVertexArrays(1, &s_Data.QuadVA);
	glDeleteBuffers(1, &s_Data.QuadVB);
	glDeleteBuffers(1, &s_Data.QuadIB);

	glDeleteTextures(1, &s_Data.WhiteTexture);

	delete[] s_Data.QuadBuffer;
}

void Renderer::BeginBatch()
{
	s_Data.QuadBufferPtr = s_Data.QuadBuffer;
}

void Renderer::EndBatch()
{
	GLsizeiptr size = (uint8_t*)s_Data.QuadBufferPtr - (uint8_t*)s_Data.QuadBuffer;
	glBindBuffer(GL_ARRAY_BUFFER, s_Data.QuadVB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_Data.QuadBuffer);
}

void Renderer::Flush()
{
	for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		glBindTextureUnit(i, s_Data.TextureSlots[i]);

	glBindVertexArray(s_Data.QuadVA);
	glDrawElements(GL_TRIANGLES, s_Data.IndexCount, GL_UNSIGNED_INT, nullptr);
	s_Data.RendererStats.DrawCount++;

	s_Data.IndexCount = 0;
	s_Data.TextureSlotIndex = 1;
}

void Renderer::DrawQuad(const glm::vec2 & position, const glm::vec2 & size, const glm::vec4 & color)
{
	if (s_Data.IndexCount + 6 >= MaxIndexCount)
	{
		EndBatch();
		Flush();
		BeginBatch();
	}

	float textureIndex = 0.0f;


	s_Data.QuadBufferPtr->Position = { position.x, position.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
}

void Renderer::DrawQuad(const glm::vec2 & position, const glm::vec2 & size, uint32_t textureID)
{
	if (s_Data.IndexCount + 6 >= MaxIndexCount || s_Data.TextureSlotIndex > (MaxTextures - 1))
	{
		EndBatch();
		Flush();
		BeginBatch();
	}

	constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	float textureIndex = 0.0f;
	for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
	{
		if (s_Data.TextureSlots[i] == textureID)
		{
			textureIndex = (float)i;
			break;
		}
	}

	if (textureIndex == 0.0f)
	{
		textureIndex = (float)s_Data.TextureSlotIndex;
		s_Data.TextureSlots[s_Data.TextureSlotIndex] = textureID;
		s_Data.TextureSlotIndex++;
	}

	s_Data.QuadBufferPtr->Position = { position.x, position.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
}

void Renderer::DrawBox(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color, const glm::vec3& facing)
{
	// Should leave this a 2d quad renderer and create a second 3d renderer
	// Forcing this here is doubling the number of verticies used vs required
	
	if (s_Data.IndexCount + 36 >= MaxIndexCount)
	{
		EndBatch();
		Flush();
		BeginBatch();
	}

	float textureIndex = 0.0f;

	//facing should already be normalized
	glm::vec3 v_facing_norm = glm::normalize(facing);
	glm::vec3 v_up = { 0,1,0 };
	glm::vec3 v_right = glm::normalize(glm::cross(v_up, v_facing_norm));
	v_up = glm::normalize(glm::cross(v_facing_norm, v_right));

	glm::vec3 frontBottomLeftPosition = position + (v_facing_norm * size.x * 0.5f) - (v_right * size.y * 0.5f) - (v_up * size.z * 0.5f);
	
	// front quad
	glm::vec3 frontNormal = glm::cross(v_right, v_up);
	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition;
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = frontNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = frontNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y) + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = frontNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = color;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = frontNormal;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
	
	// back quad
	glm::vec3 backNormal = glm::cross(v_right, -v_up);
	glm::vec4 backColor = { 0.8f, 0.1f, 0.2f, 1.0f };
	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x);
	s_Data.QuadBufferPtr->Color = backColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = backNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y) - (v_facing_norm * size.x);
	s_Data.QuadBufferPtr->Color = backColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = backNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y) + (v_up * size.z) - (v_facing_norm * size.x);
	s_Data.QuadBufferPtr->Color = backColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = backNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_up * size.z) - (v_facing_norm * size.x);
	s_Data.QuadBufferPtr->Color = backColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = backNormal;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
	
	// left quad
	glm::vec3 leftNormal = glm::cross(v_facing_norm, v_up);
	glm::vec4 leftColor = { 0.4f, 0.6f, 0.2f, 1.0f };
	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition;
	s_Data.QuadBufferPtr->Color = leftColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = leftNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x);
	s_Data.QuadBufferPtr->Color = leftColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = leftNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x) + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = leftColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = leftNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = leftColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = leftNormal;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
	
	// right quad
	glm::vec3 rightNormal = glm::cross(v_facing_norm, -v_up);
	glm::vec4 rightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = rightColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = rightNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x) + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = rightColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = rightNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x) + (v_up * size.z) + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = rightColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = rightNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_up * size.z) + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = rightColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = rightNormal;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
	
	// bottom quad
	glm::vec3 bottomNormal = glm::cross(v_right, v_facing_norm);
	glm::vec4 bottomColor = { 0.0f, 1.0f, 1.0f, 1.0f };
	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition;
	s_Data.QuadBufferPtr->Color = bottomColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = bottomNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x);
	s_Data.QuadBufferPtr->Color = bottomColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = bottomNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x) + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = bottomColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = bottomNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y);
	s_Data.QuadBufferPtr->Color = bottomColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = bottomNormal;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;

	// top quad
	glm::vec3 topNormal = glm::cross(v_facing_norm, v_right);
	glm::vec4 topColor = { 1.0f, 0.0f, 1.0f, 1.0f };
	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = topColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = topNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x) + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = topColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = topNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition - (v_facing_norm * size.x) + (v_right * size.y) + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = topColor;
	s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = topNormal;
	s_Data.QuadBufferPtr++;

	s_Data.QuadBufferPtr->Position = frontBottomLeftPosition + (v_right * size.y) + (v_up * size.z);
	s_Data.QuadBufferPtr->Color = topColor;
	s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadBufferPtr->TexIndex = textureIndex;
	s_Data.QuadBufferPtr->Normal = topNormal;
	s_Data.QuadBufferPtr++;

	s_Data.IndexCount += 6;
	s_Data.RendererStats.QuadCount++;
}

const Renderer::Stats & Renderer::GetStats()
{
	return s_Data.RendererStats;
}

void Renderer::ResetStats()
{
	memset(&s_Data.RendererStats, 0, sizeof(Stats));
}
