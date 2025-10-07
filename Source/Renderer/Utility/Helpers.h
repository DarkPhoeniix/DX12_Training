#pragma once

#define BINARY_OPERATION_TO_ENUM(type) \
	inline type& operator|=(type &x, const type y)\
	{\
		x = (type)((int)x | (int)y); return x;\
	}\
	inline type operator|(const type x, const type y)\
	{\
		return (type)((int)x | (int)y);\
	}\
	inline type& operator&=(type &x, const type y)\
	{\
		x = (type)((int)x & (int)y); return x;\
	}\
	inline type operator&(const type x, const type y)\
	{\
		return (type)((int)x & (int)y);\
	}\
	inline type& operator^=(type &x, const type y)\
	{\
		x = (type)((int)x ^ (int)y); return x;\
	}\
	inline type operator^(const type x, const type y)\
	{\
		return (type)((int)x ^ (int)y);\
	}

template<typename Enum> requires std::is_enum_v<Enum>
inline constexpr bool HasFlag(Enum value, Enum flag)
{
	using Underlying = std::underlying_type_t<Enum>;
    return (static_cast<Underlying>(value) & static_cast<Underlying>(flag)) == static_cast<Underlying>(flag);
};

template<typename Enum> requires std::is_enum_v<Enum>
inline constexpr bool HasAnyFlag(Enum value, Enum flag)
{
	using Underlying = std::underlying_type_t<Enum>;
	return (static_cast<Underlying>(value) & static_cast<Underlying>(flag)) != 0;
}

namespace Math
{
    template <typename T>
    inline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template<typename T>
    inline T Clamp(const T lhs, const T min, const T max)
    {
        if (lhs < min)
            return min;
        else if (lhs > max)
            return max;
        else
            return lhs;
    }
}
