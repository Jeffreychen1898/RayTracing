#pragma once

#include <iostream>
#include <cstring>

#include "../Utils/Exceptions.hpp"
#include "Vector.hpp"

namespace Renderer
{
	template<typename T, int N>
	class Mat
	{
		protected:
			T m_data[N * N];
		public:
			Mat()
			{
				identity();
			}

			Mat(const Mat<T, N>& _other)
			{
				memcpy(m_data, _other.m_data, sizeof(T) * N * N);
			}

			void transpose()
			{
				for(int i=0;i<N;++i)
				{
					for(int j=i+1;j<N;++j)
					{
						T store_original = m_data[i * N + j];
						m_data[i * N + j] = m_data[j * N + i];
						m_data[j * N + i] = store_original;
					}
				}
			}

			void identity()
			{
				for(int i=0;i<N*N;++i)
				{
					if(i % (N + 1) == 0)
						m_data[i] = 1;
					else
						m_data[i] = 0;
				}
			}

			T atRawIndex(int _i) const
			{
				if(_i >= N * N || _i < 0) throw Renderer::OutOfRangeException("Index out of range!");
				return m_data[_i];
			}

			T at(int _r, int _c) const
			{
				if(_r >= N || _r < 0 || _c >= N || _c < 0)
					throw Renderer::OutOfRangeException("Index out of range!");
				return m_data[_r * N + _c];
			}

			void set(int _r, int _c, T _val)
			{
				if(_r >= N || _r < 0 || _c >= N || _c < 0)
					throw Renderer::OutOfRangeException("Index out of range!");
				m_data[_r * N + _c] = _val;
			}

			virtual const T* operator*() const
			{
				return &m_data[0];
			}

			virtual Mat<T, N>& operator=(const Mat<T, N>& _other)
			{
				if(&_other == this)
					return *this;

				memcpy(m_data, _other.m_data, sizeof(T) * N * N);

				return *this;
			}

			virtual Vec<T, N> operator*(const Vec<T, N>& _other) const
			{
				Vec<T, N> new_vector;
				for(int i=0;i<N;++i)
				{
					T sum = 0;
					for(int k=0;k<N;++k)
						sum += m_data[i * N + k] * _other.get(k);

					new_vector.set(i, sum);
				}

				return new_vector;
			}

			virtual Mat<T, N> operator*(const Mat<T, N>& _other) const
			{
				Mat<T, N> new_matrix;
				for(int i=0;i<N;++i)
				{
					for(int j=0;j<N;++j)
					{
						T sum = 0;
						for(int k=0;k<N;++k)
							sum += m_data[i * N + k] * _other.at(k, j);

						new_matrix.set(i, j, sum);
					}
				}

				return new_matrix;
			}

			virtual Mat<T, N> operator+(const Mat<T, N>& _other) const
			{
				Mat<T, N> new_matrix;
				for(int i=0;i<N;++i)
					for(int j=0;j<N;++j)
						new_matrix.set(i, j, m_data[i*N+j] + _other.at(i,j));

				return new_matrix;
			}

			virtual Mat<T, N> operator-(const Mat<T, N>& _other) const
			{
				Mat<T, N> new_matrix;
				for(int i=0;i<N;++i)
					for(int j=0;j<N;++j)
						new_matrix.set(i, j, m_data[i*N+j] - _other.at(i,j));

				return new_matrix;
			}

			virtual bool operator==(const Mat<T, N>& _other) const
			{
				Mat<T, N> new_matrix;
				for(int i=0;i<N*N;++i)
					if(m_data[i] != _other.atRawIndex(i)) return false;

				return true;
			}

			virtual bool operator!=(const Mat<T, N>& _other) const
			{
				Mat<T, N> new_matrix;
				for(int i=0;i<N*N;++i)
					if(m_data[i] != _other.atRawIndex(i)) return true;

				return false;
			}

			void print(std::ostream& _os) const
			{
				_os << "[\n";
				for(int i=0;i<N;++i)
				{
					for(int j=0;j<N;++j)
						_os << m_data[i * N + j] << " ";
					_os << "\n";
				}
				_os << "]\n";
			}
	};

	template<typename T = float>
	class Mat4 : public Mat<T, 4>
	{
		public:
			T& v00 = this->m_data[0];
			T& v01 = this->m_data[1];
			T& v02 = this->m_data[2];
			T& v03 = this->m_data[3];

			T& v10 = this->m_data[4];
			T& v11 = this->m_data[5];
			T& v12 = this->m_data[6];
			T& v13 = this->m_data[7];

