#include "Pch.h"
#include "Renderer/Color.h"

namespace esperanza
{
	uint32_t Color::PackToR11G11B10F() const noexcept
	{
		return PackToR11G11B10F(false);
	}

	uint32_t Color::PackToR11G11B10F(bool bRoundToEven) const noexcept
	{
		static constexpr const float MAX_VALUE = static_cast<float>(1 << 16);
		static constexpr const float F32_TO_F16 = (1.0 / (1ull << 56)) * (1.0 / (1ull << 56));

		union
		{
			float F;
			uint32_t U;
		} R, G, B;

		R.F = math::Clamp(m_Value.f[0], 0.0f, MAX_VALUE) * F32_TO_F16;
		G.F = math::Clamp(m_Value.f[1], 0.0f, MAX_VALUE) * F32_TO_F16;
		B.F = math::Clamp(m_Value.f[2], 0.0f, MAX_VALUE) * F32_TO_F16;

		if (bRoundToEven)
		{
			// Banker's roudning: 2.5 -> 2.0 ; 3.5 -> 4.0
			R.U += 0x0FFFF + ((R.U >> 16) & 1);
			G.U += 0x0FFFF + ((G.U >> 16) & 1);
			B.U += 0x1FFFF + ((B.U >> 17) & 1);
		}
		else
		{
			// Default rounding: 2.5 -> 3.0 ; 3.5 -> 4.0
			R.U += 0x00010000;
			G.U += 0x00010000;
			B.U += 0x00020000;
		}

		R.U &= 0x0FFE0000;
		G.U &= 0x0FFE0000;
		B.U &= 0x0FFC0000;

		return R.U >> 17 | G.U >> 6 | B.U << 4;
	}

	uint32_t Color::PackToR9G9B9E5() const noexcept
	{
		static constexpr const float MAX_VALUE = static_cast<float>(0x1FF << 7);
		static constexpr const float MIN_VALUE = 1.0f / static_cast<float>(1 << 16);

		// Clamp RGB to [0. 1/FF*2^16]
		float r = math::Clamp(m_Value.f[0], 0.0f, MAX_VALUE);
		float g = math::Clamp(m_Value.f[1], 0.0f, MAX_VALUE);
		float b = math::Clamp(m_Value.f[2], 0.0f, MAX_VALUE);

		// Compute the maximum channel, no less than 1.0 & 2^-15
		float maxChannel = math::Max(math::Max(r, g), math::Max(b, MIN_VALUE));

		// Take the exponent of the maximum channel (rounding up the 9th bit) and
		// add 15 to it. When added to the channels, it causes the implicit '1.0'
		// bit and the first 8 mantissa bits to be shifted down to the low 9 bits
		// of the mantissa, rounding the truncated bits.

		union
		{
			float F;
			int32_t I;
		} R, G, B, E;

		E.F = maxChannel;
		E.I += 0x07804000;	// Add 15 to the exponent and 0x4000 to the mantissa
		E.I &= 0x7F800000;	// Zero the mantissa

		// This shifts the 9-bit values we need into the lowest bits, rounding as
		// needed.  Note that if the channel has a smaller exponent than the max
		// channel, it will shift even more.  This is intentional.
		R.F = r + E.F;
		G.F = g + E.F;
		B.F = b + E.F;

		// Convert the Bias to the correct exponent in the upper 5 bits.
		E.I <<= 4;
		E.I += 0x10000000;

		// Combine the fields.  RGB floats have unwanted data in the upper 9
		// bits.  Only red needs to mask them off because green and blue shift
		// it out to the left.
		return E.I | B.I << 18 | G.I << 9 | R.I & 511;
	}
}