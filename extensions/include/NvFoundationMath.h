// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.


#ifndef NV_FOUNDATION_MATH_H
#define NV_FOUNDATION_MATH_H

#include "NvFoundation.h"

#ifndef NV_DOXYGEN
namespace nvidia
{
#endif


//***************************************
// FILE: Nv.h
//***************************************

class NvVec2;
class NvVec3;
class NvVec4;
class NvMat33;
class NvMat44;
class NvQuat;
class NvTransform;
class NvBounds3;

class NvAllocatorCallback;
class NvErrorCallback;

class NvFoundation;

//***************************************
// FILE: NvMath.h
//***************************************

	/** enum for zero constructor tag for vectors and matrices */
	enum NvZERO			{	NvZero		};

	/** enum for identity constructor flag for quaternions, transforms, and matrices */
	enum NvIDENTITY		{	NvIdentity	};


	// constants
	static const float NvPi			=	float(3.141592653589793);
	static const float NvHalfPi		=	float(1.57079632679489661923);
	static const float NvTwoPi			=	float(6.28318530717958647692);
	static const float NvInvPi			=	float(0.31830988618379067154);
	static const float NvInvTwoPi		=   float(0.15915494309189533577);
	static const float NvPiDivTwo		=   float(1.57079632679489661923);
	static const float NvPiDivFour		=   float(0.78539816339744830962);

	/**
	* Select float b or c based on whether a is >= 0
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvFsel(float a, float b, float c)	{	return (a >= 0.0f) ? b : c;	}

	/**
	\brief The return value is the greater of the two specified values.
	*/
	template<class T>
	NV_CUDA_CALLABLE NV_FORCE_INLINE T NvMax(T a, T b)							{ return a<b ? b : a;	}

	/**
	\brief The return value is the lesser of the two specified values. 
	*/
	template<class T>
	NV_CUDA_CALLABLE NV_FORCE_INLINE T NvMin(T a, T b)							{ return a<b ? a : b;	}

	/*
	Many of these are just implemented as NV_CUDA_CALLABLE NV_FORCE_INLINE calls to the C lib right now,
	but later we could replace some of them with some approximations or more
	clever stuff.
	*/

	/**
	\brief abs returns the absolute value of its argument. 
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvAbs(float a)						{ return fabsf(a);	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvEquals(float a, float b,float eps)	{ return (NvAbs(a - b) < eps);	}

	/**
	\brief abs returns the absolute value of its argument. 
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvAbs(double a)						{ return ::fabs(a);	}

	/**
	\brief abs returns the absolute value of its argument. 
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE int32_t NvAbs(int32_t a)						{ return ::abs(a);	}

	/**
	\brief Clamps v to the range [hi,lo]
	*/
	template<class T>
	NV_CUDA_CALLABLE NV_FORCE_INLINE T NvClamp(T v, T lo, T hi)					{ NV_ASSERT(lo<=hi); return NvMin(hi, NvMax(lo, v)); }

	//!	\brief Square root.
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvSqrt(float a)						{ return sqrtf(a);		}

	//!	\brief Square root.
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvSqrt(double a)						{ return ::sqrt(a);	}

	//!	\brief reciprocal square root.
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvRecipSqrt(float a)					{ return 1.0f / ::sqrtf(a);	}

	//!	\brief reciprocal square root.
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvRecipSqrt(double a)					{ return 1/::sqrt(a);	}

	//!trigonometry -- all angles are in radians.

	//!	\brief Sine of an angle ( <b>Unit:</b> Radians )
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvSin(float a)						{ return ::sinf(a);				}

	//!	\brief Sine of an angle ( <b>Unit:</b> Radians )
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvSin(double a)						{ return ::sin(a);							}

	//!	\brief Cosine of an angle (<b>Unit:</b> Radians)
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvCos(float a)						{ return ::cosf(a);				}

	//!	\brief Cosine of an angle (<b>Unit:</b> Radians)
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvCos(double a)						{ return ::cos(a);							}

	/**
	\brief Tangent of an angle.
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvTan(float a)						{ return ::tan(a);							}

	/**
	\brief Tangent of an angle.
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvTan(double a)						{ return ::tan(a);							}

	/**
	\brief Arcsine.
	Returns angle between -PI/2 and PI/2 in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvAsin(float f)						{ return ::asin(NvClamp(f,-1.0f,1.0f));	}

	/**
	\brief Arcsine.
	Returns angle between -PI/2 and PI/2 in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvAsin(double f)						{ return ::asin(NvClamp(f,-1.0,1.0));		}

	/**
	\brief Arccosine.
	Returns angle between 0 and PI in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvAcos(float f)						{ return ::acos(NvClamp(f,-1.0f,1.0f));			}

	/**
	\brief Arccosine.
	Returns angle between 0 and PI in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvAcos(double f)						{ return ::acos(NvClamp(f,-1.0,1.0));				}

	/**
	\brief ArcTangent.
	Returns angle between -PI/2 and PI/2 in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvAtan(float a)						{ return ::atan(a);	}

	/**
	\brief ArcTangent.
	Returns angle between -PI/2 and PI/2 in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvAtan(double a)						{ return ::atan(a);	}

	/**
	\brief Arctangent of (x/y) with correct sign.
	Returns angle between -PI and PI in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvAtan2(float x, float y)			{ return ::atan2(x,y);	}

	/**
	\brief Arctangent of (x/y) with correct sign.
	Returns angle between -PI and PI in radians
	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE double NvAtan2(double x, double y)			{ return ::atan2(x,y);	}

	//!	\brief returns true if the passed number is a finite floating point number as opposed to INF, NAN, etc.
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvIsFinite(float a)					
	{ 
#ifdef _MSC_VER
		return (0 == ((_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF) & _fpclass(a) ));
#else
		return isfinite(a);
#endif
	}

	//!	\brief returns true if the passed number is a finite floating point number as opposed to INF, NAN, etc.
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvIsFinite(double a)					
	{ 
#ifdef _MSC_VER
		return (0 == ((_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF) & _fpclass(a) ));
#else
		return isfinite(a);
#endif
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvFloor(float a)	{ return ::floorf(a);					}

	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvExp(float a)						{ return ::expf(a);	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvCeil(float a)						{ return ::ceilf(a);	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvSign(float a)						{ return (a >= 0.0f) ? 1.0f : -1.0f; }

	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvPow(float x,float y)				{ return ::powf(x,y); };

	NV_CUDA_CALLABLE NV_FORCE_INLINE float NvLog(float x)						{ return ::log(x); };

// **********************************************
// FILE: NvVec2.h
// **********************************************

/**
\brief 2 Element vector class.

This is a 2-dimensional vector class with public data members.
*/
class NvVec2
{
public:

	/**
	\brief default constructor leaves data uninitialized.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2() {}


	/**
	\brief zero constructor.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2(NvZERO r): x(0.0f), y(0.0f) 
	{
		NV_UNUSED(r);
	}

	/**
	\brief Assigns scalar parameter to all elements.

	Useful to initialize to zero or one.

	\param[in] a Value to assign to elements.
	*/
	explicit NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2(float a): x(a), y(a) {}

	/**
	\brief Initializes from 2 scalar parameters.

	\param[in] nx Value to initialize X component.
	\param[in] ny Value to initialize Y component.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2(float nx, float ny): x(nx), y(ny){}

	/**
	\brief Copy ctor.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2(const NvVec2& v): x(v.x), y(v.y) {}

	//Operators

	/**
	\brief Assignment operator
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE	NvVec2&	operator=(const NvVec2& p)			{ x = p.x; y = p.y;	return *this;		}

	/**
	\brief element access
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float& operator[](int index)
	{
		NV_ASSERT(index>=0 && index<=1);

		return reinterpret_cast<float*>(this)[index];
	}

	/**
	\brief element access
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE const float& operator[](int index) const
	{
		NV_ASSERT(index>=0 && index<=1);

		return reinterpret_cast<const float*>(this)[index];
	}

	/**
	\brief returns true if the two vectors are exactly equal.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool operator==(const NvVec2&v) const	{ return x == v.x && y == v.y; }

	/**
	\brief returns true if the two vectors are not exactly equal.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool operator!=(const NvVec2&v) const	{ return x != v.x || y != v.y; }

	/**
	\brief tests for exact zero vector
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isZero()	const					{ return x==0.0f && y==0.0f;			}

	/**
	\brief returns true if all 2 elems of the vector are finite (not NAN or INF, etc.)
	*/
	NV_CUDA_CALLABLE NV_INLINE bool isFinite() const
	{
		return NvIsFinite(x) && NvIsFinite(y);
	}

	/**
	\brief is normalized - used by API parameter validation
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isNormalized() const
	{
		const float unitTolerance = 1e-4f;
		return isFinite() && NvAbs(magnitude()-1)<unitTolerance;
	}

	/**
	\brief returns the squared magnitude

	Avoids calling NvSqrt()!
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float magnitudeSquared() const		{	return x * x + y * y;					}

	/**
	\brief returns the magnitude
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float magnitude() const				{	return NvSqrt(magnitudeSquared());		}

	/**
	\brief negation
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 operator -() const
	{
		return NvVec2(-x, -y);
	}

	/**
	\brief vector addition
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 operator +(const NvVec2& v) const		{	return NvVec2(x + v.x, y + v.y);	}

	/**
	\brief vector difference
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 operator -(const NvVec2& v) const		{	return NvVec2(x - v.x, y - v.y);	}

	/**
	\brief scalar post-multiplication
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 operator *(float f) const				{	return NvVec2(x * f, y * f);			}

	/**
	\brief scalar division
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 operator /(float f) const
	{
		f = 1.0f / f;	// PT: inconsistent notation with operator /=
		return NvVec2(x * f, y * f);
	}

	/**
	\brief vector addition
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2& operator +=(const NvVec2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}
	
	/**
	\brief vector difference
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2& operator -=(const NvVec2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	/**
	\brief scalar multiplication
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2& operator *=(float f)
	{
		x *= f;
		y *= f;
		return *this;
	}
	/**
	\brief scalar division
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2& operator /=(float f)
	{
		f = 1.0f/f;	// PT: inconsistent notation with operator /
		x *= f;
		y *= f;
		return *this;
	}

	/**
	\brief returns the scalar product of this and other.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float dot(const NvVec2& v) const		
	{
		return x * v.x + y * v.y;				
	}

	/** return a unit vector */

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 getNormalized() const
	{
		const float m = magnitudeSquared();
		return m>0.0f ? *this * NvRecipSqrt(m) : NvVec2(0,0);
	}