			T& v20 = this->m_data[8];
			T& v21 = this->m_data[9];
			T& v22 = this->m_data[10];
			T& v23 = this->m_data[11];

			T& v30 = this->m_data[12];
			T& v31 = this->m_data[13];
			T& v32 = this->m_data[14];
			T& v33 = this->m_data[15];

			Mat4()
				: Mat<T, 4>()
			{ }

			Mat4(T* _arr)
				: Mat<T, 4>()
			{
				memcpy(this->m_data, _arr, sizeof(T) * 16);
			}

			Mat4(
					T a00, T a01, T a02, T a03,
					T a10, T a11, T a12, T a13,
					T a20, T a21, T a22, T a23,
					T a30, T a31, T a32, T a33
			)
			{
				v00 = a00;
				v01 = a01;
				v02 = a02;
				v03 = a03;

				v10 = a10;
				v11 = a11;
				v12 = a12;
				v13 = a13;

				v20 = a20;
				v21 = a21;
				v22 = a22;
				v23 = a23;

				v30 = a30;
				v31 = a31;
				v32 = a32;
				v33 = a33;
			}

			Mat4(const Renderer::Vec4<T>& _basis1, const Renderer::Vec4<T>& _basis2,
					const Renderer::Vec4<T>& _basis3, const Renderer::Vec4<T>& _basis4)
			{
				v00 = _basis1.x;
				v10 = _basis1.y;
				v20 = _basis1.z;
				v30 = _basis1.w;

				v01 = _basis2.x;
				v11 = _basis2.y;
				v21 = _basis2.z;
				v31 = _basis2.w;

				v02 = _basis3.x;
				v12 = _basis3.y;
				v22 = _basis3.z;
				v32 = _basis3.w;

				v03 = _basis4.x;
				v13 = _basis4.y;
				v23 = _basis4.z;
				v33 = _basis4.w;
			}

			Mat4(const Mat4<T>& _other)
			{
				memcpy(this->m_data, _other.m_data, sizeof(T) * 16);
			}

			const T* operator*() const override
			{
				return &(this->m_data[0]);
			}

			T determinant() const
			{
				T determinant_value = 0;
				for(int i=0;i<4;++i)
				{
					T sign = i % 2 == 1 ? -1 : 1;
					int c0 = i != 0 ? 0 : 1;
					int c1 = i != 1 && c0 != 1 ? 1 : 2;
					int c2 = i != 2 && c1 != 2 ? 2 : 3;

					T det3x3 =
						this->at(1, c0) *
						(this->at(2, c1) * this->at(3, c2) - this->at(2, c2) * this->at(3, c1)) -
						this->at(1, c1) *
						(this->at(2, c0) * this->at(3, c2) - this->at(2, c2) * this->at(3, c0)) +
						this->at(1, c2) *
						(this->at(2, c0) * this->at(3, c1) - this->at(2, c1) * this->at(3, c0));

					determinant_value += sign * this->m_data[i] * det3x3;
				}

				return determinant_value;
			}

			void inverse()
			{
				T get_det = determinant();
				if(get_det == 0)
					throw Renderer::InvalidOperationException("Mat4 matrix det=0. Cannot take inverse!");

				T inv_det = ((T)1.f) / get_det;

				T cpy_data[16];
				memcpy(cpy_data, this->m_data, sizeof(T) * 16);

				for(int i=0;i<4;++i)
				{
					for(int j=0;j<4;++j)
					{
						T sign = (i + j) % 2 == 1 ? -1 : 1;

						// calculate the determinant of the 3x3
						int r0 = i != 0 ? 0 : 1;
						int r1 = i != 1 && r0 != 1 ? 1 : 2;
						int r2 = i != 2 && r1 != 2 ? 2 : 3;
						int c0 = j != 0 ? 0 : 1;
						int c1 = j != 1 && c0 != 1 ? 1 : 2;
						int c2 = j != 2 && c1 != 2 ? 2 : 3;

						T det3x3 =
							cpy_data[r0*4+c0] *
							(cpy_data[r1*4+c1] * cpy_data[r2*4+c2] - cpy_data[r1*4+c2] * cpy_data[r2*4+c1]) -
							cpy_data[r0*4+c1] *
							(cpy_data[r1*4+c0] * cpy_data[r2*4+c2] - cpy_data[r1*4+c2] * cpy_data[r2*4+c0]) +
							cpy_data[r0*4+c2] *
							(cpy_data[r1*4+c0] * cpy_data[r2*4+c1] - cpy_data[r1*4+c1] * cpy_data[r2*4+c0]);

						// j as row i as col -> transpose
						this->m_data[j * 4 + i] = inv_det * sign * det3x3;
					}
				}
			}

