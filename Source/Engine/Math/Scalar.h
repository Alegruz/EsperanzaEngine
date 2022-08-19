#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace esperanza
{
	class Scalar final
	{
	public:
		explicit constexpr Scalar() noexcept = default;
		explicit Scalar(float f) noexcept;
		explicit constexpr Scalar(FXMVECTOR vec) noexcept;
		explicit constexpr Scalar(const Scalar& other) noexcept = default;
		explicit constexpr Scalar(Scalar&& other) noexcept = default;
		constexpr Scalar& operator=(const Scalar& other) noexcept = default;
		constexpr Scalar& operator=(Scalar&& other) noexcept = default;
		~Scalar() noexcept = default;

		constexpr operator XMVECTOR() const noexcept;
		operator float() const noexcept;

	private:
		XMVECTOR m_Vec;
	};

	inline esperanza::Scalar::Scalar(float f) noexcept
		: m_Vec(XMVectorReplicate(f))
	{
	}

	inline constexpr esperanza::Scalar::Scalar(FXMVECTOR vec) noexcept
		: m_Vec(vec)
	{
	}

	inline constexpr Scalar::operator XMVECTOR() const noexcept
	{
		return m_Vec;
	}
	
	inline Scalar::operator float() const noexcept
	{
		return XMVectorGetX(m_Vec);
	}

	inline constexpr Scalar operator-(const Scalar& s) noexcept
	{
		return Scalar(XMVectorNegate(s));
	}

	inline constexpr Scalar operator+(const Scalar& s1, const Scalar& s2) noexcept
	{
		return Scalar(XMVectorAdd(s1, s2));
	}

	inline constexpr Scalar operator-(const Scalar& s1, const Scalar& s2) noexcept
	{
		return Scalar(XMVectorSubtract(s1, s2));
	}

	inline constexpr Scalar operator*(const Scalar& s1, const Scalar& s2) noexcept
	{
		return Scalar(XMVectorMultiply(s1, s2));
	}

	inline constexpr Scalar operator/(const Scalar& s1, const Scalar& s2) noexcept
	{
		return Scalar(XMVectorDivide(s1, s2));
	}

	inline constexpr Scalar operator+(const Scalar& s1, float s2) noexcept
	{
		return s1 + Scalar(s2);
	}

	inline constexpr Scalar operator-(const Scalar& s1, float s2) noexcept
	{
		return s1 - Scalar(s2);
	}

	inline constexpr Scalar operator*(const Scalar& s1, float s2) noexcept
	{
		return s1 * Scalar(s2);
	}

	inline constexpr Scalar operator/(const Scalar& s1, float s2) noexcept
	{
		return s1 / Scalar(s2);
	}

	inline constexpr Scalar operator+(float s1, const Scalar& s2) noexcept
	{
		return Scalar(s1) + s2;
	}

	inline constexpr Scalar operator-(float s1, const Scalar& s2) noexcept
	{
		return Scalar(s1) - s2;
	}

	inline constexpr Scalar operator*(float s1, const Scalar& s2) noexcept
	{
		return Scalar(s1) * s2;
	}

	inline constexpr Scalar operator/(float s1, const Scalar& s2) noexcept
	{
		return Scalar(s1) / s2;
	}
}