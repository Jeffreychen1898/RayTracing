#include "Scene.hpp"


Sphere::Sphere(float _radius, float _x, float _y, float _z, float _r, float _g, float _b, float _lightIntensity)
	: radius(_radius), center(_x, _y, _z), color_red(_r), color_green(_g), color_blue(_b), light_intensity(_lightIntensity)
{ }

float Sphere::getNormal(
		Renderer::Vec3<float>& _normal,
		Renderer::Vec3<float>& _surface,
		const Renderer::Vec3<float>& _rayOrigin,
		const Renderer::Vec3<float>& _rayDirection) const
{
	Renderer::Vec3<float> origin_to_center = _rayOrigin - center;

	float a = _rayDirection.dot(_rayDirection);
	float b = 2.f * origin_to_center.dot(_rayDirection);
	float c = origin_to_center.dot(origin_to_center) - radius * radius;

	float discriminant = b * b - 4.f * a * c;
	if(discriminant < 0)
		return -1.f;

	float t = (-b - std::sqrt(discriminant)) / (2.f * a);

	_surface = _rayOrigin + _rayDirection * t;

	_normal = _surface - center;

	return t;
}

Renderer::Vec3<float> Sphere::findNormal(float t, const Renderer::Vec3<float>& _rayOrigin, const Renderer::Vec3<float>& _rayDirection)
{
	return _rayOrigin + _rayDirection * t - center;
}

float Sphere::findT(const Renderer::Vec3<float>& _rayOrigin, const Renderer::Vec3<float>& _rayDirection) const
{
	Renderer::Vec3<float> origin_to_center = _rayOrigin - center;
	float a = _rayDirection.dot(_rayDirection);
	float b = 2.f * origin_to_center.dot(_rayDirection);
	float c = origin_to_center.dot(origin_to_center) - radius * radius;

	float discriminant = b * b - 4.f * a * c;
	if(discriminant < 0)
		return -1.f;

	return (-b - std::sqrt(discriminant)) / (2.f * a);
}

void Sphere::multColors(float& _r, float& _g, float& _b)
{
	_r *= color_red;
	_g *= color_green;
	_b *= color_blue;
}

void Sphere::addLights(float& _r, float& _g, float& _b, float _ar, float _ag, float _ab)
{
	_r += color_red * light_intensity * _ar;
	_g += color_green * light_intensity * _ag;
	_b += color_blue * light_intensity * _ab;
}