			Mat4<T>& operator=(Mat4<T> _other)
			{
				if(&_other == this)
					return *this;

				memcpy(this->m_data, _other.m_data, sizeof(T) * 16);

				return *this;
			}

			Vec4<T> operator*(const Vec4<T>& _other) const
			{
				return Vec4<T>(
						_other.x * v00 + _other.y * v01 + _other.z * v02 + _other.w * v03,
						_other.x * v10 + _other.y * v11 + _other.z * v12 + _other.w * v13,
						_other.x * v20 + _other.y * v21 + _other.z * v22 + _other.w * v23,
						_other.x * v30 + _other.y * v31 + _other.z * v32 + _other.w * v33
				);
			}

			Mat4<T> operator*(const Mat4<T>& _other) const
			{
				Mat4<T> new_matrix;
				new_matrix.v00 = v00 * _other.v00 + v01 * _other.v10 + v02 * _other.v20 + v03 * _other.v30;
				new_matrix.v01 = v00 * _other.v01 + v01 * _other.v11 + v02 * _other.v21 + v03 * _other.v31;
				new_matrix.v02 = v00 * _other.v02 + v01 * _other.v12 + v02 * _other.v22 + v03 * _other.v32;
				new_matrix.v03 = v00 * _other.v03 + v01 * _other.v13 + v02 * _other.v23 + v03 * _other.v33;

				new_matrix.v10 = v10 * _other.v00 + v11 * _other.v10 + v12 * _other.v20 + v13 * _other.v30;
				new_matrix.v11 = v10 * _other.v01 + v11 * _other.v11 + v12 * _other.v21 + v13 * _other.v31;
				new_matrix.v12 = v10 * _other.v02 + v11 * _other.v12 + v12 * _other.v22 + v13 * _other.v32;
				new_matrix.v13 = v10 * _other.v03 + v11 * _other.v13 + v12 * _other.v23 + v13 * _other.v33;

				new_matrix.v20 = v20 * _other.v00 + v21 * _other.v10 + v22 * _other.v20 + v23 * _other.v30;
				new_matrix.v21 = v20 * _other.v01 + v21 * _other.v11 + v22 * _other.v21 + v23 * _other.v31;
				new_matrix.v22 = v20 * _other.v02 + v21 * _other.v12 + v22 * _other.v22 + v23 * _other.v32;
				new_matrix.v23 = v20 * _other.v03 + v21 * _other.v13 + v22 * _other.v23 + v23 * _other.v33;

				new_matrix.v30 = v30 * _other.v00 + v31 * _other.v10 + v32 * _other.v20 + v33 * _other.v30;
				new_matrix.v31 = v30 * _other.v01 + v31 * _other.v11 + v32 * _other.v21 + v33 * _other.v31;
				new_matrix.v32 = v30 * _other.v02 + v31 * _other.v12 + v32 * _other.v22 + v33 * _other.v32;
				new_matrix.v33 = v30 * _other.v03 + v31 * _other.v13 + v32 * _other.v23 + v33 * _other.v33;

				return new_matrix;
			}

			Mat4<T> operator+(const Mat4<T>& _other) const
			{
				return Mat4<T>(
						v00 + _other.v00, v01 + _other.v01, v02 + _other.v02, v03 + _other.v03,
						v10 + _other.v10, v11 + _other.v11, v12 + _other.v12, v13 + _other.v13,
						v20 + _other.v20, v21 + _other.v21, v22 + _other.v22, v23 + _other.v23,
						v30 + _other.v30, v31 + _other.v31, v32 + _other.v32, v33 + _other.v33
				);
			}

			Mat4<T> operator-(const Mat4<T>& _other) const
			{
				return Mat4<T>(
						v00 - _other.v00, v01 - _other.v01, v02 - _other.v02, v03 - _other.v03,
						v10 - _other.v10, v11 - _other.v11, v12 - _other.v12, v13 - _other.v13,
						v20 - _other.v20, v21 - _other.v21, v22 - _other.v22, v23 - _other.v23,
						v30 - _other.v30, v31 - _other.v31, v32 - _other.v32, v33 - _other.v33
				);
			}
	};

