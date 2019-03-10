/******************************************************************************/
// Shader Bump Mapping Modes
#define SBUMP_ZERO      0 // no vertex normal
#define SBUMP_FLAT      1
#define SBUMP_NORMAL    2
#define SBUMP_PARALLAX0 2 // !! this is on purpose the same as SBUMP_NORMAL because it's never used anyway, it's used just for calculations
#define SBUMP_PARALLAX1 3
#define SBUMP_PARALLAX2 4
#define SBUMP_PARALLAX3 5
#define SBUMP_PARALLAX4 6
#define SBUMP_RELIEF    7

#define SBUMP_PARALLAX_MIN       SBUMP_PARALLAX2 // ignore SBUMP_PARALLAX1 because just 1 sample is not worth to keep as separate shader
#define SBUMP_PARALLAX_MAX       SBUMP_PARALLAX4
#define SBUMP_PARALLAX_MAX_MULTI SBUMP_PARALLAX2 // for multi-material shaders, limit to only SBUMP_PARALLAX2

// Effects
#define FX_NONE  0
#define FX_GRASS 1
#define FX_LEAF  2
#define FX_LEAFS 3
#define FX_BONE  4

#define MAX_MOTION_BLUR_PIXEL_RANGE 24

#define MAX_MTRLS 4 // 3 or 4 (3 to make shaders smaller, 4 to support more materials per tri)

#define LCScale 0.2f // must be in sync with GLSL

// Buffer Indexes
#define SBI_GLOBAL     0
#define SBI_OBJ_MATRIX 1
#define SBI_OBJ_VEL    2
#define SBI_MESH       3
#define SBI_MATERIAL   4
#define SBI_VIEWPORT   5
#define SBI_COLOR      6
#define SBI_NUM        7
// !! if any change is made here then all shaders need to be recompiled !!

// Sampler Indexes
#define SSI_DEFAULT      0
#define SSI_POINT        1
#define SSI_LINEAR_CLAMP 2
#define SSI_LINEAR_WRAP  3
#define SSI_SHADOW       4
#define SSI_FONT         5
#define SSI_LINEAR_CWW   6
/******************************************************************************/
