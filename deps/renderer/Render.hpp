#pragma once

#include <iostream>
#include <cstring>
#include <cmath>

#include <glad/glad.h>

#include "Utils/Exceptions.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "Window/Window.hpp"
#include "Opengl/Shader.hpp"
#include "Opengl/Texture.hpp"

namespace Renderer
{
	enum class VerticalAlign
	{
		DEFAULT, TOP, CENTER, BOTTOM
	};

	enum class HorizontalAlign
	{
		DEFAULT, LEFT, CENTER, RIGHT
	};

	enum class BlendMode
	{
		BLEND,
		MULTIPLY,
		ADD,
		SUBTRACT,
		REPLACE,
		IFEMPTY,
		MIN,
		MAX
	};

	enum class DrawType
	{
		NONE, POINTS,
		TRIANGLE, TRIANGLE_STRIP, TRIANGLE_FAN,
		LINE, LINE_STRIP, LINE_LOOP
	};

	struct RectStyle
	{
		Renderer::Color color;
		VerticalAlign verticalAlign;
		HorizontalAlign horizontalAlign;
		int verticalAlignAmount;
		int horizontalAlignAmount;
		float angle;
	};

	class Render
	{
		friend class RendererWindowEvent;
		private:
			Renderer::Window* m_window;

			// batch rendering
			unsigned int m_vertexBatchSize;
			unsigned int m_indexBatchSize;

			unsigned char* m_verticesBatch;
			unsigned int* m_indicesBatch;

			unsigned int m_verticesTracker;
			unsigned int m_indicesTracker;

			// draw types
			DrawType m_currentDrawType;

			// default shaders and textures
			Renderer::Shader* m_defaultShader;

			Renderer::Texture* m_whiteTexture;

			// for drawing shapes
			unsigned int m_shapeVertexTracker;
			unsigned int m_shapeVertexBytesLeft;
			unsigned int m_shapeIndexCount;
			unsigned int m_startOfShapeVertexTracker;

			// drawing rectangles
			RectStyle m_defaultRectStyle;

		public:
			Render(unsigned int _vertexBatchSize = 200000, unsigned int _indexBatchSize = 10000);
			~Render();

			void attach(Renderer::Window* _window);
			void init();

			void bindShader(Renderer::Shader* _shader = nullptr);
			void bindTexture(Renderer::Texture* _texture, unsigned int _slot = 0);

			// styles
			void setColor(const Renderer::Color& _color);
			void setAlign(HorizontalAlign _horizontal, VerticalAlign _vertical);
			void setAlign(int _x, int _y);
			void setAngle(float _angle);
			RectStyle getStyle() const { return m_defaultRectStyle; };

			void setBlendMode(BlendMode blendMode);

			// drawing
			void drawRect(int _x, int _y, int _width, int _height);
			void drawRect(int _x, int _y, int _width, int _height, const RectStyle& _style);
			void drawImage(Renderer::Texture& _texture, int _x, int _y, int _width, int _height);
			void drawImage(Renderer::Texture& _texture, int _x, int _y, int _width, int _height,
					const RectStyle& _style);

			void beginShape(DrawType _type, unsigned int _vertexCount, unsigned int _indicesCount);
			void nextVertex();
			void vertex1f(float _v);
			void vertex2f(float _v0, float _v1);
			void vertex3f(float _v0, float _v1, float _v2);
			void vertex4f(float _v0, float _v1, float _v2, float _v3);
			void vertex1i(int _v);
			void vertex2i(int _v0, int _v1);
			void vertex3i(int _v0, int _v1, int _v2);
			void vertex4i(int _v0, int _v1, int _v2, int _v3);
			void endShape(const unsigned int* _indices);
			void endShape();

			// render
			void render();

			Renderer::Window* getWindow() { return m_window; };
		private:
			void assertShapeVertexSafeToStore(unsigned int _bytesRequired);
	};

	class RendererWindowEvent : public Renderer::WindowEvents
	{
		private:
			Render* m_renderer;

		public:
			RendererWindowEvent(Render* _renderer) : m_renderer(_renderer) {};
			void WindowResize(int _width, int _height) override;
	};
}
