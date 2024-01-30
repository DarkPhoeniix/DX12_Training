#pragma once

// TODO: binary op macros
// why const for the enum?
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

enum class EResourceType : int
{
    None = 1 << 0,

    // access type 
    Dynamic = 1 << 1,
    ReadBack = 1 << 2,
    Unordered = 1 << 3,

    // kind of resource
    Buffer = 1 << 4,
    Texture = 1 << 5,
    RenderTarget = 1 << 6,
    DepthTarget = 1 << 7,

    // addition flags
    StrideAlignment = 1 << 8,

    // acceleration flags
    Deny_shader_resource = 1 << 9,

    Last = 1 << 10
};
BINARY_OPERATION_TO_ENUM(EResourceType);

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

    ID3D12Resource* CreateBuffers(ComPtr<ID3D12Device2> device, EResourceType type, unsigned int size, unsigned int stride);
    ID3D12Resource* CreateBuffers(ComPtr<ID3D12Device2> device, EResourceType type, unsigned int sizeW, unsigned int sizeH, unsigned int stride, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST);

    ID3D12Resource* CreateBuffers(ID3D12Heap* pHeap, ComPtr<ID3D12Device2> device, EResourceType type, unsigned int size, unsigned int stride, unsigned int offset);
}

namespace Math
{
    constexpr float PI = 3.1415926535897932384626433832795f;
    constexpr float _2PI = 2.0f * PI;
    // Convert radians to degrees.
    constexpr float Degrees(const float radians)
    {
        return radians * (180.0f / PI);
    }

    // Convert degrees to radians.
    constexpr float Radians(const float degrees)
    {
        return degrees * (PI / 180.0f);
    }

    template<typename T>
    inline T Deadzone(T val, T deadzone)
    {
        if (std::abs(val) < deadzone)
        {
            return T(0);
        }

        return val;
    }

    // Normalize a value in the range [min - max]
    template<typename T, typename U>
    inline T NormalizeRange(U x, U min, U max)
    {
        return T(x - min) / T(max - min);
    }

    // Shift and bias a value into another range.
    template<typename T, typename U>
    inline T ShiftBias(U x, U shift, U bias)
    {
        return T(x * bias) + T(shift);
    }

    /***************************************************************************
    * These functions were taken from the MiniEngine.
    * Source code available here:
    * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
    * Retrieved: January 13, 2016
    **************************************************************************/
    template <typename T>
    inline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    template <typename T>
    inline bool IsAligned(T value, size_t alignment)
    {
        return 0 == ((size_t)value & (alignment - 1));
    }

    template <typename T>
    inline T DivideByMultiple(T value, size_t alignment)
    {
        return (T)((value + alignment - 1) / alignment);
    }
    /***************************************************************************/

    /**
    * Round up to the next highest power of 2.
    * @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    * @retrieved: January 16, 2016
    */
    inline uint32_t NextHighestPow2(uint32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;

        return v;
    }

    /**
    * Round up to the next highest power of 2.
    * @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    * @retrieved: January 16, 2016
    */
    inline uint64_t NextHighestPow2(uint64_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;

        return v;
    }
}
