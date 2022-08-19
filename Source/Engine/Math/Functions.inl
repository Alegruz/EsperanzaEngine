#pragma once

namespace esperanza
{
	namespace math
	{
#define CREATE_SIMD_FUNCTIONS(type) \
	inline type Max(type a, type b) { return type(XMVectorMax(a, b)); } \
	inline type Min(type a, type b) { return type(XMVectorMin(a, b)); } \
	inline type Clamp(type v, type a, type b) { return Min(Max(v, a), b); }

		//CREATE_SIMD_FUNCTIONS(Scalar);
#undef CREATE_SIMD_FUNCTIONS

		inline constexpr float Max(float a, float b) 
		{ 
			return a > b ? a : b; 
		}

		inline constexpr float Min(float a, float b) 
		{ 
			return a < b ? a : b; 
		}

		inline constexpr float Clamp(float v, float a, float b) 
		{ 
			return Min(Max(v, a), b); 
		}
	}
}