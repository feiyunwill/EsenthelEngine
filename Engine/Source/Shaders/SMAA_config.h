#define SMAA_AREATEX_SELECT(sample) sample.rg
#define SMAA_RT_METRICS             ColSize
#define SMAA_THRESHOLD              SMAAThreshold // best noticable on "iloyjp6kr6q56_jzjamo0z6#" /* Vehicles\Cartoon\Tank */
#define SMAA_MAX_SEARCH_STEPS       6
#define SMAA_MAX_SEARCH_STEPS_DIAG  0
#define SMAA_CORNER_ROUNDING        100
#if SMAA_MAX_SEARCH_STEPS_DIAG==0
   #define SMAA_DISABLE_DIAG_DETECTION
#endif
#define SMAA_COLOR_WEIGHT_USE 1 // enabling slightly increases performance
#define SMAA_COLOR_WEIGHT     float3(0.509, 1.000, 0.194) // ClipSet(ColorLumWeight2/ColorLumWeight2.max());
