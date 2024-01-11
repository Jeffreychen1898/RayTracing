#pragma once

#include <string>
#include <list>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image/stb_image.h>

#include "./../Utils/Exceptions.hpp"

namespace Renderer
{
	/* no resource sharing support ... for now. Kinda new to this :P */
	class WindowEvents
	{
		public:
			virtual void KeyPressed(int _key, int _scancode, int _mods) {};
			virtual void KeyReleased(int _key, int _scancode, int _mods) {};
			virtual void MousePressed(int _button, int _mods) {};
			virtual void MouseReleased(int _button, int _mods) {};
			virtual void MouseMove(double _x, double _y) {};
			virtual void WindowResize(int _width, int _height) {};
			virtual void MouseScroll(double _dx, double _dy) {}
			// add controller support later
	};

	struct Monitor
	{
		GLFWmonitor* monitor;
		int width;
		int height;
		int refreshRate;
		int bits[3];
	};

	class Window
	{
		private:
			static int s_monitorCount;
			static Monitor* s_monitors;
			static Window* s_currentContext;
			static int s_numWindows;

			GLFWwindow* m_window;
			unsigned int m_width;
			unsigned int m_height;
			std::string m_windowTitle;
			GLFWcursor* m_cursor;
			GLFWimage m_cursorGLFWImage;
			unsigned char* m_cursorImage;
			GLFWimage m_icon;
			unsigned char* m_iconImage;

			unsigned int m_currentMonitor;
			bool m_fullscreen;

			std::list<WindowEvents*> m_windowEvents;

			bool m_autoMakeCurrent;

		public:
			Window(bool _autoMakeCurrent = false);
			~Window();

			void init(unsigned int _width, unsigned int _height, const char* _title);

			void resize(unsigned int _width, unsigned int _height);
			void setFullScreen(int _monitor = 0);
			void exitFullScreen();
			void setVSync(bool _enable);
			void setCursorMode(int _cursorMode);
			void setCursor(int _cursor);
			void setCursor(const char* _image, int _hotspotX, int _hotspotY);
			void setIcon(const char* _image);
			void setPosition(int _x, int _y);

			void addEvents(WindowEvents* _event) { m_windowEvents.push_back(_event); };
			std::list<WindowEvents*>& getEvents() { return m_windowEvents; };

			void makeCurrent();
			void swapBuffers();

			bool isOpened();

			int getX();
			int getY();
			unsigned int getWidth() const { return m_width; };
			unsigned int getHeight() const { return m_height; };
			bool isFullScreen() const { return m_fullscreen; };
			bool isCurrentContext() const;
			bool willAutoMakeCurrent() const { return m_autoMakeCurrent; };
			std::string getWindowTitle() const { return m_windowTitle; };
			bool onFocus() const { return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) == GLFW_TRUE; };

			static unsigned int getMonitorCount() { return s_monitorCount; };
			static void GLFWInit();
			static void pollEvents() { glfwPollEvents(); };

			static Monitor getMonitorInfo(int _monitor = 0) { return s_monitors[_monitor]; };

		private:
			static void GLFWKeyEvent(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
			static void GLFWMouseButtonEvent(GLFWwindow* _window, int _button, int _action, int _mods);
			static void GLFWMouseMoveEvent(GLFWwindow* _window, double _x, double _y);
			static void GLFWFrameBufferResizeEvent(GLFWwindow* _window, int _width, int _height);
			static void GLFWWindowResizeEvent(GLFWwindow* _window, int _width, int _height);
			static void GLFWMouseScrollEvent(GLFWwindow* _window, double _dx, double _dy);

			void assertContextCurrent(std::string _func);

			static void GLFWFinished();
	};
}
