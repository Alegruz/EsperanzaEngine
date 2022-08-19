#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace esperanza
{
	class Color final
	{
	public:
		explicit constexpr Color() noexcept;
		explicit constexpr Color(FXMVECTOR vec) noexcept;
		explicit Color(float r, float g, float b) noexcept;
		explicit Color(float r, float g, float b, float a) noexcept;
		explicit Color(uint16_t r, uint16_t g, uint16_t b) noexcept;
		explicit Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a) noexcept;
		explicit Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth) noexcept;
		explicit Color(uint32_t rgbaLittleEndian) noexcept;
		explicit constexpr Color(const Color& other) noexcept = default;
		explicit constexpr Color(Color&& other) noexcept = default;
		constexpr Color& operator=(const Color& other) noexcept = default;
		constexpr Color& operator=(Color&& other) noexcept = default;
		constexpr ~Color() noexcept = default;

		float GetR() const noexcept;
		float GetG() const noexcept;
		float GetB() const noexcept;
		float GetA() const noexcept;

		bool operator==(const Color& rhs) const noexcept;
		bool operator!=(const Color& rhs) const noexcept;

		constexpr void SetR(float r) noexcept;
		constexpr void SetG(float g) noexcept;
		constexpr void SetB(float b) noexcept;
		constexpr void SetA(float a) noexcept;

		float* GetPtr() noexcept;
		const float* GetPtr() const noexcept;

		float& operator[](int idx) noexcept;
		const float& operator[](int idx) const noexcept;

		void SetRgb(float r, float g, float b) noexcept;

		Color ConvertToSRgb() const noexcept;
		Color ConvertFromSRgb() const noexcept;
		Color ConvertToRec709() const noexcept;
		Color ConvertFromRec709() const noexcept;

		uint32_t ConvertToR10G10B10A2() const noexcept;
		uint32_t ConvertToR8G8B8A8() const noexcept;

		uint32_t PackToR11G11B10F() const noexcept;
		uint32_t PackToR11G11B10F(bool bRoundToEven) const noexcept;
		uint32_t PackToR9G9B9E5() const noexcept;

		operator XMVECTOR() const noexcept;

	private:
		XMVECTORF32 m_Value;
	};

	constexpr Color GetMax(const Color& lhs, const Color& rhs)
	{
		return Color(XMVectorMax(lhs, rhs));
	}

	constexpr Color GetMin(const Color& lhs, const Color& rhs)
	{
		return Color(XMVectorMin(lhs, rhs));
	}

	constexpr Color Clamp(const Color& x, const Color& a, const Color& b)
	{
		return Color(XMVectorClamp(x, a, b));
	}

	inline constexpr Color::Color() noexcept
		: m_Value(g_XMOne)
	{
	}

	inline constexpr Color::Color(FXMVECTOR vec) noexcept
		: m_Value()
	{
		m_Value.v = vec;
	}

	inline Color::Color(float r, float g, float b) noexcept
		: Color(r, g, b, 1.0f)
	{
	}

	inline Color::Color(float r, float g, float b, float a) noexcept
		: m_Value()
	{
		m_Value.v = XMVectorSet(r, g, b, a);
	}

	inline Color::Color(uint16_t r, uint16_t g, uint16_t b) noexcept
		: Color(r, g, b, 255)
	{
	}

	inline Color::Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a) noexcept
		: Color(r, g, b, a, 8)
	{
	}

	inline Color::Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth) noexcept
		: m_Value()
	{
		m_Value.v = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / ((1 << bitDepth) - 1));
	}

	inline Color::Color(uint32_t rgbaLittleEndian) noexcept
		: m_Value()
	{
		float r = static_cast<float>((rgbaLittleEndian >> 0) & 0xFF);
		float g = static_cast<float>((rgbaLittleEndian >> 8) & 0xFF);
		float b = static_cast<float>((rgbaLittleEndian >> 16) & 0xFF);
		float a = static_cast<float>((rgbaLittleEndian >> 24) & 0xFF);

		m_Value.v = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / 255.0f);
	}

	inline float Color::GetR() const noexcept
	{
		return XMVectorGetX(m_Value);
	}

	inline float Color::GetG() const noexcept
	{
		return XMVectorGetY(m_Value);
	}

	inline float Color::GetB() const noexcept
	{
		return XMVectorGetZ(m_Value);
	}

	inline float Color::GetA() const noexcept
	{
		return XMVectorGetW(m_Value);
	}

	inline bool Color::operator==(const Color& rhs) const noexcept
	{
		return XMVector4Equal(m_Value, rhs.m_Value);
	}
	inline bool Color::operator!=(const Color& rhs) const noexcept
	{
		return !XMVector4Equal(m_Value, rhs.m_Value);
	}

	inline constexpr void Color::SetR(float r) noexcept
	{
		m_Value.f[0] = r;
	}

	inline constexpr void Color::SetG(float g) noexcept
	{
		m_Value.f[1] = g;
	}

	inline constexpr void Color::SetB(float b) noexcept
	{
		m_Value.f[2] = b;
	}

	inline constexpr void Color::SetA(float a) noexcept
	{
		m_Value.f[3] = a;
	}

	inline float* Color::GetPtr() noexcept
	{
		return reinterpret_cast<float*>(this);
	}

	inline const float* Color::GetPtr() const noexcept
	{
		return reinterpret_cast<const float*>(this);
	}

	inline float& Color::operator[](int idx) noexcept
	{
		return GetPtr()[idx];
	}
	inline const float& Color::operator[](int idx) const noexcept
	{
		return GetPtr()[idx];
	}

	inline void Color::SetRgb(float r, float g, float b) noexcept
	{
		m_Value.v = XMVectorSelect(m_Value, XMVectorSet(r, g, b, b), g_XMMask3);
	}

	inline Color Color::ConvertToSRgb() const noexcept
	{
		XMVECTOR t = XMVectorSaturate(m_Value);
		XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(t, XMVectorReplicate(1.0f / 2.4f)), 1.055f), XMVectorReplicate(0.055f));
		result = XMVectorSelect(result, XMVectorScale(t, 12.92f), XMVectorLess(t, XMVectorReplicate(0.0031307f)));
		return Color(XMVectorSelect(t, result, g_XMSelect1110));
	}

	inline Color Color::ConvertFromSRgb() const noexcept
	{
		XMVECTOR t = XMVectorSaturate(m_Value);
		XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(t, XMVectorReplicate(0.055f)), 1.0f / 1.055f), XMVectorReplicate(2.4f));
		result = XMVectorSelect(result, XMVectorScale(t, 1.0f / 12.92f), XMVectorLess(t, XMVectorReplicate(0.0031308f)));
		return Color(XMVectorSelect(t, result, g_XMSelect1110));
	}

	inline Color Color::ConvertToRec709() const noexcept
	{
		XMVECTOR t = XMVectorSaturate(m_Value);
		XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(t, XMVectorReplicate(0.45f)), 1.099f), XMVectorReplicate(0.099f));
		result = XMVectorSelect(result, XMVectorScale(t, 4.5f), XMVectorLess(t, XMVectorReplicate(0.0018f)));
		return Color(XMVectorSelect(t, result, g_XMSelect1110));
	}

	inline Color Color::ConvertFromRec709() const noexcept
	{
		XMVECTOR t = XMVectorSaturate(m_Value);
		XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(t, XMVectorReplicate(0.099f)), 1.0f / 1.099f), XMVectorReplicate(1.0f / 0.45f));
		result = XMVectorSelect(result, XMVectorScale(t, 1.0f / 4.5f), XMVectorLess(t, XMVectorReplicate(0.0081f)));
		return Color(XMVectorSelect(t, result, g_XMSelect1110));
	}

	inline uint32_t Color::ConvertToR10G10B10A2() const noexcept
	{
		XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_Value), XMVectorSet(1023.0f, 1023.0f, 1023.0f, 3.0f)));
		result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
		uint32_t r = XMVectorGetIntX(result);
		uint32_t g = XMVectorGetIntY(result);
		uint32_t b = XMVectorGetIntZ(result);
		uint32_t a = XMVectorGetIntW(result) >> 8;
		return a << 30 | b << 20 | g << 10 | r;
	}

	inline uint32_t Color::ConvertToR8G8B8A8() const noexcept
	{
		XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_Value), XMVectorReplicate(255.0f)));
		result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
		uint32_t r = XMVectorGetIntX(result);
		uint32_t g = XMVectorGetIntY(result);
		uint32_t b = XMVectorGetIntZ(result);
		uint32_t a = XMVectorGetIntW(result);
		return a << 24 | b << 16 | g << 8 | r;
	}

	inline Color::operator XMVECTOR() const noexcept
	{
		return m_Value;
	}
}