	template<typename T = float>
	class Mat3 : public Mat<T, 3>
	{
		public:
			T& v00 = this->m_data[0];
			T& v01 = this->m_data[1];
			T& v02 = this->m_data[2];
			T& v10 = this->m_data[3];
			T& v11 = this->m_data[4];
			T& v12 = this->m_data[5];
			T& v20 = this->m_data[6];
			T& v21 = this->m_data[7];
			T& v22 = this->m_data[8];

			Mat3()
				: Mat<T, 3>()
			{ }

			Mat3(T* _arr)
				: Mat<T, 3>()
			{
				memcpy(this->m_data, _arr, sizeof(T) * 9);
			}

			Mat3(
					T a00, T a01, T a02,
					T a10, T a11, T a12,
					T a20, T a21, T a22
			)
			{
				v00 = a00;
				v01 = a01;
				v02 = a02;
				v10 = a10;
				v11 = a11;
				v12 = a12;
				v20 = a20;
				v21 = a21;
				v22 = a22;
			}

			Mat3(const Renderer::Vec3<T>& _basis1, const Renderer::Vec3<T>& _basis2,
					const Renderer::Vec3<T>& _basis3)
			{
				v00 = _basis1.x;
				v10 = _basis1.y;
				v20 = _basis1.z;
				v01 = _basis2.x;
				v11 = _basis2.y;
				v21 = _basis2.z;
				v02 = _basis3.x;
				v12 = _basis3.y;
				v22 = _basis3.z;
			}

			Mat3(const Mat3<T>& _other)
			{
				memcpy(this->m_data, _other.m_data, sizeof(T) * 9);
			}

			T determinant() const
			{
				return v00 * (v11 * v22 - v12 * v21) -
					v01 * (v10 * v22 - v12 * v20) +
					v02 * (v10 * v21 - v11 * v20);
			}

			const T* operator*() const override
			{
				return &(this->m_data[0]);
			}

			void inverse()
			{
				T get_det = determinant();
				if(get_det == 0)
					throw Renderer::InvalidOperationException("Mat3 matrix det=0. Cannot take inverse!");

				T inv_det = ((T)1.f) / get_det;

				T cpy_data[9];
				memcpy(cpy_data, this->m_data, sizeof(T) * 9);

				for(int i=0;i<3;++i)
				{
					for(int j=0;j<3;++j)
					{
						T sign = (i + j) % 2 == 1 ? -1 : 1;

						int index_t = i != 0 ? 0 : 1;
						int index_b = i != 1 && index_t != 1 ? 1 : 2;
						int index_l = j != 0 ? 0 : 1;
						int index_r = j != 1 && index_l != 1 ? 1 : 2;
						// a * b - c * d
						T component_a = cpy_data[index_t * 3 + index_l];
						T component_b = cpy_data[index_b * 3 + index_r];
						T component_c = cpy_data[index_t * 3 + index_r];
						T component_d = cpy_data[index_b * 3 + index_l];

						// j as row i as col -> transpose
						this->m_data[j * 3 + i] = inv_det * sign *
							(component_a * component_b - component_c * component_d);
					}
				}
			}

			Mat3<T>& operator=(Mat3<T> _other)
			{
				if(&_other == this)
					return *this;

				memcpy(this->m_data, _other.m_data, sizeof(T) * 9);

				return *this;
			}

			Vec3<T> operator*(const Vec3<T>& _other) const
			{
				return Vec3<T>(
						_other.x * v00 + _other.y * v01 + _other.z * v02,
						_other.x * v10 + _other.y * v11 + _other.z * v12,
						_other.x * v20 + _other.y * v21 + _other.z * v22
				);
			}

			Mat3<T> operator*(const Mat3<T>& _other) const
			{
				Mat3<T> new_matrix;
				new_matrix.v00 = v00 * _other.v00 + v01 * _other.v10 + v02 * _other.v20;
				new_matrix.v01 = v00 * _other.v01 + v01 * _other.v11 + v02 * _other.v21;
				new_matrix.v02 = v00 * _other.v02 + v01 * _other.v12 + v02 * _other.v22;

				new_matrix.v10 = v10 * _other.v00 + v11 * _other.v10 + v12 * _other.v20;
				new_matrix.v11 = v10 * _other.v01 + v11 * _other.v11 + v12 * _other.v21;
				new_matrix.v12 = v10 * _other.v02 + v11 * _other.v12 + v12 * _other.v22;

				new_matrix.v20 = v20 * _other.v00 + v21 * _other.v10 + v22 * _other.v20;
				new_matrix.v21 = v20 * _other.v01 + v21 * _other.v11 + v22 * _other.v21;
				new_matrix.v22 = v20 * _other.v02 + v21 * _other.v12 + v22 * _other.v22;

				return new_matrix;
			}

