#pragma once

#include <cmath>
#include <iostream>
#include <cstring>

#include "../Utils/Exceptions.hpp"

namespace Renderer
{
	template<typename T, int N>
	class Vec
	{
		protected:
			T m_data[N];
		public:
			Vec()
			{
				for(int i=0;i<N;++i)
					m_data[i] = 0;
			}

			Vec(const Vec<T, N>& _other)
			{
				for(int i=0;i<N;++i)
					m_data[i] = _other.get(i);
			}

			void set(int _index, T _value)
			{
				if(_index >= N || _index < 0)
					throw Renderer::OutOfRangeException("Index out of range!");

				m_data[_index] = _value;
			}

			T get(int _index) const
			{
				if(_index >= N || _index < 0)
					throw Renderer::OutOfRangeException("Index out of range!");

				return m_data[_index];
			}

			T length() const
			{
				T sq_len = 0;
				for(int i=0;i<N;++i)
					sq_len += m_data[i] * m_data[i];

				return std::sqrt(sq_len);
			}

			void normalize()
			{
				T len = length();
				for(int i=0;i<N;++i)
					m_data[i] /= len;
			}

			virtual Vec<T, N>& operator=(const Vec<T, N>& _other)
			{
				if(&_other == this)
					return *this;

				for(int i=0;i<N;++i)
					m_data[i] = _other.get(i);

				return *this;
			}

			virtual const T* operator*() const
			{
				return &m_data[0];
			}

			virtual Vec<T, N> operator*(const Vec<T, N>& _other) const
			{
				Vec<T, N> new_vector;
				for(int i=0;i<N;++i)
					new_vector.set(i, m_data[i] * _other.get(i));

				return new_vector;
			}

			virtual Vec<T, N> operator*(T _scalar) const
			{
				Vec<T, N> new_vector;
				for(int i=0;i<N;++i)
					new_vector.set(i, m_data[i] * _scalar);

				return new_vector;
			}

			virtual Vec<T, N> operator+(const Vec<T, N>& _other) const
			{
				Vec<T, N> new_vector;
				for(int i=0;i<N;++i)
					new_vector.set(i, m_data[i] + _other.get(i));

				return new_vector;
			}

			virtual Vec<T, N> operator-(const Vec<T, N>& _other) const
			{
				Vec<T, N> new_vector;
				for(int i=0;i<N;++i)
					new_vector.set(i, m_data[i] - _other.get(i));

				return new_vector;
			}

			bool operator==(const Vec<T, N>& _other) const
			{
				for(int i=0;i<N;++i)
					if(_other.get(i) != m_data[i]) return false;

				return true;
			}

			bool operator!=(const Vec<T, N>& _other) const
			{
				for(int i=0;i<N;++i)
					if(_other.get(i) != m_data[i]) return true;

				return false;
			}

			virtual T dot(Vec<T, N> _other) const
			{
				T result = 0;
				for(int i=0;i<N;++i)
					result += m_data[i] * _other.get(i);

				return result;
			}

			virtual void print(std::ostream& _os, bool _newline = false) const
			{
				_os << "< ";
				for(int i=0;i<N;++i)
					_os << m_data[i] << " ";
				_os << ">";

				if(_newline)
					_os << "\n";
			}
	};

	template<typename T = float>
	class Vec4 : public Vec<T, 4>
	{
		public:
			T& x = this->m_data[0];
			T& y = this->m_data[1];
			T& z = this->m_data[2];
			T& w = this->m_data[3];

			Vec4()
				: Vec<T, 4>()
			{ }

			Vec4(T _v)
				: Vec<T, 4>()
			{
				x = _v;
				y = _v;
				z = _v;
				w = _v;
			}

			Vec4(T _x, T _y, T _z, T _w=(T)1)
				: Vec<T, 4>()
			{
				x = _x;
				y = _y;
				z = _z;
				w = _w;
			}

			Vec4(const Vec4<T>& _other)
				: Vec<T, 4>()
			{
				x = _other.x;
				y = _other.y;
				z = _other.z;
				w = _other.w;
			}

			virtual const T* operator*() const override
			{
				return &(this->m_data[0]);
			}