	/**
	\brief normalizes the vector in place
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float normalize()
	{
		const float m = magnitude();
		if (m>0.0f) 
			*this /= m;
		return m;
	}

	/**
	\brief a[i] * b[i], for all i.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 multiply(const NvVec2& a) const
	{
		return NvVec2(x*a.x, y*a.y);
	}

	/**
	\brief element-wise minimum
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 minimum(const NvVec2& v) const
	{ 
		return NvVec2(NvMin(x, v.x), NvMin(y,v.y));	
	}

	/**
	\brief returns MIN(x, y);
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float minElement()	const
	{
		return NvMin(x, y);
	}
	
	/**
	\brief element-wise maximum
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec2 maximum(const NvVec2& v) const
	{ 
		return NvVec2(NvMax(x, v.x), NvMax(y,v.y));	
	} 

	/**
	\brief returns MAX(x, y);
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float maxElement()	const
	{
		return NvMax(x, y);
	}

	float x,y;
};

NV_CUDA_CALLABLE static NV_FORCE_INLINE NvVec2 operator *(float f, const NvVec2& v)
{
	return NvVec2(f * v.x, f * v.y);
}

// **************************************
// FILE: NvVec3.h
// **************************************

/**
\brief 3 Element vector class.

This is a 3-dimensional vector class with public data members.
*/
class NvVec3
{
public:

	/**
	\brief default constructor leaves data uninitialized.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3() {}

	/**
	\brief zero constructor.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3(NvZERO r): x(0.0f), y(0.0f), z(0.0f) 
	{
		NV_UNUSED(r);
	}


	/**
	\brief Assigns scalar parameter to all elements.

	Useful to initialize to zero or one.

	\param[in] a Value to assign to elements.
	*/
	explicit NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3(float a): x(a), y(a), z(a) {}

	/**
	\brief Initializes from 3 scalar parameters.

	\param[in] nx Value to initialize X component.
	\param[in] ny Value to initialize Y component.
	\param[in] nz Value to initialize Z component.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3(float nx, float ny, float nz): x(nx), y(ny), z(nz) {}

	/**
	\brief Copy ctor.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3(const NvVec3& v): x(v.x), y(v.y), z(v.z) {}

	//Operators

	/**
	\brief Assignment operator
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE	NvVec3&	operator=(const NvVec3& p)			{ x = p.x; y = p.y; z = p.z;	return *this;		}

	/**
	\brief element access
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float& operator[](int index)
	{
		NV_ASSERT(index>=0 && index<=2);

		return reinterpret_cast<float*>(this)[index];
	}
	
	NV_CUDA_CALLABLE NV_FORCE_INLINE float& operator[](unsigned int index)
	{
		NV_ASSERT(index<=2);

		return reinterpret_cast<float*>(this)[index];
	}

	/**
	\brief element access
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE const float& operator[](int index) const
	{
		NV_ASSERT(index>=0 && index<=2);

		return reinterpret_cast<const float*>(this)[index];
	}
	
	NV_CUDA_CALLABLE NV_FORCE_INLINE const float& operator[](unsigned int index) const
	{
		NV_ASSERT(index<=2);

		return reinterpret_cast<const float*>(this)[index];
	}
	/**
	\brief returns true if the two vectors are exactly equal.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool operator==(const NvVec3&v) const	{ return x == v.x && y == v.y && z == v.z; }

	/**
	\brief returns true if the two vectors are not exactly equal.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool operator!=(const NvVec3&v) const	{ return x != v.x || y != v.y || z != v.z; }

	/**
	\brief tests for exact zero vector
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isZero()	const					{ return x==0.0f && y==0.0f && z == 0.0f;			}

	/**
	\brief returns true if all 3 elems of the vector are finite (not NAN or INF, etc.)
	*/
	NV_CUDA_CALLABLE NV_INLINE bool isFinite() const
	{
		return NvIsFinite(x) && NvIsFinite(y) && NvIsFinite(z);
	}

	/**
	\brief is normalized - used by API parameter validation
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isNormalized() const
	{
		const float unitTolerance = 1e-4f;
		return isFinite() && NvAbs(magnitude()-1)<unitTolerance;
	}

	/**
	\brief returns the squared magnitude

	Avoids calling NvSqrt()!
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float magnitudeSquared() const		{	return x * x + y * y + z * z;					}

	/**
	\brief returns the magnitude
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float magnitude() const				{	return NvSqrt(magnitudeSquared());		}

	/**
	\brief negation
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 operator -() const
	{
		return NvVec3(-x, -y, -z);
	}

	/**
	\brief vector addition
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 operator +(const NvVec3& v) const		{	return NvVec3(x + v.x, y + v.y, z + v.z);	}

	/**
	\brief vector difference
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 operator -(const NvVec3& v) const		{	return NvVec3(x - v.x, y - v.y, z - v.z);	}

	/**
	\brief scalar post-multiplication
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 operator *(float f) const				{	return NvVec3(x * f, y * f, z * f);			}

	/**
	\brief scalar division
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 operator /(float f) const
	{
		f = 1.0f / f;
		return NvVec3(x * f, y * f, z * f);
	}

	/**
	\brief vector addition
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3& operator +=(const NvVec3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	
	/**
	\brief vector difference
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3& operator -=(const NvVec3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	/**
	\brief scalar multiplication
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3& operator *=(float f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	/**
	\brief scalar division
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3& operator /=(float f)
	{
		f = 1.0f/f;
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	/**
	\brief returns the scalar product of this and other.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float dot(const NvVec3& v) const		
	{	
		return x * v.x + y * v.y + z * v.z;				
	}

	/**
	\brief cross product
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 cross(const NvVec3& v) const
	{
		return NvVec3(y * v.z - z * v.y,
					  z * v.x - x * v.z, 
					  x * v.y - y * v.x);
	}

	/** return a unit vector */

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getNormalized() const
	{
		const float m = magnitudeSquared();
		return m>0.0f ? *this * NvRecipSqrt(m) : NvVec3(0,0,0);
	}

	/**
	\brief normalizes the vector in place
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float normalize()
	{
		const float m = magnitude();
		if (m>0.0f) 
			*this /= m;
		return m;
	}

	/**
	\brief normalizes the vector in place. Does nothing if vector magnitude is under NV_NORMALIZATION_EPSILON.
	Returns vector magnitude if >= NV_NORMALIZATION_EPSILON and 0.0f otherwise.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float normalizeSafe()
	{
		const float mag = magnitude();
		if (mag < NV_NORMALIZATION_EPSILON)
			return 0.0f;
		*this *= 1.0f / mag;
		return mag;
	}

	/**
	\brief normalizes the vector in place. Asserts if vector magnitude is under NV_NORMALIZATION_EPSILON.
	returns vector magnitude.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float normalizeFast()
	{
		const float mag = magnitude();
		NV_ASSERT(mag >= NV_NORMALIZATION_EPSILON);
		*this *= 1.0f / mag;
		return mag;
	}

	/**
	\brief a[i] * b[i], for all i.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 multiply(const NvVec3& a) const
	{
		return NvVec3(x*a.x, y*a.y, z*a.z);
	}

	/**
	\brief element-wise minimum
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 minimum(const NvVec3& v) const
	{ 
		return NvVec3(NvMin(x, v.x), NvMin(y,v.y), NvMin(z,v.z));	
	}

	/**
	\brief returns MIN(x, y, z);
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float minElement()	const
	{
		return NvMin(x, NvMin(y, z));
	}
	
	/**
	\brief element-wise maximum
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 maximum(const NvVec3& v) const
	{ 
		return NvVec3(NvMax(x, v.x), NvMax(y,v.y), NvMax(z,v.z));	
	} 

	/**
	\brief returns MAX(x, y, z);
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float maxElement()	const
	{
		return NvMax(x, NvMax(y, z));
	}

	/**
	\brief returns absolute values of components;
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 abs()	const
	{
		return NvVec3(NvAbs(x), NvAbs(y), NvAbs(z));
	}


	float x,y,z;
};

NV_CUDA_CALLABLE static NV_FORCE_INLINE NvVec3 operator *(float f, const NvVec3& v)
{
	return NvVec3(f * v.x, f * v.y, f * v.z);
}

// ****************************************
// FILE: NvVec4.h
// ****************************************

/**
\brief 4 Element vector class.

This is a 4-dimensional vector class with public data members.
*/

class NvVec4
{
public:

	/**
	\brief default constructor leaves data uninitialized.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4() {}

	/**
	\brief zero constructor.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec4(NvZERO r): x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{
		NV_UNUSED(r);
	}

	/**
	\brief Assigns scalar parameter to all elements.

	Useful to initialize to zero or one.

	\param[in] a Value to assign to elements.
	*/
	explicit NV_CUDA_CALLABLE NV_INLINE NvVec4(float a): x(a), y(a), z(a), w(a) {}

	/**
	\brief Initializes from 3 scalar parameters.

	\param[in] nx Value to initialize X component.
	\param[in] ny Value to initialize Y component.
	\param[in] nz Value to initialize Z component.
	\param[in] nw Value to initialize W component.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4(float nx, float ny, float nz, float nw): x(nx), y(ny), z(nz), w(nw) {}


	/**
	\brief Initializes from 3 scalar parameters.

	\param[in] v Value to initialize the X, Y, and Z components.
	\param[in] nw Value to initialize W component.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4(const NvVec3& v, float nw): x(v.x), y(v.y), z(v.z), w(nw) {}


	/**
	\brief Initializes from an array of scalar parameters.

	\param[in] v Value to initialize with.
	*/
	explicit NV_CUDA_CALLABLE NV_INLINE NvVec4(const float v[]): x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

	/**
	\brief Copy ctor.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4(const NvVec4& v): x(v.x), y(v.y), z(v.z), w(v.w) {}

	//Operators

	/**
	\brief Assignment operator
	*/
	NV_CUDA_CALLABLE NV_INLINE	NvVec4&	operator=(const NvVec4& p)			{ x = p.x; y = p.y; z = p.z; w = p.w;	return *this;		}

	/**
	\brief element access
	*/
	NV_CUDA_CALLABLE NV_INLINE float& operator[](int index)
	{
		NV_ASSERT(index>=0 && index<=3);

		return reinterpret_cast<float*>(this)[index];
	}
	
	NV_CUDA_CALLABLE NV_INLINE float& operator[](unsigned int index)
	{
		NV_ASSERT(index<=3);

		return reinterpret_cast<float*>(this)[index];
	}
	/**
	\brief element access
	*/
	NV_CUDA_CALLABLE NV_INLINE const float& operator[](int index) const
	{
		NV_ASSERT(index>=0 && index<=3);

		return reinterpret_cast<const float*>(this)[index];
	}
	NV_CUDA_CALLABLE NV_INLINE const float& operator[](unsigned int index) const
	{
		NV_ASSERT(index<=3);

		return reinterpret_cast<const float*>(this)[index];
	}

	/**
	\brief returns true if the two vectors are exactly equal.
	*/
	NV_CUDA_CALLABLE NV_INLINE bool operator==(const NvVec4&v) const	{ return x == v.x && y == v.y && z == v.z && w == v.w; }

	/**
	\brief returns true if the two vectors are not exactly equal.
	*/
	NV_CUDA_CALLABLE NV_INLINE bool operator!=(const NvVec4&v) const	{ return x != v.x || y != v.y || z != v.z || w!= v.w; }

	/**
	\brief tests for exact zero vector
	*/
	NV_CUDA_CALLABLE NV_INLINE bool isZero()	const					{ return x==0 && y==0 && z == 0 && w == 0;			}

