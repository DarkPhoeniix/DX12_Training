
#define AABB_RootSig \
	"RootFlags " \
	"( " \
		"DENY_VERTEX_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS " \
	"), " \
	"RootConstants(num32BitConstants=8, b0, visibility=SHADER_VISIBILITY_ALL), " \
	"RootConstants(num32BitConstants=16, b1, visibility=SHADER_VISIBILITY_ALL)"