			Mat3<T> operator+(const Mat3<T>& _other) const
			{
				return Mat3<T>(
						v00 + _other.v00, v01 + _other.v01, v02 + _other.v02,
						v10 + _other.v10, v11 + _other.v11, v12 + _other.v12,
						v20 + _other.v20, v21 + _other.v21, v22 + _other.v22
				);
			}

			Mat3<T> operator-(const Mat3<T>& _other) const
			{
				return Mat3<T>(
						v00 - _other.v00, v01 - _other.v01, v02 - _other.v02,
						v10 - _other.v10, v11 - _other.v11, v12 - _other.v12,
						v20 - _other.v20, v21 - _other.v21, v22 - _other.v22
				);
			}
	};

	template<typename T = float>
	class Mat2 : public Mat<T, 2>
	{
		public:
			T& v00 = this->m_data[0];
			T& v01 = this->m_data[1];
			T& v10 = this->m_data[2];
			T& v11 = this->m_data[3];

			Mat2()
				: Mat<T, 2>()
			{ }

			Mat2(T* _arr)
				: Mat<T, 2>()
			{
				memcpy(this->m_data, _arr, sizeof(T) * 4);
			}

			Mat2(
					T a00, T a01,
					T a10, T a11
			)
			{
				v00 = a00;
				v01 = a01;
				v10 = a10;
				v11 = a11;
			}

			Mat2(const Renderer::Vec2<T>& _basis1, const Renderer::Vec2<T>& _basis2)
			{
				v00 = _basis1.x;
				v10 = _basis1.y;
				v01 = _basis2.x;
				v11 = _basis2.y;
			}

			Mat2(const Mat2<T>& _other)
			{
				memcpy(this->m_data, _other.m_data, sizeof(T) * 4);
			}

			const T* operator*() const override
			{
				return &(this->m_data[0]);
			}

			T determinant() const
			{
				return (v00 * v11) - (v01 * v10);
			}

			void inverse()
			{
				T get_det = determinant();
				if(get_det == 0)
					throw Renderer::InvalidOperationException("Mat2 matrix det=0. Cannot take inverse!");

				T inv_det = ((T)1.f) / get_det;

				T v00_cpy = v00;

				v00 = v11 * inv_det;
				v11 = v00_cpy * inv_det;
				v10 *= -inv_det;
				v01 *= -inv_det;
			}

			Mat2<T>& operator=(Mat2<T> _other)
			{
				if(&_other == this)
					return *this;

				memcpy(this->m_data, _other.m_data, sizeof(T) * 4);

				return *this;
			}

			Vec2<T> operator*(const Vec2<T>& _other) const
			{
				return Vec2<T>(_other.x * v00 + _other.y * v01, _other.x * v10 + _other.y * v11);
			}

			Mat2<T> operator*(const Mat2<T>& _other) const
			{
				Mat2<T> new_matrix;
				new_matrix.v00 = v00 * _other.v00 + v01 * _other.v10;
				new_matrix.v01 = v00 * _other.v01 + v01 * _other.v11;
				new_matrix.v10 = v10 * _other.v00 + v11 * _other.v10;
				new_matrix.v11 = v10 * _other.v01 + v11 * _other.v11;

				return new_matrix;
			}

			Mat2<T> operator+(const Mat2<T>& _other) const
			{
				return Mat2<T>(
						v00 + _other.v00, v01 + _other.v01,
						v10 + _other.v10, v11 + _other.v11
				);
			}

			Mat2<T> operator-(const Mat2<T>& _other) const
			{
				return Mat2<T>(
						v00 - _other.v00, v01 - _other.v01,
						v10 - _other.v10, v11 - _other.v11
				);
			}
	};

	namespace Math
	{
		template<typename T>
		Mat4<T> projection2D(T _l, T _r, T _t, T _b, T _n, T _f)
		{
			return Mat4<T>(
				((T) 2) / (_r - _l), 0, 0, -(_r + _l)/(_r - _l),
				0, ((T) 2) / (_t - _b), 0, -(_t + _b)/(_t - _b),
				0, 0, ((T) -2) / (_f - _n), -(_f + _n)/(_f - _n),
				0, 0, 0, 1
			);
		}
	}
}
