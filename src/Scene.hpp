#pragma once

#include <cmath>

#include <renderer/Renderer.hpp>

/*enum class ObjectType
{
	SPHERE, TRIANGLE
};

class SceneObject
{
	public:
		SceneObject(ObjectType _type) : m_type(_type) {};

		ObjectType getType() const { return m_type; };
		
		virtual float getNormal(
				Renderer::Vec3<float>& _normal,
				Renderer::Vec3<float>& _surface,
				const Renderer::Vec3<float>& _rayOrigin,
				const Renderer::Vec3<float>& _rayDirection) const = 0;

	private:
		ObjectType m_type;
};*/

class Sphere
{
	public:
		float radius;
		Renderer::Vec3<float> center;

		float color_red;
		float color_green;
		float color_blue;
		float light_intensity;

		Sphere(float _radius, float _x, float _y, float _z, float _r, float _g, float _b, float _lightIntensity);

		float getNormal(
				Renderer::Vec3<float>& _normal,
				Renderer::Vec3<float>& _surface,
				const Renderer::Vec3<float>& _rayOrigin,
				const Renderer::Vec3<float>& _rayDirection) const;

		Renderer::Vec3<float> findNormal(float t, const Renderer::Vec3<float>& _rayOrigin, const Renderer::Vec3<float>& _rayDirection);

		float findT(const Renderer::Vec3<float>& _rayOrigin, const Renderer::Vec3<float>& _rayDirection) const;

		void multColors(float& _r, float& _g, float& _b);
		void addLights(float& _r, float& _g, float& _b, float _ar, float _ag, float _ab);

		bool isLightSource() const { return light_intensity > 0.f; };
};
