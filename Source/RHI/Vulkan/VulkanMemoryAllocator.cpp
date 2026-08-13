
#include "RHI_PCH.h"

// VMA ships header-only; this is its single implementation unit. Its configuration is set on the
// RHI target so every translation unit including the header agrees with this one
#pragma warning(push, 0)
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#pragma warning(pop)