	/**
	\brief returns true if all 3 elems of the vector are finite (not NAN or INF, etc.)
	*/
	NV_CUDA_CALLABLE NV_INLINE bool isFinite() const
	{
		return NvIsFinite(x) && NvIsFinite(y) && NvIsFinite(z) && NvIsFinite(w);
	}

	/**
	\brief is normalized - used by API parameter validation
	*/
	NV_CUDA_CALLABLE NV_INLINE bool isNormalized() const
	{
		const float unitTolerance = 1e-4f;
		return isFinite() && NvAbs(magnitude()-1)<unitTolerance;
	}


	/**
	\brief returns the squared magnitude

	Avoids calling NvSqrt()!
	*/
	NV_CUDA_CALLABLE NV_INLINE float magnitudeSquared() const		{	return x * x + y * y + z * z + w * w;		}

	/**
	\brief returns the magnitude
	*/
	NV_CUDA_CALLABLE NV_INLINE float magnitude() const				{	return NvSqrt(magnitudeSquared());		}

	/**
	\brief negation
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 operator -() const
	{
		return NvVec4(-x, -y, -z, -w);
	}

	/**
	\brief vector addition
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 operator +(const NvVec4& v) const		{	return NvVec4(x + v.x, y + v.y, z + v.z, w + v.w);	}

	/**
	\brief vector difference
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 operator -(const NvVec4& v) const		{	return NvVec4(x - v.x, y - v.y, z - v.z, w - v.w);	}

	/**
	\brief scalar post-multiplication
	*/

	NV_CUDA_CALLABLE NV_INLINE NvVec4 operator *(float f) const				{	return NvVec4(x * f, y * f, z * f, w * f);		}

	/**
	\brief scalar division
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 operator /(float f) const
	{
		f = 1.0f / f; 
		return NvVec4(x * f, y * f, z * f, w * f);
	}

	/**
	\brief vector addition
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4& operator +=(const NvVec4& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}
	
	/**
	\brief vector difference
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4& operator -=(const NvVec4& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	/**
	\brief scalar multiplication
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4& operator *=(float f)
	{
		x *= f;
		y *= f;
		z *= f;
		w *= f;
		return *this;
	}
	/**
	\brief scalar division
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4& operator /=(float f)
	{
		f = 1.0f/f;
		x *= f;
		y *= f;
		z *= f;
		w *= f;
		return *this;
	}

	/**
	\brief returns the scalar product of this and other.
	*/
	NV_CUDA_CALLABLE NV_INLINE float dot(const NvVec4& v) const		
	{	
		return x * v.x + y * v.y + z * v.z + w * v.w;				
	}

	/** return a unit vector */

	NV_CUDA_CALLABLE NV_INLINE NvVec4 getNormalized() const
	{
		float m = magnitudeSquared();
		return m>0.0f ? *this * NvRecipSqrt(m) : NvVec4(0,0,0,0);
	}


	/**
	\brief normalizes the vector in place
	*/
	NV_CUDA_CALLABLE NV_INLINE float normalize()
	{
		float m = magnitude();
		if (m>0.0f) 
			*this /= m;
		return m;
	}

	/**
	\brief a[i] * b[i], for all i.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 multiply(const NvVec4& a) const
	{
		return NvVec4(x*a.x, y*a.y, z*a.z, w*a.w);
	}

	/**
	\brief element-wise minimum
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 minimum(const NvVec4& v) const
	{
		return NvVec4(NvMin(x, v.x), NvMin(y,v.y), NvMin(z,v.z), NvMin(w,v.w));	
	}

	/**
	\brief element-wise maximum
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec4 maximum(const NvVec4& v) const
	{ 
		return NvVec4(NvMax(x, v.x), NvMax(y,v.y), NvMax(z,v.z), NvMax(w,v.w));	
	} 

	NV_CUDA_CALLABLE NV_INLINE NvVec3 getXYZ() const
	{
		return NvVec3(x,y,z);
	}

	/**
	\brief set vector elements to zero
	*/
	NV_CUDA_CALLABLE NV_INLINE void setZero() {	x = y = z = w = 0.0f; }

	float x,y,z,w;
};


NV_CUDA_CALLABLE static NV_INLINE NvVec4 operator *(float f, const NvVec4& v)
{
	return NvVec4(f * v.x, f * v.y, f * v.z, f * v.w);
}

// ********************************
// FILE: NvQuat.h
// ********************************

/**
\brief This is a quaternion class. For more information on quaternion mathematics
consult a mathematics source on complex numbers.

*/

class NvQuat
{
public:
	/**
	\brief Default constructor, does not do any initialization.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat()	{	}


	//! identity constructor
	NV_CUDA_CALLABLE NV_INLINE NvQuat(NvIDENTITY r)
		: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{
		NV_UNUSED(r);
	}

	/**
	\brief Constructor from a scalar: sets the real part w to the scalar value, and the imaginary parts (x,y,z) to zero
	*/
	explicit NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat(float r)
		: x(0.0f), y(0.0f), z(0.0f), w(r)
	{
	}

	/**
	\brief Constructor.  Take note of the order of the elements!
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat(float nx, float ny, float nz, float nw) : x(nx),y(ny),z(nz),w(nw) {}

	/**
	\brief Creates from angle-axis representation.

	Axis must be normalized!

	Angle is in radians!

	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_INLINE NvQuat(float angleRadians, const NvVec3& unitAxis)
	{
		NV_ASSERT(NvAbs(1.0f-unitAxis.magnitude())<1e-3f);
		const float a = angleRadians * 0.5f;
		const float s = NvSin(a);
		w = NvCos(a);
		x = unitAxis.x * s;
		y = unitAxis.y * s;
		z = unitAxis.z * s;
	}

	/**
	\brief Copy ctor.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat(const NvQuat& v): x(v.x), y(v.y), z(v.z), w(v.w) {}

	/**
	\brief Creates from orientation matrix.

	\param[in] m Rotation matrix to extract quaternion from.
	*/
	NV_CUDA_CALLABLE NV_INLINE explicit NvQuat(const NvMat33& m); /* defined in NvMat33.h */

	/**
	\brief returns true if all elements are finite (not NAN or INF, etc.)
	*/
	NV_CUDA_CALLABLE bool isFinite() const
	{
		return NvIsFinite(x) 
			&& NvIsFinite(y) 
			&& NvIsFinite(z)
			&& NvIsFinite(w);
	}


	/**
	\brief returns true if finite and magnitude is close to unit
	*/

	NV_CUDA_CALLABLE bool isUnit() const
	{
		const float unitTolerance = 1e-4f;
		return isFinite() && NvAbs(magnitude()-1)<unitTolerance;
	}


	/**
	\brief returns true if finite and magnitude is reasonably close to unit to allow for some accumulation of error vs isValid
	*/

	NV_CUDA_CALLABLE bool isSane() const
	{
	      const float unitTolerance = 1e-2f;
	      return isFinite() && NvAbs(magnitude()-1)<unitTolerance;
	}

	/**
	\brief converts this quaternion to angle-axis representation
	*/

	NV_CUDA_CALLABLE NV_INLINE void toRadiansAndUnitAxis(float& angle, NvVec3& axis) const
	{
		const float quatEpsilon = 1.0e-8f;
		const float s2 = x*x+y*y+z*z;
		if(s2<quatEpsilon*quatEpsilon)  // can't extract a sensible axis
		{
			angle = 0.0f;
			axis = NvVec3(1.0f,0.0f,0.0f);
		}
		else
		{
			const float s = NvRecipSqrt(s2);
			axis = NvVec3(x,y,z) * s; 
			angle = NvAbs(w)<quatEpsilon ? NvPi : NvAtan2(s2*s, w) * 2.0f;
		}

	}

	/**
	\brief Gets the angle between this quat and the identity quaternion.

	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_INLINE float getAngle() const
	{
		return NvAcos(w) * 2.0f;
	}


	/**
	\brief Gets the angle between this quat and the argument

	<b>Unit:</b> Radians
	*/
	NV_CUDA_CALLABLE NV_INLINE float getAngle(const NvQuat& q) const
	{
		return NvAcos(dot(q)) * 2.0f;
	}


	/**
	\brief This is the squared 4D vector length, should be 1 for unit quaternions.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float magnitudeSquared() const
	{
		return x*x + y*y + z*z + w*w;
	}

	/**
	\brief returns the scalar product of this and other.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float dot(const NvQuat& v) const
	{
		return x * v.x + y * v.y + z * v.z  + w * v.w;
	}

	NV_CUDA_CALLABLE NV_INLINE NvQuat getNormalized() const
	{
		const float s = 1.0f/magnitude();
		return NvQuat(x*s, y*s, z*s, w*s);
	}


	NV_CUDA_CALLABLE NV_INLINE float magnitude() const
	{
		return NvSqrt(magnitudeSquared());
	}

	//modifiers:
	/**
	\brief maps to the closest unit quaternion.
	*/
	NV_CUDA_CALLABLE NV_INLINE float normalize()											// convert this NvQuat to a unit quaternion
	{
		const float mag = magnitude();
		if (mag)
		{
			const float imag = 1.0f / mag;

			x *= imag;
			y *= imag;
			z *= imag;
			w *= imag;
		}
		return mag;
	}

	/*
	\brief returns the conjugate.

	\note for unit quaternions, this is the inverse.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvQuat getConjugate() const
	{
		return NvQuat(-x,-y,-z,w);
	}

	/*
	\brief returns imaginary part.
	*/
	NV_CUDA_CALLABLE NV_INLINE NvVec3 getImaginaryPart() const
	{
		return NvVec3(x,y,z);
	}

	/** brief computes rotation of x-axis */
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getBasisVector0()	const
	{	
//		return rotate(NvVec3(1,0,0));
		const float x2 = x*2.0f;
		const float w2 = w*2.0f;
		return NvVec3(	(w * w2) - 1.0f + x*x2,
						(z * w2)        + y*x2,
						(-y * w2)       + z*x2);
	}
	
	/** brief computes rotation of y-axis */
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getBasisVector1()	const 
	{	
//		return rotate(NvVec3(0,1,0));
		const float y2 = y*2.0f;
		const float w2 = w*2.0f;
		return NvVec3(	(-z * w2)       + x*y2,
						(w * w2) - 1.0f + y*y2,
						(x * w2)        + z*y2);
	}


	/** brief computes rotation of z-axis */
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getBasisVector2() const	
	{	
//		return rotate(NvVec3(0,0,1));
		const float z2 = z*2.0f;
		const float w2 = w*2.0f;
		return NvVec3(	(y * w2)        + x*z2,
						(-x * w2)       + y*z2,
						(w * w2) - 1.0f + z*z2);
	}

	/**
	rotates passed vec by this (assumed unitary)
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE const NvVec3 rotate(const NvVec3& v) const
//	NV_CUDA_CALLABLE NV_INLINE const NvVec3 rotate(const NvVec3& v) const
	{
		const float vx = 2.0f*v.x;
		const float vy = 2.0f*v.y;
		const float vz = 2.0f*v.z;
		const float w2 = w*w-0.5f;
		const float dot2 = (x*vx + y*vy +z*vz);
		return NvVec3
		(
			(vx*w2 + (y * vz - z * vy)*w + x*dot2), 
			(vy*w2 + (z * vx - x * vz)*w + y*dot2), 
			(vz*w2 + (x * vy - y * vx)*w + z*dot2)
		);
		/*
		const NvVec3 qv(x,y,z);
		return (v*(w*w-0.5f) + (qv.cross(v))*w + qv*(qv.dot(v)))*2;
		*/
	}