			Vec4& operator=(Vec4<T> _other)
			{
				if(&_other == this)
					return *this;

				x = _other.x;
				y = _other.y;
				z = _other.z;
				w = _other.w;
				return *this;
			}

			Vec4 operator*(Vec4<T> _other) const
			{
				Vec4<T> result;
				result.x = x * _other.x;
				result.y = y * _other.y;
				result.z = z * _other.z;
				result.w = w * _other.w;

				return result;
			}

			Vec4 operator+(Vec4<T> _other) const
			{
				Vec4<T> result;
				result.x = x + _other.x;
				result.y = y + _other.y;
				result.z = z + _other.z;
				result.w = w + _other.w;

				return result;
			}

			Vec4 operator-(Vec4<T> _other) const
			{
				Vec4<T> result;
				result.x = x - _other.x;
				result.y = y - _other.y;
				result.z = z - _other.z;
				result.w = w - _other.w;

				return result;
			}

			Vec4<T> reflect(Vec4<T> _other) const
			{
				Vec4<T> projection = project(_other) * (T)(2.f);
				return projection - *this;
			}

			Vec4<T> project(Vec4<T> _other) const
			{
				T other_dot_self = _other.dot(_other);
				if(other_dot_self == (T)(0))
					return Vec4<T>(0, 0, 0, 0);

				T this_dot_other = this->dot(_other);

				T scale = this_dot_other / other_dot_self;
				return _other * scale;
			}

			Vec4<T> Q_mult(Vec4<T> _other) const
			{
				T component_x = w * _other.x + x * _other.w + y * _other.z - z * _other.y;
				T component_y = w * _other.y - x * _other.z + y * _other.w + z * _other.x;
				T component_z = w * _other.z + x * _other.y - y * _other.x + z * _other.w;
				T real_component = w * _other.w - x * _other.x - y * _other.y - z * _other.z;

				return Vec4<T>(component_x, component_y, component_z, real_component);
			}

			void Q_inverse()
			{
				T length_sq = x * x + y * y + z * z + w * w;
				length_sq = (T)(1.f) / length_sq;
				x = -x * length_sq;
				y = -y * length_sq;
				z = -z * length_sq;
				w = w * length_sq;
			}
	};

	template<typename T = float>
	class Vec3 : public Vec<T, 3>
	{
		public:
			T& x = this->m_data[0];
			T& y = this->m_data[1];
			T& z = this->m_data[2];

			Vec3()
				: Vec<T, 3>()
			{ }

			Vec3(T _v)
				: Vec<T, 3>()
			{
				x = _v;
				y = _v;
				z = _v;
			}

			Vec3(T _x, T _y, T _z)
				: Vec<T, 3>()
			{
				x = _x;
				y = _y;
				z = _z;
			}

			Vec3(const Vec3<T>& _other)
				: Vec<T, 3>()
			{
				x = _other.x;
				y = _other.y;
				z = _other.z;
			}

			virtual const T* operator*() const override
			{
				return &(this->m_data[0]);
			}

			Vec3& operator=(Vec3<T> _other)
			{
				if(&_other == this)
					return *this;

				x = _other.x;
				y = _other.y;
				z = _other.z;
				return *this;
			}

			Vec3 operator*(Vec3<T> _other) const
			{
				Vec3<T> result;
				result.x = x * _other.x;
				result.y = y * _other.y;
				result.z = z * _other.z;

				return result;
			}

			Vec3 operator+(Vec3<T> _other) const
			{
				Vec3<T> result;
				result.x = x + _other.x;
				result.y = y + _other.y;
				result.z = z + _other.z;

				return result;
			}

			Vec3 operator-(Vec3<T> _other) const
			{
				Vec3<T> result;
				result.x = x - _other.x;
				result.y = y - _other.y;
				result.z = z - _other.z;

				return result;
			}

			Vec3<T> reflect(Vec3<T> _other) const
			{
				Vec3<T> projection = project(_other) * (T)(2.f);
				return projection - *this;
			}

			Vec3<T> project(Vec3<T> _other) const
			{
				T other_dot_self = _other.dot(_other);
				if(other_dot_self == (T)(0))
					return Vec3<T>(0, 0, 0);

				T this_dot_other = this->dot(_other);

				T scale = this_dot_other / other_dot_self;
				return _other * scale;
			}

