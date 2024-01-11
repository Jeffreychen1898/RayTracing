#pragma once

#include <iostream>
#include <list>
#include <unordered_map>
#include <fstream>

#include <glad/glad.h>

#include "../Window/Window.hpp"

namespace Renderer
{
	enum class AttribType
	{
		VEC2, VEC3, VEC4,
		IVEC2, IVEC3, IVEC4,
		FLOAT, INT
	};

	enum class AttribDataType
	{
		FLOAT, INT
	};

	struct VBO
	{
		GLuint vbo;
		int size;
		unsigned int location;
		AttribDataType type;
	};

	enum class UniformType
	{
		FLOAT, FLOAT_ARR,
		VEC2, VEC3, VEC4,

		INT, INT_ARR,
		IVEC2, IVEC3, IVEC4,

		MAT2, MAT3, MAT4
	};

	struct UniformObject
	{
		const char* name;
		int location;
		UniformType type;
	};

	class Shader
	{
		private:
			static Shader* s_currentShader;

			GLuint m_program;

			GLuint m_vao;
			GLuint m_ibo;

			uintptr_t m_attribOffset;

			std::list<VBO> m_vbos;

			Renderer::Window* m_window;

			bool m_initialized;

			bool m_autoBind;

			std::unordered_map<const char*, UniformObject> m_uniformLocation;
		public:
			Shader(bool _autoBind = false);
			~Shader();

			void attach(Renderer::Window* _window);
			void create(const char* _vertexCode, const char* _fragmentCode, bool _checkErrs = false);
			void createFromFile(const char* _vertexPath, const char* _fragmentPath, bool _checkErrs = false);

			void bind();
			bool isBound() const { return this == s_currentShader; };
			bool willAutoBind() const { return m_autoBind; };

			void verticesData(const void* _vertices, unsigned int _arrBitSize);

			void indicesData(unsigned int* _indices, unsigned int _indicesCount);

			void uniformAdd(const char* _uniformName, UniformType _type);

			void setUniformInt(const char* _name, int _data);
			void setUniformInt(const char* _name, const int* _data);
			void setUniformInt(const char* _name, int _count, const int* _data);
			void setUniformFloat(const char* _name, float _data);
			void setUniformFloat(const char* _name, const float* _data);
			void setUniformFloat(const char* _name, int _count, const float* _data);
			void setUniformMatrix(const char* _name, const float* _data);

			void vertexAttribAdd(unsigned int _location, AttribType _attribType);
			void vertexAttribsEnable();

			const Renderer::Window* getWindow() const { return m_window; };
			const std::list<VBO>& getVBO() const { return m_vbos; };
			const std::unordered_map<const char*, UniformObject>& getUniforms() const { return m_uniformLocation; };
			unsigned int getVertexBitSize() const { return static_cast<unsigned int>(m_attribOffset); };

			static Shader* getCurrentShader() { return s_currentShader; };

		private:
			void assertValidRenderer();
			void assertCurrentContext();
			void assertShaderBound(const char* _func);

			GLuint createShader(const char* _sourcecode, GLenum _shaderType, bool _checkErrs);
			static int getAttribSize(const AttribType& _type);
			static AttribDataType getAttribDataType(const AttribType& _type);
	};
}