	/**
	inverse rotates passed vec by this (assumed unitary)
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE const NvVec3 rotateInv(const NvVec3& v) const
//	NV_CUDA_CALLABLE NV_INLINE const NvVec3 rotateInv(const NvVec3& v) const
	{
		const float vx = 2.0f*v.x;
		const float vy = 2.0f*v.y;
		const float vz = 2.0f*v.z;
		const float w2 = w*w-0.5f;
		const float dot2 = (x*vx + y*vy +z*vz);
		return NvVec3
		(
			(vx*w2 - (y * vz - z * vy)*w + x*dot2), 
			(vy*w2 - (z * vx - x * vz)*w + y*dot2), 
			(vz*w2 - (x * vy - y * vx)*w + z*dot2)
		);
//		const NvVec3 qv(x,y,z);
//		return (v*(w*w-0.5f) - (qv.cross(v))*w + qv*(qv.dot(v)))*2;
	}

	/**
	\brief Assignment operator
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE	NvQuat&	operator=(const NvQuat& p)			{ x = p.x; y = p.y; z = p.z; w = p.w;	return *this;		}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat& operator*= (const NvQuat& q)
	{
		const float tx = w*q.x + q.w*x + y*q.z - q.y*z;
		const float ty = w*q.y + q.w*y + z*q.x - q.z*x;
		const float tz = w*q.z + q.w*z + x*q.y - q.x*y;

		w = w*q.w - q.x*x - y*q.y - q.z*z;
		x = tx;
		y = ty;
		z = tz;

		return *this;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat& operator+= (const NvQuat& q)
	{
		x+=q.x;
		y+=q.y;
		z+=q.z;
		w+=q.w;
		return *this;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat& operator-= (const NvQuat& q)
	{
		x-=q.x;
		y-=q.y;
		z-=q.z;
		w-=q.w;
		return *this;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat& operator*= (const float s)
	{
		x*=s;
		y*=s;
		z*=s;
		w*=s;
		return *this;
	}

	/** quaternion multiplication */
	NV_CUDA_CALLABLE NV_INLINE NvQuat operator*(const NvQuat& q) const
	{
		return NvQuat(w*q.x + q.w*x + y*q.z - q.y*z,
					  w*q.y + q.w*y + z*q.x - q.z*x,
					  w*q.z + q.w*z + x*q.y - q.x*y,
					  w*q.w - x*q.x - y*q.y - z*q.z);
	}

	/** quaternion addition */
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat operator+(const NvQuat& q) const
	{
		return NvQuat(x+q.x,y+q.y,z+q.z,w+q.w);
	}

	/** quaternion subtraction */
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat operator-() const
	{
		return NvQuat(-x,-y,-z,-w);
	}


	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat operator-(const NvQuat& q) const
	{
		return NvQuat(x-q.x,y-q.y,z-q.z,w-q.w);
	}


	NV_CUDA_CALLABLE NV_FORCE_INLINE NvQuat operator*(float r) const
	{
		return NvQuat(x*r,y*r,z*r,w*r);
	}

	
	/** \deprecated use NvQuat(NvIdentity) */
	NV_DEPRECATED static NV_CUDA_CALLABLE NV_INLINE NvQuat createIdentity() { return NvQuat(NvIdentity); }

	/** the quaternion elements */
	float x,y,z,w;
};

// ******************************************
// FILE: NvPlane.h
// ******************************************

/**
\brief Representation of a plane.

 Plane equation used: n.dot(v) + d = 0
*/
class NvPlane
{
public:
	/**
	\brief Constructor
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane()
	{
	}

	/**
	\brief Constructor from a normal and a distance
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane(float nx, float ny, float nz, float distance)
		: n(nx, ny, nz)
		, d(distance)
	{
	}

	/**
	\brief Constructor from a normal and a distance
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane(const NvVec3& normal, float distance) 
		: n(normal)
		, d(distance)
	{
	}


	/**
	\brief Constructor from a point on the plane and a normal
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane(const NvVec3& point, const NvVec3& normal)
		: n(normal)		
		, d(-point.dot(n))		// p satisfies normal.dot(p) + d = 0
	{
	}

	/**
	\brief Constructor from three points
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane(const NvVec3& p0, const NvVec3& p1, const NvVec3& p2)
	{
		n = (p1 - p0).cross(p2 - p0).getNormalized();
		d = -p0.dot(n);
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE float distance(const NvVec3& p) const
	{
		return p.dot(n) + d;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE bool contains(const NvVec3& p) const
	{
		return NvAbs(distance(p)) < (1.0e-7f);
	}

	/**
	\brief projects p into the plane
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 project(const NvVec3 & p) const
	{
		return p - n * distance(p);
	}

	/**
	\brief find an arbitrary point in the plane
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 pointInPlane() const
	{
		return -n*d;
	}

	/**
	\brief equivalent plane with unit normal
	*/

	NV_CUDA_CALLABLE NV_FORCE_INLINE void normalize()
	{
		float denom = 1.0f / n.magnitude();
		n *= denom;
		d *= denom;
	}