			Vec3<T> cross(Vec3<T> _other) const
			{
				T component_x = y * _other.z - z * _other.y;
				T component_y = -(x * _other.z - z * _other.x);
				T component_z = x * _other.y - y * _other.x;

				return Vec3<T>(component_x, component_y, component_z);
			}
	};

	template<typename T = float>
	class Vec2 : public Vec<T, 2>
	{
		public:
			T& x = this->m_data[0];
			T& y = this->m_data[1];

			Vec2()
				: Vec<T, 2>()
			{ }

			Vec2(T _v)
				: Vec<T, 2>()
			{
				x = _v;
				y = _v;
			}

			Vec2(T _x, T _y)
				: Vec<T, 2>()
			{
				x = _x;
				y = _y;
			}

			Vec2(const Vec2<T>& _other)
				: Vec<T, 2>()
			{
				x = _other.x;
				y = _other.y;
			}

			virtual const T* operator*() const override
			{
				return &(this->m_data[0]);
			}

			Vec2& operator=(Vec2<T> _other)
			{
				if(&_other == this)
					return *this;

				x = _other.x;
				y = _other.y;
				return *this;
			}

			Vec2 operator*(Vec2<T> _other) const
			{
				Vec2<T> result;
				result.x = x * _other.x;
				result.y = y * _other.y;

				return result;
			}

			Vec2 operator+(Vec2<T> _other) const
			{
				Vec2<T> result;
				result.x = x + _other.x;
				result.y = y + _other.y;

				return result;
			}

			Vec2 operator-(Vec2<T> _other) const
			{
				Vec2<T> result;
				result.x = x - _other.x;
				result.y = y - _other.y;

				return result;
			}

			Vec2<T> reflect(Vec2<T> _other) const
			{
				Vec2<T> projection = project(_other) * (T)(2.f);
				return projection - *this;
			}

			Vec2<T> project(Vec2<T> _other) const
			{
				T other_dot_self = _other.dot(_other);
				if(other_dot_self == (T)(0))
					return Vec2<T>(0, 0);

				T this_dot_other = this->dot(_other);

				T scale = this_dot_other / other_dot_self;
				return _other * scale;
			}
	};

	namespace Math
	{
		const float PI = 3.14159265359;

		template<typename T>
		T Lerp(T _start, T _end, float _t)
		{
			T difference = _end - _start;
			return _start + (difference * _t);
		}

		template<typename T>
		Vec3<T> Slerp(Vec3<T> _start, Vec3<T> _end, float _t)
		{
			// make sure the two vectors are normalized
			_start.normalize();
			_end.normalize();

			// calculate the angle
			float calculate_angle = std::acos(_start.dot(_end));
			// if angle = 0, _start = _end
			if(calculate_angle == 0.f) return _start;

			// calculate the weights
			T weight_one = (T)(std::sin((1.f - _t) * calculate_angle));
			T weight_two = (T)(std::sin(_t * calculate_angle));
			T csc_angle = (T)(1.f / std::sin(calculate_angle));

			// return the interpolated vector
			return ((_start * weight_one) + (_end * weight_two)) * csc_angle;
		}

		template<typename T>
		void RotateVec3(Renderer::Vec3<T>& _rotateVector, Renderer::Vec3<T> _axis, float _angle)
		{
			_axis.normalize();
			// create the q quaternion
			T cos_angle = std::cos(_angle / 2.f);
			_axis = _axis * (T)(std::sin(_angle / 2.f));
			Renderer::Vec4<T> vector_q(_axis.x, _axis.y, _axis.z, cos_angle);

			// create the q^-1 quaternion
			Renderer::Vec4<T> vector_q_inv = vector_q;
			vector_q_inv.Q_inverse();

			Renderer::Vec4<T> vector_quaternion(_rotateVector.x, _rotateVector.y, _rotateVector.z, (T)0);

			vector_quaternion = vector_q.Q_mult(vector_quaternion).Q_mult(vector_q_inv);

			_rotateVector.x = vector_quaternion.x;
			_rotateVector.y = vector_quaternion.y;
			_rotateVector.z = vector_quaternion.z;
		}
	}
}
