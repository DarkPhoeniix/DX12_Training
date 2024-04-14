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
	inline int operator&(const type x, const type y)\
	{\
		return ((int)x & (int)y);\
	}\
	inline type& operator^=(type &x, const type y)\
	{\
		x = (type)((int)x ^ (int)y); return x;\
	}\
	inline type operator^(const type x, const type y)\
	{\
		return (type)((int)x ^ (int)y);\
	}

namespace Helper
{
    std::string HrToString(HRESULT hr);

    class HrException : public std::runtime_error
    {
    public:
        HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
        HRESULT Error() const { return m_hr; }

    private:
        const HRESULT m_hr;
    };

    void throwIfFailed(HRESULT hr);

    Json::Value ParseJson(const std::string& filepath);
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
}