	NvVec3	n;			//!< The normal to the plane
	float	d;			//!< The distance from the origin
};

// ***************************************
// FILE: NvTransform.h
// ***************************************

/*!
\brief class representing a rigid euclidean transform as a quaternion and a vector
*/

class NvTransform
{
public:
	NvQuat q;
	NvVec3 p;

//#define PXTRANSFORM_DEFAULT_CONSTRUCT_NAN

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform() 
#ifdef PXTRANSFORM_DEFAULT_CONSTRUCT_IDENTITY
		: q(0, 0, 0, 1), p(0, 0, 0)
#elif defined(PXTRANSFORM_DEFAULT_CONSTRUCT_NAN)
#define invalid NvSqrt(-1.0f)
		: q(invalid, invalid, invalid, invalid), p(invalid, invalid, invalid)
#undef invalid
#endif
	{
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE explicit NvTransform(const NvVec3& position): q(NvIdentity), p(position)
	{
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE explicit NvTransform(NvIDENTITY r)
		: q(NvIdentity), p(NvZero)
	{
		NV_UNUSED(r);
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE explicit NvTransform(const NvQuat& orientation): q(orientation), p(0)
	{
		NV_ASSERT(orientation.isSane());
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform(float x, float y, float z, NvQuat aQ = NvQuat(NvIdentity))
		: q(aQ), p(x, y, z)
	{
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform(const NvVec3& p0, const NvQuat& q0): q(q0), p(p0) 
	{
		NV_ASSERT(q0.isSane());
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE explicit NvTransform(const NvMat44& m);	// defined in NvMat44.h
	
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform operator*(const NvTransform& x) const
	{
		NV_ASSERT(x.isSane());
		return transform(x);
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform getInverse() const
	{
		NV_ASSERT(isFinite());
		return NvTransform(q.rotateInv(-p),q.getConjugate());
	}


	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 transform(const NvVec3& input) const
	{
		NV_ASSERT(isFinite());
		return q.rotate(input) + p;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 transformInv(const NvVec3& input) const
	{
		NV_ASSERT(isFinite());
		return q.rotateInv(input-p);
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 rotate(const NvVec3& input) const
	{
		NV_ASSERT(isFinite());
		return q.rotate(input);
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 rotateInv(const NvVec3& input) const
	{
		NV_ASSERT(isFinite());
		return q.rotateInv(input);
	}

	//! Transform transform to parent (returns compound transform: first src, then *this)
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform transform(const NvTransform& src) const
	{
		NV_ASSERT(src.isSane());
		NV_ASSERT(isSane());
		// src = [srct, srcr] -> [r*srct + t, r*srcr]
		return NvTransform(q.rotate(src.p) + p, q*src.q);
	}

	/**
	\brief returns true if finite and q is a unit quaternion
	*/

	NV_CUDA_CALLABLE bool isValid() const
	{
		return p.isFinite() && q.isFinite() && q.isUnit();
	}

	/**
	\brief returns true if finite and quat magnitude is reasonably close to unit to allow for some accumulation of error vs isValid
	*/

	NV_CUDA_CALLABLE bool isSane() const
	{
	      return isFinite() && q.isSane();
	}


	/**
	\brief returns true if all elems are finite (not NAN or INF, etc.)
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isFinite() const { return p.isFinite() && q.isFinite(); }

	//! Transform transform from parent (returns compound transform: first src, then this->inverse)
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform transformInv(const NvTransform& src) const
	{
		NV_ASSERT(src.isSane());
		NV_ASSERT(isFinite());
		// src = [srct, srcr] -> [r^-1*(srct-t), r^-1*srcr]
		NvQuat qinv = q.getConjugate();
		return NvTransform(qinv.rotate(src.p - p), qinv*src.q);
	}


	
	/**
	\deprecated
	\brief deprecated - use NvTransform(NvIdentity)
	*/

	NV_DEPRECATED static NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform createIdentity() 
	{ 
		return NvTransform(NvIdentity); 
	}

	/**
	\brief transform plane
	*/

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane transform(const NvPlane& plane) const
	{
		NvVec3 transformedNormal = rotate(plane.n);
		return NvPlane(transformedNormal, plane.d - p.dot(transformedNormal));
	}

	/**
	\brief inverse-transform plane
	*/

	NV_CUDA_CALLABLE NV_FORCE_INLINE NvPlane inverseTransform(const NvPlane& plane) const
	{
		NvVec3 transformedNormal = rotateInv(plane.n);
		return NvPlane(transformedNormal, plane.d + p.dot(plane.n));
	}


	/**
	\brief return a normalized transform (i.e. one in which the quaternion has unit magnitude)
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform getNormalized() const
	{
		return NvTransform(p, q.getNormalized());
	}

};

// **************************************
// FILE: NvMat33.h
// **************************************

/*!
\brief 3x3 matrix class

Some clarifications, as there have been much confusion about matrix formats etc in the past.

Short:
- Matrix have base vectors in columns (vectors are column matrices, 3x1 matrices).
- Matrix is physically stored in column major format
- Matrices are concaternated from left

Long:
Given three base vectors a, b and c the matrix is stored as
         
|a.x b.x c.x|
|a.y b.y c.y|
|a.z b.z c.z|

Vectors are treated as columns, so the vector v is 

|x|
|y|
|z|

And matrices are applied _before_ the vector (pre-multiplication)
v' = M*v

|x'|   |a.x b.x c.x|   |x|   |a.x*x + b.x*y + c.x*z|
|y'| = |a.y b.y c.y| * |y| = |a.y*x + b.y*y + c.y*z|
|z'|   |a.z b.z c.z|   |z|   |a.z*x + b.z*y + c.z*z|


Physical storage and indexing:
To be compatible with popular 3d rendering APIs (read D3d and OpenGL)
the physical indexing is

|0 3 6|
|1 4 7|
|2 5 8|

index = column*3 + row

which in C++ translates to M[column][row]

The mathematical indexing is M_row,column and this is what is used for _-notation 
so _12 is 1st row, second column and operator(row, column)!

*/
class NvMat33
{
public:
	//! Default constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat33()
	{}

	//! identity constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat33(NvIDENTITY r)
		: column0(1.0f,0.0f,0.0f), column1(0.0f,1.0f,0.0f), column2(0.0f,0.0f,1.0f)
	{
		NV_UNUSED(r);
	}

	//! zero constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat33(NvZERO r)
		: column0(0.0f), column1(0.0f), column2(0.0f)
	{
		NV_UNUSED(r);
	}


	//! Construct from three base vectors
	NV_CUDA_CALLABLE NvMat33(const NvVec3& col0, const NvVec3& col1, const NvVec3& col2)
		: column0(col0), column1(col1), column2(col2)
	{}


	//! constructor from a scalar, which generates a multiple of the identity matrix
	explicit NV_CUDA_CALLABLE NV_INLINE NvMat33(float r)
		: column0(r,0.0f,0.0f), column1(0.0f,r,0.0f), column2(0.0f,0.0f,r)
	{}


	//! Construct from float[9]
	explicit NV_CUDA_CALLABLE NV_INLINE NvMat33(float values[]):
		column0(values[0],values[1],values[2]),
		column1(values[3],values[4],values[5]),
		column2(values[6],values[7],values[8])
	{
	}

	//! Construct from a quaternion
	explicit NV_CUDA_CALLABLE NV_FORCE_INLINE NvMat33(const NvQuat& q)
	{
		const float x = q.x;
		const float y = q.y;
		const float z = q.z;
		const float w = q.w;

		const float x2 = x + x;
		const float y2 = y + y;
		const float z2 = z + z;

		const float xx = x2*x;
		const float yy = y2*y;
		const float zz = z2*z;

		const float xy = x2*y;
		const float xz = x2*z;
		const float xw = x2*w;

		const float yz = y2*z;
		const float yw = y2*w;
		const float zw = z2*w;

		column0 = NvVec3(1.0f - yy - zz, xy + zw, xz - yw);
		column1 = NvVec3(xy - zw, 1.0f - xx - zz, yz + xw);
		column2 = NvVec3(xz + yw, yz - xw, 1.0f - xx - yy);
	}

	//! Copy constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat33(const NvMat33& other)
		: column0(other.column0), column1(other.column1), column2(other.column2)
	{}

	//! Assignment operator
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvMat33& operator=(const NvMat33& other)
	{
		column0 = other.column0;
		column1 = other.column1;
		column2 = other.column2;
		return *this;
	}

	//! \deprecated Set to identity matrix. Deprecated. use NvMat33(NvIdentity)
	NV_DEPRECATED NV_CUDA_CALLABLE NV_INLINE static NvMat33 createIdentity()
	{
		return NvMat33(NvIdentity);
	}

	//! \deprecated Set to zero matrix. Deprecated. use NvMat33(NvZero).
	NV_DEPRECATED NV_CUDA_CALLABLE NV_INLINE static NvMat33 createZero()
	{
		return NvMat33(NvZero);	// NvMat33(0) is ambiguous, it can either be the array constructor or the scalar constructor
	}

	//! Construct from diagonal, off-diagonals are zero.
	NV_CUDA_CALLABLE NV_INLINE static NvMat33 createDiagonal(const NvVec3& d)
	{
		return NvMat33(NvVec3(d.x,0.0f,0.0f), NvVec3(0.0f,d.y,0.0f), NvVec3(0.0f,0.0f,d.z));
	}


	//! Get transposed matrix
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvMat33 getTranspose() const
	{
		const NvVec3 v0(column0.x, column1.x, column2.x);
		const NvVec3 v1(column0.y, column1.y, column2.y);
		const NvVec3 v2(column0.z, column1.z, column2.z);

		return NvMat33(v0,v1,v2);   
	}

	//! Get the real inverse
	NV_CUDA_CALLABLE NV_INLINE NvMat33 getInverse() const
	{
		const float det = getDeterminant();
		NvMat33 inverse;

		if(det != 0)
		{
			const float invDet = 1.0f/det;

			inverse.column0[0] = invDet * (column1[1]*column2[2] - column2[1]*column1[2]);							
			inverse.column0[1] = invDet *-(column0[1]*column2[2] - column2[1]*column0[2]);
			inverse.column0[2] = invDet * (column0[1]*column1[2] - column0[2]*column1[1]);

			inverse.column1[0] = invDet *-(column1[0]*column2[2] - column1[2]*column2[0]);
			inverse.column1[1] = invDet * (column0[0]*column2[2] - column0[2]*column2[0]);
			inverse.column1[2] = invDet *-(column0[0]*column1[2] - column0[2]*column1[0]);

			inverse.column2[0] = invDet * (column1[0]*column2[1] - column1[1]*column2[0]);
			inverse.column2[1] = invDet *-(column0[0]*column2[1] - column0[1]*column2[0]);
			inverse.column2[2] = invDet * (column0[0]*column1[1] - column1[0]*column0[1]);

			return inverse;
		}
		else
		{
			return createIdentity();
		}
	}

	//! Get determinant
	NV_CUDA_CALLABLE NV_INLINE float getDeterminant() const
	{
		return column0.dot(column1.cross(column2));
	}

	//! Unary minus
	NV_CUDA_CALLABLE NV_INLINE NvMat33 operator-() const
	{
		return NvMat33(-column0, -column1, -column2);
	}

	//! Add
	NV_CUDA_CALLABLE NV_INLINE NvMat33 operator+(const NvMat33& other) const
	{
		return NvMat33( column0+other.column0,
					  column1+other.column1,
					  column2+other.column2);
	}

	//! Subtract
	NV_CUDA_CALLABLE NV_INLINE NvMat33 operator-(const NvMat33& other) const
	{
		return NvMat33( column0-other.column0,
					  column1-other.column1,
					  column2-other.column2);
	}

	//! Scalar multiplication
	NV_CUDA_CALLABLE NV_INLINE NvMat33 operator*(float scalar) const
	{
		return NvMat33(column0*scalar, column1*scalar, column2*scalar);
	}

	friend NvMat33 operator*(float, const NvMat33&);

	//! Matrix vector multiplication (returns 'this->transform(vec)')
	NV_CUDA_CALLABLE NV_INLINE NvVec3 operator*(const NvVec3& vec) const
	{
		return transform(vec);
	}

	//! Matrix multiplication
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvMat33 operator*(const NvMat33& other) const
	{
		//Rows from this <dot> columns from other
		//column0 = transform(other.column0) etc
		return NvMat33(transform(other.column0), transform(other.column1), transform(other.column2));
	}

	// a <op>= b operators

	//! Equals-add
	NV_CUDA_CALLABLE NV_INLINE NvMat33& operator+=(const NvMat33& other)
	{
		column0 += other.column0;
		column1 += other.column1;
		column2 += other.column2;
		return *this;
	}

	//! Equals-sub
	NV_CUDA_CALLABLE NV_INLINE NvMat33& operator-=(const NvMat33& other)
	{
		column0 -= other.column0;
		column1 -= other.column1;
		column2 -= other.column2;
		return *this;
	}

	//! Equals scalar multiplication
	NV_CUDA_CALLABLE NV_INLINE NvMat33& operator*=(float scalar)
	{
		column0 *= scalar;
		column1 *= scalar;
		column2 *= scalar;
		return *this;
	}

	//! Element access, mathematical way!
	NV_CUDA_CALLABLE NV_FORCE_INLINE float operator()(unsigned int row, unsigned int col) const
	{
		return (*this)[col][row];
	}

	//! Element access, mathematical way!
	NV_CUDA_CALLABLE NV_FORCE_INLINE float& operator()(unsigned int row, unsigned int col)
	{
		return (*this)[col][row];
	}

	// Transform etc

	//! Transform vector by matrix, equal to v' = M*v
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 transform(const NvVec3& other) const
	{
		return column0*other.x + column1*other.y + column2*other.z;
	}

	//! Transform vector by matrix transpose, v' = M^t*v
	NV_CUDA_CALLABLE NV_INLINE NvVec3 transformTranspose(const NvVec3& other) const
	{
		return NvVec3(	column0.dot(other),
						column1.dot(other),
						column2.dot(other));
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE const float* front() const
	{
		return &column0.x;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE			NvVec3& operator[](int num)			{return (&column0)[num];}
	NV_CUDA_CALLABLE NV_FORCE_INLINE			NvVec3& operator[](unsigned int num)			{return (&column0)[num];}
	NV_CUDA_CALLABLE NV_FORCE_INLINE const	NvVec3& operator[](int num) const	{return (&column0)[num];}
	NV_CUDA_CALLABLE NV_FORCE_INLINE const	NvVec3& operator[](unsigned int num) const	{return (&column0)[num];}

	//Data, see above for format!

	NvVec3 column0, column1, column2; //the three base vectors
};

// implementation from NvQuat.h
NV_CUDA_CALLABLE NV_INLINE NvQuat::NvQuat(const NvMat33& m)
{
	float tr = m(0,0) + m(1,1) + m(2,2), h;
	if(tr >= 0)
	{
		h = NvSqrt(tr +1);
		w = 0.5f * h;
		h = 0.5f / h;

		x = (m(2,1) - m(1,2)) * h;
		y = (m(0,2) - m(2,0)) * h;
		z = (m(1,0) - m(0,1)) * h;
	}
	else
	{
		unsigned int i = 0; 
		if (m(1,1) > m(0,0))
			i = 1; 
		if (m(2,2) > m(i,i))
			i = 2; 
		switch (i)
		{
		case 0:
			h = NvSqrt((m(0,0) - (m(1,1) + m(2,2))) + 1);
			x = 0.5f * h;
			h = 0.5f / h;

			y = (m(0,1) + m(1,0)) * h; 
			z = (m(2,0) + m(0,2)) * h;
			w = (m(2,1) - m(1,2)) * h;
			break;
		case 1:
			h = NvSqrt((m(1,1) - (m(2,2) + m(0,0))) + 1);
			y = 0.5f * h;
			h = 0.5f / h;

			z = (m(1,2) + m(2,1)) * h;
			x = (m(0,1) + m(1,0)) * h;
			w = (m(0,2) - m(2,0)) * h;
			break;
		case 2:
			h = NvSqrt((m(2,2) - (m(0,0) + m(1,1))) + 1);
			z = 0.5f * h;
			h = 0.5f / h;

			x = (m(2,0) + m(0,2)) * h;
			y = (m(1,2) + m(2,1)) * h;
			w = (m(1,0) - m(0,1)) * h;
			break;
		default: // Make compiler happy
			x = y = z = w = 0;
			break;
		}
	}
}

// *******************************
// FILE: NvBounds3.h
// *******************************

// maximum extents defined such that floating point exceptions are avoided for standard use cases
#define NV_MAX_BOUNDS_EXTENTS (NV_MAX_REAL * 0.25f)

/**
\brief Class representing 3D range or axis aligned bounding box.

Stored as minimum and maximum extent corners. Alternate representation
would be center and dimensions.
May be empty or nonempty. For nonempty bounds, minimum <= maximum has to hold for all axes.
Empty bounds have to be represented as minimum = NV_MAX_BOUNDS_EXTENTS and maximum = -NV_MAX_BOUNDS_EXTENTS for all axes.
All other representations are invalid and the behavior is undefined.
*/
class NvBounds3
{
public:

	/**
	\brief Default constructor, not performing any initialization for performance reason.
	\remark Use empty() function below to construct empty bounds.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3()	{}

	/**
	\brief Construct from two bounding points
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3(const NvVec3& minimum, const NvVec3& maximum);

	/**
	\brief Return empty bounds. 
	*/
	static NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3 empty();

	/**
	\brief returns the AABB containing v0 and v1.
	\param v0 first point included in the AABB.
	\param v1 second point included in the AABB.
	*/
	static NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3 boundsOfPoints(const NvVec3& v0, const NvVec3& v1);

	/**
	\brief returns the AABB from center and extents vectors.
	\param center Center vector
	\param extent Extents vector
	*/
	static NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3 centerExtents(const NvVec3& center, const NvVec3& extent);

	/**
	\brief Construct from center, extent, and (not necessarily orthogonal) basis
	*/
	static NV_CUDA_CALLABLE NV_INLINE NvBounds3 basisExtent(const NvVec3& center, const NvMat33& basis, const NvVec3& extent);

	/**
	\brief Construct from pose and extent
	*/
	static NV_CUDA_CALLABLE NV_INLINE NvBounds3 poseExtent(const NvTransform& pose, const NvVec3& extent);

	/**
	\brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).

	This version is safe to call for empty bounds.

	\param[in] matrix Transform to apply, can contain scaling as well
	\param[in] bounds The bounds to transform.
	*/
	static NV_CUDA_CALLABLE NV_INLINE NvBounds3 transformSafe(const NvMat33& matrix, const NvBounds3& bounds);

	/**
	\brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).

	Calling this method for empty bounds leads to undefined behavior. Use #transformSafe() instead.

	\param[in] matrix Transform to apply, can contain scaling as well
	\param[in] bounds The bounds to transform.
	*/
	static NV_CUDA_CALLABLE NV_INLINE NvBounds3 transformFast(const NvMat33& matrix, const NvBounds3& bounds);

	/**
	\brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).

	This version is safe to call for empty bounds.

	\param[in] transform Transform to apply, can contain scaling as well
	\param[in] bounds The bounds to transform.
	*/
	static NV_CUDA_CALLABLE NV_INLINE NvBounds3 transformSafe(const NvTransform& transform, const NvBounds3& bounds);

	/**
	\brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).

	Calling this method for empty bounds leads to undefined behavior. Use #transformSafe() instead.

	\param[in] transform Transform to apply, can contain scaling as well
	\param[in] bounds The bounds to transform.
	*/
	static NV_CUDA_CALLABLE NV_INLINE NvBounds3 transformFast(const NvTransform& transform, const NvBounds3& bounds);

	/**
	\brief Sets empty to true
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void setEmpty();

	/**
	\brief Sets the bounds to maximum size [-NV_MAX_BOUNDS_EXTENTS, NV_MAX_BOUNDS_EXTENTS].
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void setMaximal();

	/**
	\brief expands the volume to include v
	\param v Point to expand to.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void include(const NvVec3& v);

	/**
	\brief expands the volume to include b.
	\param b Bounds to perform union with.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void include(const NvBounds3& b);

	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isEmpty() const;

	/**
	\brief indicates whether the intersection of this and b is empty or not.
	\param b Bounds to test for intersection.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool intersects(const NvBounds3& b) const;

	/**
	 \brief computes the 1D-intersection between two AABBs, on a given axis.
	 \param	a		the other AABB
	 \param	axis	the axis (0, 1, 2)
	 */
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool intersects1D(const NvBounds3& a, uint32_t axis)	const;

	/**
	\brief indicates if these bounds contain v.
	\param v Point to test against bounds.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool contains(const NvVec3& v) const;

	/**
	 \brief	checks a box is inside another box.
	 \param	box		the other AABB
	 */
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isInside(const NvBounds3& box) const;

	/**
	\brief returns the center of this axis aligned box.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getCenter() const;

	/**
	\brief get component of the box's center along a given axis
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float getCenter(uint32_t axis)	const;

	/**
	\brief get component of the box's extents along a given axis
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE float getExtents(uint32_t axis)	const;

	/**
	\brief returns the dimensions (width/height/depth) of this axis aligned box.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getDimensions() const;

	/**
	\brief returns the extents, which are half of the width/height/depth.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 getExtents() const;

	/**
	\brief scales the AABB.

	This version is safe to call for empty bounds.

	\param scale Factor to scale AABB by.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void scaleSafe(float scale);

	/**
	\brief scales the AABB.

	Calling this method for empty bounds leads to undefined behavior. Use #scaleSafe() instead.

	\param scale Factor to scale AABB by.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void scaleFast(float scale);

	/** 
	fattens the AABB in all 3 dimensions by the given distance.

	This version is safe to call for empty bounds.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void fattenSafe(float distance);

	/**
	fattens the AABB in all 3 dimensions by the given distance. 

	Calling this method for empty bounds leads to undefined behavior. Use #fattenSafe() instead.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE void fattenFast(float distance);

	/** 
	checks that the AABB values are not NaN
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isFinite() const;

	/** 
	checks that the AABB values describe a valid configuration.
	*/
	NV_CUDA_CALLABLE NV_FORCE_INLINE bool isValid() const;

	NvVec3 minimum, maximum;
};


NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3::NvBounds3(const NvVec3& minimum, const NvVec3& maximum)
: minimum(minimum), maximum(maximum)
{
}

NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3 NvBounds3::empty()
{
	return NvBounds3(NvVec3(NV_MAX_BOUNDS_EXTENTS), NvVec3(-NV_MAX_BOUNDS_EXTENTS));
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::isFinite() const
{
	return minimum.isFinite() && maximum.isFinite();
}

NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3 NvBounds3::boundsOfPoints(const NvVec3& v0, const NvVec3& v1)
{
	return NvBounds3(v0.minimum(v1), v0.maximum(v1));
}

NV_CUDA_CALLABLE NV_FORCE_INLINE NvBounds3 NvBounds3::centerExtents(const NvVec3& center, const NvVec3& extent)
{
	return NvBounds3(center - extent, center + extent);
}

NV_CUDA_CALLABLE NV_INLINE NvBounds3 NvBounds3::basisExtent(const NvVec3& center, const NvMat33& basis, const NvVec3& extent)
{
	// extended basis vectors
	NvVec3 c0 = basis.column0 * extent.x;
	NvVec3 c1 = basis.column1 * extent.y;
	NvVec3 c2 = basis.column2 * extent.z;

	NvVec3 w;
	// find combination of base vectors that produces max. distance for each component = sum of abs()
	w.x = NvAbs(c0.x) + NvAbs(c1.x) + NvAbs(c2.x);
	w.y = NvAbs(c0.y) + NvAbs(c1.y) + NvAbs(c2.y);
	w.z = NvAbs(c0.z) + NvAbs(c1.z) + NvAbs(c2.z);

	return NvBounds3(center - w, center + w);
}

NV_CUDA_CALLABLE NV_INLINE NvBounds3 NvBounds3::poseExtent(const NvTransform& pose, const NvVec3& extent)
{
	return basisExtent(pose.p, NvMat33(pose.q), extent);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::setEmpty()
{
	minimum = NvVec3(NV_MAX_BOUNDS_EXTENTS);
	maximum = NvVec3(-NV_MAX_BOUNDS_EXTENTS);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::setMaximal()
{
	minimum = NvVec3(-NV_MAX_BOUNDS_EXTENTS);
	maximum = NvVec3(NV_MAX_BOUNDS_EXTENTS);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::include(const NvVec3& v)
{
	NV_ASSERT(isValid());
	minimum = minimum.minimum(v);
	maximum = maximum.maximum(v);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::include(const NvBounds3& b)
{
	NV_ASSERT(isValid());
	minimum = minimum.minimum(b.minimum);
	maximum = maximum.maximum(b.maximum);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::isEmpty() const
{
	NV_ASSERT(isValid());
	return minimum.x > maximum.x;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::intersects(const NvBounds3& b) const
{
	NV_ASSERT(isValid() && b.isValid());
	return !(b.minimum.x > maximum.x || minimum.x > b.maximum.x ||
			 b.minimum.y > maximum.y || minimum.y > b.maximum.y ||
			 b.minimum.z > maximum.z || minimum.z > b.maximum.z);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::intersects1D(const NvBounds3& a, uint32_t axis)	const
{
	NV_ASSERT(isValid() && a.isValid());
	return maximum[axis] >= a.minimum[axis] && a.maximum[axis] >= minimum[axis];
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::contains(const NvVec3& v) const
{
	NV_ASSERT(isValid());

	return !(v.x < minimum.x || v.x > maximum.x ||
		v.y < minimum.y || v.y > maximum.y ||
		v.z < minimum.z || v.z > maximum.z);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::isInside(const NvBounds3& box) const
{
	NV_ASSERT(isValid() && box.isValid());
	if(box.minimum.x>minimum.x)	return false;
	if(box.minimum.y>minimum.y)	return false;
	if(box.minimum.z>minimum.z)	return false;
	if(box.maximum.x<maximum.x)	return false;
	if(box.maximum.y<maximum.y)	return false;
	if(box.maximum.z<maximum.z)	return false;
	return true;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 NvBounds3::getCenter() const
{
	NV_ASSERT(isValid());
	return (minimum+maximum) * 0.5f;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE float NvBounds3::getCenter(uint32_t axis)	const
{
	NV_ASSERT(isValid());
	return (minimum[axis] + maximum[axis]) * 0.5f;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE float NvBounds3::getExtents(uint32_t axis)	const
{
	NV_ASSERT(isValid());
	return (maximum[axis] - minimum[axis]) * 0.5f;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 NvBounds3::getDimensions() const
{
	NV_ASSERT(isValid());
	return maximum - minimum;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE NvVec3 NvBounds3::getExtents() const
{
	NV_ASSERT(isValid());
	return getDimensions() * 0.5f;
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::scaleSafe(float scale)
{
	NV_ASSERT(isValid());
	if (!isEmpty())
		scaleFast(scale);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::scaleFast(float scale)
{
	NV_ASSERT(isValid());
	*this = centerExtents(getCenter(), getExtents() * scale);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::fattenSafe(float distance)
{
	NV_ASSERT(isValid());
	if (!isEmpty())
		fattenFast(distance);
}

NV_CUDA_CALLABLE NV_FORCE_INLINE void NvBounds3::fattenFast(float distance)
{
	NV_ASSERT(isValid());
	minimum.x -= distance;
	minimum.y -= distance;
	minimum.z -= distance;

	maximum.x += distance;
	maximum.y += distance;
	maximum.z += distance;
}

NV_CUDA_CALLABLE NV_INLINE NvBounds3 NvBounds3::transformSafe(const NvMat33& matrix, const NvBounds3& bounds)
{
	NV_ASSERT(bounds.isValid());
	return !bounds.isEmpty() ? transformFast(matrix, bounds) : bounds;
}

NV_CUDA_CALLABLE NV_INLINE NvBounds3 NvBounds3::transformFast(const NvMat33& matrix, const NvBounds3& bounds)
{
	NV_ASSERT(bounds.isValid());
	return NvBounds3::basisExtent(matrix * bounds.getCenter(), matrix, bounds.getExtents());
}

NV_CUDA_CALLABLE NV_INLINE NvBounds3 NvBounds3::transformSafe(const NvTransform& transform, const NvBounds3& bounds)
{
	NV_ASSERT(bounds.isValid());
	return !bounds.isEmpty() ? transformFast(transform, bounds) : bounds;
}

NV_CUDA_CALLABLE NV_INLINE NvBounds3 NvBounds3::transformFast(const NvTransform& transform, const NvBounds3& bounds)
{
	NV_ASSERT(bounds.isValid());
	return NvBounds3::basisExtent(transform.transform(bounds.getCenter()), NvMat33(transform.q), bounds.getExtents());
}

NV_CUDA_CALLABLE NV_FORCE_INLINE bool NvBounds3::isValid() const
{
	return (isFinite() && 
			(((minimum.x <= maximum.x) && (minimum.y <= maximum.y) && (minimum.z <= maximum.z)) ||
			 ((minimum.x == NV_MAX_BOUNDS_EXTENTS) && (minimum.y == NV_MAX_BOUNDS_EXTENTS) && (minimum.z == NV_MAX_BOUNDS_EXTENTS) && 
			  (maximum.x == -NV_MAX_BOUNDS_EXTENTS) && (maximum.y == -NV_MAX_BOUNDS_EXTENTS) && (maximum.z == -NV_MAX_BOUNDS_EXTENTS)))
		);
}

// ***********************************
// FILE: NvMat44.h
// ***********************************

/*!
\brief 4x4 matrix class

This class is layout-compatible with D3D and OpenGL matrices. More notes on layout are given in the NvMat33

@see NvMat33 NvTransform
*/

class NvMat44
{
public:
	//! Default constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat44()
	{}

	//! identity constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat44(NvIDENTITY r)
		: column0(1.0f,0.0f,0.0f,0.0f), column1(0.0f,1.0f,0.0f,0.0f), column2(0.0f,0.0f,1.0f,0.0f), column3(0.0f,0.0f,0.0f,1.0f)
	{
		NV_UNUSED(r);
	}

	//! zero constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat44(NvZERO r)
		: column0(NvZero), column1(NvZero), column2(NvZero), column3(NvZero)
	{
		NV_UNUSED(r);
	}

	//! Construct from four 4-vectors
	NV_CUDA_CALLABLE NvMat44(const NvVec4& col0, const NvVec4& col1, const NvVec4& col2, const NvVec4 &col3)
		: column0(col0), column1(col1), column2(col2), column3(col3)
	{}

	//! constructor that generates a multiple of the identity matrix
	explicit NV_CUDA_CALLABLE NV_INLINE NvMat44(float r)
		: column0(r,0.0f,0.0f,0.0f), column1(0.0f,r,0.0f,0.0f), column2(0.0f,0.0f,r,0.0f), column3(0.0f,0.0f,0.0f,r)
	{}


	//! Construct from three base vectors and a translation
	NV_CUDA_CALLABLE NvMat44(const NvVec3& column0, const NvVec3& column1, const NvVec3& column2, const NvVec3& column3)
		: column0(column0,0), column1(column1,0), column2(column2,0), column3(column3,1)
	{}

	//! Construct from float[16]
	explicit NV_CUDA_CALLABLE NV_INLINE NvMat44(float values[]):
		column0(values[0],values[1],values[2], values[3]),
		column1(values[4],values[5],values[6], values[7]),
		column2(values[8],values[9],values[10], values[11]),
		column3(values[12], values[13], values[14], values[15])
	{
	}

	//! Construct from a quaternion
	explicit NV_CUDA_CALLABLE NV_INLINE NvMat44(const NvQuat& q)
	{
		const float x = q.x;
		const float y = q.y;
		const float z = q.z;
		const float w = q.w;

		const float x2 = x + x;
		const float y2 = y + y;
		const float z2 = z + z;

		const float xx = x2*x;
		const float yy = y2*y;
		const float zz = z2*z;

		const float xy = x2*y;
		const float xz = x2*z;
		const float xw = x2*w;

		const float yz = y2*z;
		const float yw = y2*w;
		const float zw = z2*w;

		column0 = NvVec4(1.0f - yy - zz, xy + zw, xz - yw, 0.0f);
		column1 = NvVec4(xy - zw, 1.0f - xx - zz, yz + xw, 0.0f);
		column2 = NvVec4(xz + yw, yz - xw, 1.0f - xx - yy, 0.0f);
		column3 = NvVec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	//! Construct from a diagonal vector
	explicit NV_CUDA_CALLABLE NV_INLINE NvMat44(const NvVec4& diagonal):
		column0(diagonal.x,0.0f,0.0f,0.0f),
		column1(0.0f,diagonal.y,0.0f,0.0f),
		column2(0.0f,0.0f,diagonal.z,0.0f),
		column3(0.0f,0.0f,0.0f,diagonal.w)
	{
	}

	NV_CUDA_CALLABLE NvMat44(const NvMat33& orientation, const NvVec3& position):
		column0(orientation.column0,0.0f),
		column1(orientation.column1,0.0f),
		column2(orientation.column2,0.0f),
		column3(position,1)
	{
	}
		
	NV_CUDA_CALLABLE NvMat44(const NvTransform& t)
	{
		*this = NvMat44(NvMat33(t.q), t.p);
	}

	//! Copy constructor
	NV_CUDA_CALLABLE NV_INLINE NvMat44(const NvMat44& other)
		: column0(other.column0), column1(other.column1), column2(other.column2), column3(other.column3)
	{}

	//! Assignment operator
	NV_CUDA_CALLABLE NV_INLINE const NvMat44& operator=(const NvMat44& other)
	{
		column0 = other.column0;
		column1 = other.column1;
		column2 = other.column2;
		column3 = other.column3;
		return *this;
	}

	//! \deprecated Set to identity matrix. Deprecated. use NvMat44(NvIdentity).
	NV_DEPRECATED NV_CUDA_CALLABLE NV_INLINE static NvMat44 createIdentity()
	{
		return NvMat44(NvIdentity);
	}


	//! \deprecated Set to zero matrix. Deprecated. use NvMat44(NvZero).
	NV_DEPRECATED NV_CUDA_CALLABLE NV_INLINE static NvMat44 createZero()
	{
		return NvMat44(NvZero); 
	}

	//! Get transposed matrix
	NV_CUDA_CALLABLE NV_INLINE NvMat44 getTranspose() const
	{
		return NvMat44(NvVec4(column0.x, column1.x, column2.x, column3.x),
			         NvVec4(column0.y, column1.y, column2.y, column3.y),
					 NvVec4(column0.z, column1.z, column2.z, column3.z),
				     NvVec4(column0.w, column1.w, column2.w, column3.w));
	}

	//! Unary minus
	NV_CUDA_CALLABLE NV_INLINE NvMat44 operator-() const
	{
		return NvMat44(-column0, -column1, -column2, -column3);
	}

	//! Add
	NV_CUDA_CALLABLE NV_INLINE NvMat44 operator+(const NvMat44& other) const
	{
		return NvMat44( column0+other.column0,
					  column1+other.column1,
					  column2+other.column2,
					  column3+other.column3);
	}

	//! Subtract
	NV_CUDA_CALLABLE NV_INLINE NvMat44 operator-(const NvMat44& other) const
	{
		return NvMat44( column0-other.column0,
					  column1-other.column1,
					  column2-other.column2,
					  column3-other.column3);
	}

	//! Scalar multiplication
	NV_CUDA_CALLABLE NV_INLINE NvMat44 operator*(float scalar) const
	{
		return NvMat44(column0*scalar, column1*scalar, column2*scalar, column3*scalar);
	}

	friend NvMat44 operator*(float, const NvMat44&);

	//! Matrix multiplication
	NV_CUDA_CALLABLE NV_INLINE NvMat44 operator*(const NvMat44& other) const
	{
		//Rows from this <dot> columns from other
		//column0 = transform(other.column0) etc
		return NvMat44(transform(other.column0), transform(other.column1), transform(other.column2), transform(other.column3));
	}

	// a <op>= b operators

	//! Equals-add
	NV_CUDA_CALLABLE NV_INLINE NvMat44& operator+=(const NvMat44& other)
	{
		column0 += other.column0;
		column1 += other.column1;
		column2 += other.column2;
		column3 += other.column3;
		return *this;
	}

	//! Equals-sub
	NV_CUDA_CALLABLE NV_INLINE NvMat44& operator-=(const NvMat44& other)
	{
		column0 -= other.column0;
		column1 -= other.column1;
		column2 -= other.column2;
		column3 -= other.column3;
		return *this;
	}

	//! Equals scalar multiplication
	NV_CUDA_CALLABLE NV_INLINE NvMat44& operator*=(float scalar)
	{
		column0 *= scalar;
		column1 *= scalar;
		column2 *= scalar;
		column3 *= scalar;
		return *this;
	}

	//! Element access, mathematical way!
	NV_CUDA_CALLABLE NV_FORCE_INLINE float operator()(unsigned int row, unsigned int col) const
	{
		return (*this)[(int)col][(int)row];
	}

	//! Element access, mathematical way!
	NV_CUDA_CALLABLE NV_FORCE_INLINE float& operator()(unsigned int row, unsigned int col)
	{
		return (*this)[(int)col][(int)row];
	}

	//! Transform vector by matrix, equal to v' = M*v
	NV_CUDA_CALLABLE NV_INLINE NvVec4 transform(const NvVec4& other) const
	{
		return column0*other.x + column1*other.y + column2*other.z + column3*other.w;
	}

	//! Transform vector by matrix, equal to v' = M*v
	NV_CUDA_CALLABLE NV_INLINE NvVec3 transform(const NvVec3& other) const
	{
		return transform(NvVec4(other,1)).getXYZ();
	}

	//! Rotate vector by matrix, equal to v' = M*v
	NV_CUDA_CALLABLE NV_INLINE const NvVec4 rotate(const NvVec4& other) const
	{
		return column0*other.x + column1*other.y + column2*other.z;// + column3*0;
	}

	//! Rotate vector by matrix, equal to v' = M*v
	NV_CUDA_CALLABLE NV_INLINE const NvVec3 rotate(const NvVec3& other) const
	{
		return rotate(NvVec4(other,1)).getXYZ();
	}


	NV_CUDA_CALLABLE NV_INLINE NvVec3 getBasis(int num) const
	{
		NV_ASSERT(num>=0 && num<3);
		return (&column0)[num].getXYZ();
	}

	NV_CUDA_CALLABLE NV_INLINE NvVec3 getPosition() const
	{
		return column3.getXYZ();
	}

	NV_CUDA_CALLABLE NV_INLINE void setPosition(const NvVec3& position)
	{
		column3.x = position.x;
		column3.y = position.y;
		column3.z = position.z;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE const float* front() const
	{
		return &column0.x;
	}

	NV_CUDA_CALLABLE NV_FORCE_INLINE			NvVec4& operator[](int num)			{return (&column0)[num];}
	NV_CUDA_CALLABLE NV_FORCE_INLINE const	NvVec4& operator[](int num)	const	{return (&column0)[num];}

	NV_CUDA_CALLABLE NV_INLINE void	scale(const NvVec4& p)
	{
		column0 *= p.x;
		column1 *= p.y;
		column2 *= p.z;
		column3 *= p.w;
	}

	NV_CUDA_CALLABLE NV_INLINE NvMat44 inverseRT(void) const
	{
		NvVec3 r0(column0.x, column1.x, column2.x),
			r1(column0.y, column1.y, column2.y),
			r2(column0.z, column1.z, column2.z);

		return NvMat44(r0, r1, r2, -(r0 * column3.x + r1 * column3.y + r2 * column3.z));
	}

	NV_CUDA_CALLABLE NV_INLINE bool isFinite() const
	{
		return column0.isFinite() && column1.isFinite() && column2.isFinite() && column3.isFinite();
	}


	//Data, see above for format!

	NvVec4 column0, column1, column2, column3; //the four base vectors
};

// implementation from NvTransform.h
NV_CUDA_CALLABLE NV_FORCE_INLINE NvTransform::NvTransform(const NvMat44& m)
{
	NvVec3 column0  = NvVec3(m.column0.x, m.column0.y, m.column0.z);
	NvVec3 column1  = NvVec3(m.column1.x, m.column1.y, m.column1.z);
	NvVec3 column2  = NvVec3(m.column2.x, m.column2.y, m.column2.z);

	q = NvQuat(NvMat33(column0, column1, column2));
	p = NvVec3(m.column3.x, m.column3.y, m.column3.z);
}

// *********************************
// FILE: NvFlags.h
// *********************************

	/**
	\brief Container for bitfield flag variables associated with a specific enum type.

	This allows for type safe manipulation for bitfields.

	<h3>Example</h3>
		// enum that defines each bit...
		struct MyEnum
		{
			enum Enum
			{
				eMAN  = 1,
				eBEAR = 2,
				ePIG  = 4,
			};
		};
		
		// implements some convenient global operators.
		NV_FLAGS_OPERATORS(MyEnum::Enum, uint8_t);
		
		NvFlags<MyEnum::Enum, uint8_t> myFlags;
		myFlags |= MyEnum::eMAN;
		myFlags |= MyEnum::eBEAR | MyEnum::ePIG;
		if(myFlags & MyEnum::eBEAR)
		{
			doSomething();
		}
	*/

	template<typename enumtype, typename storagetype=uint32_t>
	class NvFlags
	{
	public:
		typedef storagetype InternalType;

		NV_INLINE	explicit	NvFlags(const NvEMPTY&)	{}
		NV_INLINE				NvFlags(void);
		NV_INLINE				NvFlags(enumtype e);
		NV_INLINE				NvFlags(const NvFlags<enumtype,storagetype> &f);
		NV_INLINE	explicit	NvFlags(storagetype b);		

		NV_INLINE bool							 isSet     (enumtype e) const;
		NV_INLINE NvFlags<enumtype,storagetype> &set       (enumtype e);
		NV_INLINE bool                           operator==(enumtype e) const;
		NV_INLINE bool                           operator==(const NvFlags<enumtype,storagetype> &f) const;
		NV_INLINE bool                           operator==(bool b) const;
		NV_INLINE bool                           operator!=(enumtype e) const;
		NV_INLINE bool                           operator!=(const NvFlags<enumtype,storagetype> &f) const;

		NV_INLINE NvFlags<enumtype,storagetype> &operator =(enumtype e);
		
		NV_INLINE NvFlags<enumtype,storagetype> &operator|=(enumtype e);
		NV_INLINE NvFlags<enumtype,storagetype> &operator|=(const NvFlags<enumtype,storagetype> &f);
		NV_INLINE NvFlags<enumtype,storagetype>  operator| (enumtype e) const;
		NV_INLINE NvFlags<enumtype,storagetype>  operator| (const NvFlags<enumtype,storagetype> &f) const;
		
		NV_INLINE NvFlags<enumtype,storagetype> &operator&=(enumtype e);
		NV_INLINE NvFlags<enumtype,storagetype> &operator&=(const NvFlags<enumtype,storagetype> &f);
		NV_INLINE NvFlags<enumtype,storagetype>  operator& (enumtype e) const;
		NV_INLINE NvFlags<enumtype,storagetype>  operator& (const NvFlags<enumtype,storagetype> &f) const;
		
		NV_INLINE NvFlags<enumtype,storagetype> &operator^=(enumtype e);
		NV_INLINE NvFlags<enumtype,storagetype> &operator^=(const NvFlags<enumtype,storagetype> &f);
		NV_INLINE NvFlags<enumtype,storagetype>  operator^ (enumtype e) const;
		NV_INLINE NvFlags<enumtype,storagetype>  operator^ (const NvFlags<enumtype,storagetype> &f) const;
		
		NV_INLINE NvFlags<enumtype,storagetype>  operator~ (void) const;
		
		NV_INLINE                                operator bool(void) const;
		NV_INLINE								 operator uint8_t(void) const;
		NV_INLINE                                operator uint16_t(void) const;
		NV_INLINE                                operator uint32_t(void) const;

		NV_INLINE void                           clear(enumtype e);
	
	public:
		friend NV_INLINE NvFlags<enumtype,storagetype> operator&(enumtype a, NvFlags<enumtype,storagetype> &b)
		{
			NvFlags<enumtype,storagetype> out;
			out.mBits = a & b.mBits;
			return out;
		}

	private:
		storagetype  mBits;
	};

	#define NV_FLAGS_OPERATORS(enumtype, storagetype)                                                                                         \
		NV_INLINE NvFlags<enumtype, storagetype> operator|(enumtype a, enumtype b) { NvFlags<enumtype, storagetype> r(a); r |= b; return r; } \
		NV_INLINE NvFlags<enumtype, storagetype> operator&(enumtype a, enumtype b) { NvFlags<enumtype, storagetype> r(a); r &= b; return r; } \
		NV_INLINE NvFlags<enumtype, storagetype> operator~(enumtype a)             { return ~NvFlags<enumtype, storagetype>(a);             }

	#define NV_FLAGS_TYPEDEF(x, y)	typedef NvFlags<x::Enum, y> x##s;	\
	NV_FLAGS_OPERATORS(x::Enum, y);

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::NvFlags(void)
	{
		mBits = 0;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::NvFlags(enumtype e)
	{
		mBits = static_cast<storagetype>(e);
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::NvFlags(const NvFlags<enumtype,storagetype> &f)
	{
		mBits = f.mBits;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::NvFlags(storagetype b)
	{
		mBits = b;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE bool NvFlags<enumtype,storagetype>::isSet(enumtype e) const
	{
		return (mBits & static_cast<storagetype>(e)) == static_cast<storagetype>(e);
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::set(enumtype e)
	{
		mBits = static_cast<storagetype>(e);
		return *this;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE bool NvFlags<enumtype,storagetype>::operator==(enumtype e) const
	{
		return mBits == static_cast<storagetype>(e);
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE bool NvFlags<enumtype,storagetype>::operator==(const NvFlags<enumtype,storagetype>& f) const
	{
		return mBits == f.mBits;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE bool NvFlags<enumtype,storagetype>::operator==(bool b) const
	{
		return ((bool)*this) == b;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE bool NvFlags<enumtype,storagetype>::operator!=(enumtype e) const
	{
		return mBits != static_cast<storagetype>(e);
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE bool NvFlags<enumtype,storagetype>::operator!=(const NvFlags<enumtype,storagetype> &f) const
	{
		return mBits != f.mBits;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator =(enumtype e)
	{
		mBits = static_cast<storagetype>(e);
		return *this;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator|=(enumtype e)
	{
		mBits |= static_cast<storagetype>(e);
		return *this;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator|=(const NvFlags<enumtype,storagetype> &f)
	{
		mBits |= f.mBits;
		return *this;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> NvFlags<enumtype,storagetype>::operator| (enumtype e) const
	{
		NvFlags<enumtype,storagetype> out(*this);
		out |= e;
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> NvFlags<enumtype,storagetype>::operator| (const NvFlags<enumtype,storagetype> &f) const
	{
		NvFlags<enumtype,storagetype> out(*this);
		out |= f;
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator&=(enumtype e)
	{
		mBits &= static_cast<storagetype>(e);
		return *this;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator&=(const NvFlags<enumtype,storagetype> &f)
	{
		mBits &= f.mBits;
		return *this;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> NvFlags<enumtype,storagetype>::operator&(enumtype e) const
	{
		NvFlags<enumtype,storagetype> out = *this;
		out.mBits &= static_cast<storagetype>(e);
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>  NvFlags<enumtype,storagetype>::operator& (const NvFlags<enumtype,storagetype> &f) const
	{
		NvFlags<enumtype,storagetype> out = *this;
		out.mBits &= f.mBits;
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator^=(enumtype e)
	{
		mBits ^= static_cast<storagetype>(e);
		return *this;
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> &NvFlags<enumtype,storagetype>::operator^=(const NvFlags<enumtype,storagetype> &f)
	{
		mBits ^= f.mBits;
		return *this;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> NvFlags<enumtype,storagetype>::operator^ (enumtype e) const
	{
		NvFlags<enumtype,storagetype> out = *this;
		out.mBits ^= static_cast<storagetype>(e);
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> NvFlags<enumtype,storagetype>::operator^ (const NvFlags<enumtype,storagetype> &f) const
	{
		NvFlags<enumtype,storagetype> out = *this;
		out.mBits ^= f.mBits;
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype> NvFlags<enumtype,storagetype>::operator~ (void) const
	{
		NvFlags<enumtype,storagetype> out;
		out.mBits = (storagetype)~mBits;
		return out;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::operator bool(void) const
	{
		return mBits ? true : false;
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::operator uint8_t(void) const
	{
		return static_cast<uint8_t>(mBits);
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::operator uint16_t(void) const
	{
		return static_cast<uint16_t>(mBits);
	}
	
	template<typename enumtype, typename storagetype>
	NV_INLINE NvFlags<enumtype,storagetype>::operator uint32_t(void) const
	{
		return static_cast<uint32_t>(mBits);
	}

	template<typename enumtype, typename storagetype>
	NV_INLINE void NvFlags<enumtype,storagetype>::clear(enumtype e)
	{
		mBits &= ~static_cast<storagetype>(e);
	}

#ifndef NV_DOXYGEN
} // namespace nvidia
#endif

/** @} */


#endif // NV_FOUNDATION_H
