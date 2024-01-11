#pragma once

#include <cstring>
#include <vector>

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>

#include "../Utils/Exceptions.hpp"
#include "../Math/Vector.hpp"
#include "../Window/Window.hpp"

namespace Renderer
{
	struct Color
	{
		int red;
		int green;
		int blue;
		int alpha;

		Color()
			: red(0), green(0), blue(0), alpha(0)
		{}

		Color(int v)
			: red(v), green(v), blue(v), alpha(255)
		{}

		Color(int v, int a)
			: red(v), green(v), blue(v), alpha(a)
		{}

		Color(int r, int g, int b)
			: red(r), green(g), blue(b), alpha(255)
		{}

		Color(int r, int g, int b, int a)
			: red(r), green(g), blue(b), alpha(a)
		{}
	};

	bool operator==(const Color& _lhs, const Color& _rhs);
	bool operator!=(const Color& _lhs, const Color& _rhs);
	std::ostream& operator<<(std::ostream& _os, const Color& _color);

	enum class TextureType
	{
		PNG, JPG
	};

	/* warning: load or create is meant to only be called once per instance */
	// TODO:
	// change data being represented by unsigned char* to void*
	//			m_data pixels take up same number of bits as image
	//			eh, maybe for another time :P

	class Texture
	{
		private:
			static unsigned int s_activeSlot;
			static std::vector<Texture*> s_boundedTextures;

			unsigned int m_channels;
			unsigned int m_channelSize;
			
			unsigned int m_width;
			unsigned int m_height;

			GLuint m_textureId;

			unsigned char* m_data;

			bool m_validImage;

			GLenum m_textureWrapS;
			GLenum m_textureWrapT;
			GLenum m_textureFilterMag;
			GLenum m_textureFilterMin;

			Color m_borderColor;

			bool m_useMipmaps;

			bool m_autobind;
			bool m_fromFile;

			Renderer::Window* m_window;

		public:
			Texture(unsigned int _channelSize = 8, bool _autoBind = false);
			~Texture();

			void create(Renderer::Window* _window, unsigned int _width, unsigned int _height,
					unsigned int _channels, unsigned char* _data);
			void load(Renderer::Window* _window, const char* _path);

			void readPixels();
			Color getPixel(unsigned int _x, unsigned int _y);

			void setPixel(unsigned int _x, unsigned int _y, Color _color);
			void setPixels(unsigned int _x, unsigned int _y, unsigned int _width, unsigned int _height, unsigned char* _data);

			void bind(unsigned int _slot = 0);
			bool isBound(unsigned int _slot = 0) const;

			void setTextureBorderColor(Color _color) { m_borderColor = _color; };
			void setTextureWrap(GLenum _wrapX, GLenum _wrapY);
			void setTextureFilter(GLenum _minFilter, GLenum _magFilter);
			void setMipmaps(bool _useMipmap) { m_useMipmaps = _useMipmap; };

			GLuint getId() const { return m_textureId; };
			unsigned int getWidth() const { return m_width; };
			unsigned int getHeight() const { return m_height; };
			unsigned int getChannels() const { return m_channels; };
			unsigned int getChannelSize() const { return m_channelSize; };
			bool willUseMipmaps() const { return m_useMipmaps; };
			bool isValidImage() const { return m_validImage; };
			const unsigned char* getData() const { return m_data; };

			void write(const char* _path, TextureType _type);

		private:
			GLenum getInternalFormat();

			void assertBound(const char* _func);
			void assertCurrentContext();
	};

	unsigned char* colorToUCharPtr(unsigned int _channels, unsigned int _count, const Color* _color);

	void flipColorVertically(unsigned int _width, unsigned int _height, unsigned int _channels, unsigned char* _data);

	template <typename T>
	Color Vec3ToColor(const Renderer::Vec3<T> _vec3)
	{
		return Color(_vec3.x, _vec3.y, _vec3.z);
	}

	template <typename T>
	Color Vec4ToColor(const Renderer::Vec4<T> _vec4)
	{
		return Color(_vec4.x, _vec4.y, _vec4.z, _vec4.w);
	}
}
