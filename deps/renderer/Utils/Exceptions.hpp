#pragma once

#include <exception>
#include <string>

namespace Renderer
{
	class InvalidType : std::exception
	{
		public:
			std::string message;
			InvalidType(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class InvalidOperationException : public std::exception
	{
		public:
			std::string message;
			InvalidOperationException(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class OutOfRangeException : public std::exception
	{
		public:
			std::string message;
			OutOfRangeException(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class InvalidWindowContext : public std::exception
	{
		public:
			std::string message;
			InvalidWindowContext(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class WindowCreationFailed : public std::exception
	{
		public:
			std::string message;
			WindowCreationFailed(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class FileNotFoundException : public std::exception
	{
		public:
			std::string message;
			FileNotFoundException(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class ShaderCompilationException : public std::exception
	{
		public:
			std::string message;
			ShaderCompilationException(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class ShaderOperationRejected : public std::exception
	{
		public:
			std::string message;
			ShaderOperationRejected(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class InvalidFormat : public std::exception
	{
		public:
			std::string message;
			InvalidFormat(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class TextureOperationRejected : public std::exception
	{
		public:
			std::string message;
			TextureOperationRejected(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};

	class RenderingException : public std::exception
	{
		public:
			std::string message;
			RenderingException(std::string _msg) : message(_msg) {};

			const char* what() const noexcept override { return message.c_str(); }
	};
}
