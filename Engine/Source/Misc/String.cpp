/******************************************************************************/
#include "stdafx.h"

#define USE_STD WINDOWS // 'iswalpha' on Apple/Linux supports only ASCII, on Android it works on more characters but not the same as on Windows, we need consistent results across all platforms, so use 'iswalpha' on Windows, and 'Alphas' on other platforms (which was pre-computed from 'iswalpha' on Windows)
#define EXTRA 32 // extra length allocated, used for appending
ASSERT(EXTRA>=2); // must be at least 2 because we use it for setting characters (char+nul)

namespace EE{
/******************************************************************************/
static const Char
            CharReal       =L'ℝ', // real number
            CharSmiley     =L'☺',
            CharMale       =L'♂',
            CharFemale     =L'♀',
            CharDeath      =L'☠',
            CharRadioactive=L'☢',
            CharBiohazard  =L'☣';
static const Char Combining[]=u"ัิีึืฺุู็่้๊๋์ํ๎",
                  Stack    []=u"ัิีึื็่้๊๋์ํ๎";

#if !USE_STD
static const U16 Alphas[][2]={65,90, 97,122, 170,170, 181,181, 186,186, 192,214, 216,246, 248,696, 699,705, 720,721, 736,740, 750,750, 880,883, 886,887, 890,893, 902,902, 904,906, 908,908, 910,929, 931,1013, 1015,1153, 1162,1315, 1329,1366, 1369,1369, 1377,1415, 1488,1514, 1520,1522, 1569,1610, 1646,1647, 1649,1747, 1749,1749, 1765,1766, 1774,1775, 1786,1788, 1791,1791, 1808,1808, 1810,1839, 1869,1957, 1969,1969, 1994,2026, 2036,2037, 2042,2042, 2308,2361, 2365,2365, 2384,2384, 2392,2401, 2417,2418, 2427,2431, 2437,2444, 2447,2448, 2451,2472, 2474,2480, 2482,2482, 2486,2489, 2493,2493, 2510,2510, 2524,2525, 2527,2529, 2544,2545, 2565,2570, 2575,2576, 2579,2600, 2602,2608, 2610,2611, 2613,2614, 2616,2617, 2649,2652, 2654,2654, 2674,2676, 2693,2701, 2703,2705, 2707,2728, 2730,2736, 2738,2739, 2741,2745, 2749,2749, 2768,2768, 2784,2785, 2821,2828, 2831,2832, 2835,2856, 2858,2864, 2866,2867, 2869,2873, 2877,2877, 2908,2909, 2911,2913, 2929,2929, 2947,2947, 2949,2954, 2958,2960, 2962,2965, 2969,2970, 2972,2972, 2974,2975, 2979,2980, 2984,2986, 2990,3001, 3024,3024, 3077,3084, 3086,3088, 3090,3112, 3114,3123, 3125,3129, 3133,3133, 3160,3161, 3168,3169, 3205,3212, 3214,3216, 3218,3240, 3242,3251, 3253,3257, 3261,3261, 3294,3294, 3296,3297, 3333,3340, 3342,3344, 3346,3368, 3370,3385, 3389,3389, 3424,3425, 3450,3455, 3461,3478, 3482,3505, 3507,3515, 3517,3517, 3520,3526, 3585,3642, 3648,3662, 3713,3714, 3716,3716, 3719,3720, 3722,3722, 3725,3725, 3732,3735, 3737,3743, 3745,3747, 3749,3749, 3751,3751, 3754,3755, 3757,3760, 3762,3763, 3773,3773, 3776,3780, 3782,3782, 3804,3805, 3840,3840, 3904,3911, 3913,3948, 3976,3979, 4096,4138, 4159,4159, 4176,4181, 4186,4189, 4193,4193, 4197,4198, 4206,4208, 4213,4225, 4238,4238, 4256,4293, 4304,4346, 4348,4348, 4352,4441, 4447,4514, 4520,4601, 4608,4680, 4682,4685, 4688,4694, 4696,4696, 4698,4701, 4704,4744, 4746,4749, 4752,4784, 4786,4789, 4792,4798, 4800,4800, 4802,4805, 4808,4822, 4824,4880, 4882,4885, 4888,4954, 4992,5007, 5024,5108, 5121,5740, 5743,5750, 5761,5786, 5792,5866, 5870,5872, 5888,5900, 5902,5905, 5920,5937, 5952,5969, 5984,5996, 5998,6000, 6016,6067, 6103,6103, 6108,6108, 6176,6263, 6272,6312, 6314,6314, 6400,6428, 6480,6509, 6512,6516, 6528,6569, 6593,6599, 6656,6678, 6917,6963, 6981,6987, 7043,7072, 7086,7087, 7168,7203, 7245,7247, 7258,7293, 7424,7615, 7680,7957, 7960,7965, 7968,8005, 8008,8013, 8016,8023, 8025,8025, 8027,8027, 8029,8029, 8031,8061, 8064,8116, 8118,8124, 8126,8126, 8130,8132, 8134,8140, 8144,8147, 8150,8155, 8160,8172, 8178,8180, 8182,8188, 8305,8305, 8319,8319, 8336,8340, 8450,8450, 8455,8455, 8458,8467, 8469,8469, 8473,8477, 8484,8484, 8486,8486, 8488,8488, 8490,8493, 8495,8505, 8508,8511, 8517,8521, 8526,8526, 8544,8584, 11264,11310, 11312,11358, 11360,11375, 11377,11389, 11392,11492, 11520,11557, 11568,11621, 11631,11631, 11648,11670, 11680,11686, 11688,11694, 11696,11702, 11704,11710, 11712,11718, 11720,11726, 11728,11734, 11736,11742, 12293,12295, 12321,12329, 12337,12341, 12344,12348, 12353,12438, 12445,12447, 12449,12538, 12540,12543, 12549,12589, 12593,12686, 12704,12727, 12784,12799, 13312,19893, 19968,40899, 40960,42124, 42240,42508, 42512,42527, 42538,42539, 42560,42591, 42594,42606, 42624,42647, 42786,42887, 42891,42892, 43003,43009, 43011,43013, 43015,43018, 43020,43042, 43072,43123, 43138,43187, 43274,43301, 43312,43334, 43520,43560, 43584,43586, 43588,43595, 44032,55203, 63744,64045, 64048,64106, 64112,64217, 64256,64262, 64275,64279, 64285,64285, 64287,64296, 64298,64310, 64312,64316, 64318,64318, 64320,64321, 64323,64324, 64326,64433, 64467,64829, 64848,64911, 64914,64967, 65008,65019, 65136,65140, 65142,65276, 65313,65338, 65345,65370, 65382,65470, 65474,65479, 65482,65487, 65490,65495, 65498,65500};
static const U16 DownUps[][2]={97,65, 98,66, 99,67, 100,68, 101,69, 102,70, 103,71, 104,72, 105,73, 106,74, 107,75, 108,76, 109,77, 110,78, 111,79, 112,80, 113,81, 114,82, 115,83, 116,84, 117,85, 118,86, 119,87, 120,88, 121,89, 122,90, 224,192, 225,193, 226,194, 227,195, 228,196, 229,197, 230,198, 231,199, 232,200, 233,201, 234,202, 235,203, 236,204, 237,205, 238,206, 239,207, 240,208, 241,209, 242,210, 243,211, 244,212, 245,213, 246,214, 248,216, 249,217, 250,218, 251,219, 252,220, 253,221, 254,222, 255,159, 255,376, 257,256, 259,258, 261,260, 263,262, 265,264, 267,266, 269,268, 271,270, 273,272, 275,274, 277,276, 279,278, 281,280, 283,282, 285,284, 287,286, 289,288, 291,290, 293,292, 295,294, 297,296, 299,298, 301,300, 303,302, 307,306, 309,308, 311,310, 314,313, 316,315, 318,317, 320,319, 322,321, 324,323, 326,325, 328,327, 331,330, 333,332, 335,334, 337,336, 339,338, 341,340, 343,342, 345,344, 347,346, 349,348, 351,350, 353,352, 355,354, 357,356, 359,358, 361,360, 363,362, 365,364, 367,366, 369,368, 371,370, 373,372, 375,374, 378,377, 380,379, 382,381, 384,579, 387,386, 389,388, 392,391, 396,395, 402,401, 405,502, 409,408, 410,573, 414,544, 417,416, 419,418, 421,420, 424,423, 429,428, 432,431, 436,435, 438,437, 441,440, 445,444, 447,503, 454,452, 457,455, 460,458, 462,461, 464,463, 466,465, 468,467, 470,469, 472,471, 474,473, 476,475, 477,398, 479,478, 481,480, 483,482, 485,484, 487,486, 489,488, 491,490, 493,492, 495,494, 499,497, 501,500, 505,504, 507,506, 509,508, 511,510, 513,512, 515,514, 517,516, 519,518, 521,520, 523,522, 525,524, 527,526, 529,528, 531,530, 533,532, 535,534, 537,536, 539,538, 541,540, 543,542, 547,546, 549,548, 551,550, 553,552, 555,554, 557,556, 559,558, 561,560, 563,562, 572,571, 578,577, 583,582, 585,584, 587,586, 589,588, 591,590, 592,11375, 593,11373, 595,385, 596,390, 598,393, 599,394, 601,399, 603,400, 608,403, 611,404, 616,407, 617,406, 619,11362, 623,412, 625,11374, 626,413, 629,415, 637,11364, 640,422, 643,425, 648,430, 649,580, 650,433, 651,434, 652,581, 658,439, 881,880, 883,882, 887,886, 891,1021, 892,1022, 893,1023, 940,902, 941,904, 942,905, 943,906, 945,913, 946,914, 947,915, 948,916, 949,917, 950,918, 951,919, 952,920, 953,921, 954,922, 955,923, 956,924, 957,925, 958,926, 959,927, 960,928, 961,929, 963,931, 964,932, 965,933, 966,934, 967,935, 968,936, 969,937, 970,938, 971,939, 972,908, 973,910, 974,911, 983,975, 985,984, 987,986, 989,988, 991,990, 993,992, 995,994, 997,996, 999,998, 1001,1000, 1003,1002, 1005,1004, 1007,1006, 1010,1017, 1016,1015, 1019,1018, 1072,1040, 1073,1041, 1074,1042, 1075,1043, 1076,1044, 1077,1045, 1078,1046, 1079,1047, 1080,1048, 1081,1049, 1082,1050, 1083,1051, 1084,1052, 1085,1053, 1086,1054, 1087,1055, 1088,1056, 1089,1057, 1090,1058, 1091,1059, 1092,1060, 1093,1061, 1094,1062, 1095,1063, 1096,1064, 1097,1065, 1098,1066, 1099,1067, 1100,1068, 1101,1069, 1102,1070, 1103,1071, 1104,1024, 1105,1025, 1106,1026, 1107,1027, 1108,1028, 1109,1029, 1110,1030, 1111,1031, 1112,1032, 1113,1033, 1114,1034, 1115,1035, 1116,1036, 1117,1037, 1118,1038, 1119,1039, 1121,1120, 1123,1122, 1125,1124, 1127,1126, 1129,1128, 1131,1130, 1133,1132, 1135,1134, 1137,1136, 1139,1138, 1141,1140, 1143,1142, 1145,1144, 1147,1146, 1149,1148, 1151,1150, 1153,1152, 1163,1162, 1165,1164, 1167,1166, 1169,1168, 1171,1170, 1173,1172, 1175,1174, 1177,1176, 1179,1178, 1181,1180, 1183,1182, 1185,1184, 1187,1186, 1189,1188, 1191,1190, 1193,1192, 1195,1194, 1197,1196, 1199,1198, 1201,1200, 1203,1202, 1205,1204, 1207,1206, 1209,1208, 1211,1210, 1213,1212, 1215,1214, 1218,1217, 1220,1219, 1222,1221, 1224,1223, 1226,1225, 1228,1227, 1230,1229, 1231,1216, 1233,1232, 1235,1234, 1237,1236, 1239,1238, 1241,1240, 1243,1242, 1245,1244, 1247,1246, 1249,1248, 1251,1250, 1253,1252, 1255,1254, 1257,1256, 1259,1258, 1261,1260, 1263,1262, 1265,1264, 1267,1266, 1269,1268, 1271,1270, 1273,1272, 1275,1274, 1277,1276, 1279,1278, 1281,1280, 1283,1282, 1285,1284, 1287,1286, 1289,1288, 1291,1290, 1293,1292, 1295,1294, 1297,1296, 1299,1298, 1301,1300, 1303,1302, 1305,1304, 1307,1306, 1309,1308, 1311,1310, 1313,1312, 1315,1314, 1377,1329, 1378,1330, 1379,1331, 1380,1332, 1381,1333, 1382,1334, 1383,1335, 1384,1336, 1385,1337, 1386,1338, 1387,1339, 1388,1340, 1389,1341, 1390,1342, 1391,1343, 1392,1344, 1393,1345, 1394,1346, 1395,1347, 1396,1348, 1397,1349, 1398,1350, 1399,1351, 1400,1352, 1401,1353, 1402,1354, 1403,1355, 1404,1356, 1405,1357, 1406,1358, 1407,1359, 1408,1360, 1409,1361, 1410,1362, 1411,1363, 1412,1364, 1413,1365, 1414,1366, 7545,42877, 7549,11363, 7681,7680, 7683,7682, 7685,7684, 7687,7686, 7689,7688, 7691,7690, 7693,7692, 7695,7694, 7697,7696, 7699,7698, 7701,7700, 7703,7702, 7705,7704, 7707,7706, 7709,7708, 7711,7710, 7713,7712, 7715,7714, 7717,7716, 7719,7718, 7721,7720, 7723,7722, 7725,7724, 7727,7726, 7729,7728, 7731,7730, 7733,7732, 7735,7734, 7737,7736, 7739,7738, 7741,7740, 7743,7742, 7745,7744, 7747,7746, 7749,7748, 7751,7750, 7753,7752, 7755,7754, 7757,7756, 7759,7758, 7761,7760, 7763,7762, 7765,7764, 7767,7766, 7769,7768, 7771,7770, 7773,7772, 7775,7774, 7777,7776, 7779,7778, 7781,7780, 7783,7782, 7785,7784, 7787,7786, 7789,7788, 7791,7790, 7793,7792, 7795,7794, 7797,7796, 7799,7798, 7801,7800, 7803,7802, 7805,7804, 7807,7806, 7809,7808, 7811,7810, 7813,7812, 7815,7814, 7817,7816, 7819,7818, 7821,7820, 7823,7822, 7825,7824, 7827,7826, 7829,7828, 7841,7840, 7843,7842, 7845,7844, 7847,7846, 7849,7848, 7851,7850, 7853,7852, 7855,7854, 7857,7856, 7859,7858, 7861,7860, 7863,7862, 7865,7864, 7867,7866, 7869,7868, 7871,7870, 7873,7872, 7875,7874, 7877,7876, 7879,7878, 7881,7880, 7883,7882, 7885,7884, 7887,7886, 7889,7888, 7891,7890, 7893,7892, 7895,7894, 7897,7896, 7899,7898, 7901,7900, 7903,7902, 7905,7904, 7907,7906, 7909,7908, 7911,7910, 7913,7912, 7915,7914, 7917,7916, 7919,7918, 7921,7920, 7923,7922, 7925,7924, 7927,7926, 7929,7928, 7931,7930, 7933,7932, 7935,7934, 7936,7944, 7937,7945, 7938,7946, 7939,7947, 7940,7948, 7941,7949, 7942,7950, 7943,7951, 7952,7960, 7953,7961, 7954,7962, 7955,7963, 7956,7964, 7957,7965, 7968,7976, 7969,7977, 7970,7978, 7971,7979, 7972,7980, 7973,7981, 7974,7982, 7975,7983, 7984,7992, 7985,7993, 7986,7994, 7987,7995, 7988,7996, 7989,7997, 7990,7998, 7991,7999, 8000,8008, 8001,8009, 8002,8010, 8003,8011, 8004,8012, 8005,8013, 8017,8025, 8019,8027, 8021,8029, 8023,8031, 8032,8040, 8033,8041, 8034,8042, 8035,8043, 8036,8044, 8037,8045, 8038,8046, 8039,8047, 8048,8122, 8049,8123, 8050,8136, 8051,8137, 8052,8138, 8053,8139, 8054,8154, 8055,8155, 8056,8184, 8057,8185, 8058,8170, 8059,8171, 8060,8186, 8061,8187, 8064,8072, 8065,8073, 8066,8074, 8067,8075, 8068,8076, 8069,8077, 8070,8078, 8071,8079, 8080,8088, 8081,8089, 8082,8090, 8083,8091, 8084,8092, 8085,8093, 8086,8094, 8087,8095, 8096,8104, 8097,8105, 8098,8106, 8099,8107, 8100,8108, 8101,8109, 8102,8110, 8103,8111, 8112,8120, 8113,8121, 8115,8124, 8131,8140, 8144,8152, 8145,8153, 8160,8168, 8161,8169, 8165,8172, 8179,8188, 8526,8498, 8560,8544, 8561,8545, 8562,8546, 8563,8547, 8564,8548, 8565,8549, 8566,8550, 8567,8551, 8568,8552, 8569,8553, 8570,8554, 8571,8555, 8572,8556, 8573,8557, 8574,8558, 8575,8559, 8580,8579, 9424,9398, 9425,9399, 9426,9400, 9427,9401, 9428,9402, 9429,9403, 9430,9404, 9431,9405, 9432,9406, 9433,9407, 9434,9408, 9435,9409, 9436,9410, 9437,9411, 9438,9412, 9439,9413, 9440,9414, 9441,9415, 9442,9416, 9443,9417, 9444,9418, 9445,9419, 9446,9420, 9447,9421, 9448,9422, 9449,9423, 11312,11264, 11313,11265, 11314,11266, 11315,11267, 11316,11268, 11317,11269, 11318,11270, 11319,11271, 11320,11272, 11321,11273, 11322,11274, 11323,11275, 11324,11276, 11325,11277, 11326,11278, 11327,11279, 11328,11280, 11329,11281, 11330,11282, 11331,11283, 11332,11284, 11333,11285, 11334,11286, 11335,11287, 11336,11288, 11337,11289, 11338,11290, 11339,11291, 11340,11292, 11341,11293, 11342,11294, 11343,11295, 11344,11296, 11345,11297, 11346,11298, 11347,11299, 11348,11300, 11349,11301, 11350,11302, 11351,11303, 11352,11304, 11353,11305, 11354,11306, 11355,11307, 11356,11308, 11357,11309, 11358,11310, 11361,11360, 11365,570, 11366,574, 11368,11367, 11370,11369, 11372,11371, 11379,11378, 11382,11381, 11393,11392, 11395,11394, 11397,11396, 11399,11398, 11401,11400, 11403,11402, 11405,11404, 11407,11406, 11409,11408, 11411,11410, 11413,11412, 11415,11414, 11417,11416, 11419,11418, 11421,11420, 11423,11422, 11425,11424, 11427,11426, 11429,11428, 11431,11430, 11433,11432, 11435,11434, 11437,11436, 11439,11438, 11441,11440, 11443,11442, 11445,11444, 11447,11446, 11449,11448, 11451,11450, 11453,11452, 11455,11454, 11457,11456, 11459,11458, 11461,11460, 11463,11462, 11465,11464, 11467,11466, 11469,11468, 11471,11470, 11473,11472, 11475,11474, 11477,11476, 11479,11478, 11481,11480, 11483,11482, 11485,11484, 11487,11486, 11489,11488, 11491,11490, 11520,4256, 11521,4257, 11522,4258, 11523,4259, 11524,4260, 11525,4261, 11526,4262, 11527,4263, 11528,4264, 11529,4265, 11530,4266, 11531,4267, 11532,4268, 11533,4269, 11534,4270, 11535,4271, 11536,4272, 11537,4273, 11538,4274, 11539,4275, 11540,4276, 11541,4277, 11542,4278, 11543,4279, 11544,4280, 11545,4281, 11546,4282, 11547,4283, 11548,4284, 11549,4285, 11550,4286, 11551,4287, 11552,4288, 11553,4289, 11554,4290, 11555,4291, 11556,4292, 11557,4293, 42561,42560, 42563,42562, 42565,42564, 42567,42566, 42569,42568, 42571,42570, 42573,42572, 42575,42574, 42577,42576, 42579,42578, 42581,42580, 42583,42582, 42585,42584, 42587,42586, 42589,42588, 42591,42590, 42595,42594, 42597,42596, 42599,42598, 42601,42600, 42603,42602, 42605,42604, 42625,42624, 42627,42626, 42629,42628, 42631,42630, 42633,42632, 42635,42634, 42637,42636, 42639,42638, 42641,42640, 42643,42642, 42645,42644, 42647,42646, 42787,42786, 42789,42788, 42791,42790, 42793,42792, 42795,42794, 42797,42796, 42799,42798, 42803,42802, 42805,42804, 42807,42806, 42809,42808, 42811,42810, 42813,42812, 42815,42814, 42817,42816, 42819,42818, 42821,42820, 42823,42822, 42825,42824, 42827,42826, 42829,42828, 42831,42830, 42833,42832, 42835,42834, 42837,42836, 42839,42838, 42841,42840, 42843,42842, 42845,42844, 42847,42846, 42849,42848, 42851,42850, 42853,42852, 42855,42854, 42857,42856, 42859,42858, 42861,42860, 42863,42862, 42874,42873, 42876,42875, 42879,42878, 42881,42880, 42883,42882, 42885,42884, 42887,42886, 42892,42891, 65345,65313, 65346,65314, 65347,65315, 65348,65316, 65349,65317, 65350,65318, 65351,65319, 65352,65320, 65353,65321, 65354,65322, 65355,65323, 65356,65324, 65357,65325, 65358,65326, 65359,65327, 65360,65328, 65361,65329, 65362,65330, 65363,65331, 65364,65332, 65365,65333, 65366,65334, 65367,65335, 65368,65336, 65369,65337, 65370,65338};
#endif

       static Bool Initialized;
       static void InitStr();
INLINE static void I      () {if(!Initialized){InitStr(); Initialized=true;}} // this is faster when inline

static struct InitStrClass
{
   InitStrClass() {I();}
}InitStrObj;
/******************************************************************************/
const Str  S;
const Str8 S8;

const Char8 Digits16[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

Char  _Char8To16[  256];
Char8 _Char16To8[65536];

static U16 CharOrder8 [  256],
           CharOrder16[65536];

static Char _CaseDown[65536],
            _CaseUp  [65536];

U16 _CharFlag[65536];

static struct CharReplace
{
   Char  src ;
   Char8 dest;

   void set(Char src, Char8 dest) {T.src=src; T.dest=dest;}

   static Int Compare(C CharReplace &a, C CharReplace &b) {return ::Compare(U16(a.src), U16(b.src));}
   static Int Compare(C CharReplace &a, C Char        &b) {return ::Compare(U16(a.src), U16(b    ));}
}CharReplaces[]=
{
   // !! This must be sorted !! codes for creation of this array are located in 'InitStr'
   {L'À', 'A'},
   {L'Á', 'A'},
   {L'Â', 'A'},
   {L'Ã', 'A'},
   {L'Ä', 'A'},
   {L'Å', 'A'},
   {L'Æ', 'A'},
   {L'Ç', 'C'},
   {L'È', 'E'},
   {L'É', 'E'},
   {L'Ê', 'E'},
   {L'Ë', 'E'},
   {L'Ì', 'I'},
   {L'Í', 'I'},
   {L'Î', 'I'},
   {L'Ï', 'I'},
   {L'Ñ', 'N'},
   {L'Ò', 'O'},
   {L'Ó', 'O'},
   {L'Ô', 'O'},
   {L'Õ', 'O'},
   {L'Ö', 'O'},
   {L'Ø', 'O'},
   {L'Ù', 'U'},
   {L'Ú', 'U'},
   {L'Û', 'U'},
   {L'Ü', 'U'},
   {L'Ý', 'Y'},
   {L'ß', 's'},
   {L'à', 'a'},
   {L'á', 'a'},
   {L'â', 'a'},
   {L'ã', 'a'},
   {L'ä', 'a'},
   {L'å', 'a'},
   {L'æ', 'a'},
   {L'ç', 'c'},
   {L'è', 'e'},
   {L'é', 'e'},
   {L'ê', 'e'},
   {L'ë', 'e'},
   {L'ì', 'i'},
   {L'í', 'i'},
   {L'î', 'i'},
   {L'ï', 'i'},
   {L'ñ', 'n'},
   {L'ò', 'o'},
   {L'ó', 'o'},
   {L'ô', 'o'},
   {L'õ', 'o'},
   {L'ö', 'o'},
   {L'ø', 'o'},
   {L'ù', 'u'},
   {L'ú', 'u'},
   {L'û', 'u'},
   {L'ü', 'u'},
   {L'ý', 'y'},
   {L'Ą', 'A'},
   {L'ą', 'a'},
   {L'Ć', 'C'},
   {L'ć', 'c'},
   {L'Ę', 'E'},
   {L'ę', 'e'},
   {L'Ł', 'L'},
   {L'ł', 'l'},
   {L'Ń', 'N'},
   {L'ń', 'n'},
   {L'Ŕ', 'R'},
   {L'ŕ', 'r'},
   {L'Ś', 'S'},
   {L'ś', 's'},
   {L'Ź', 'Z'},
   {L'ź', 'z'},
   {L'Ż', 'Z'},
   {L'ż', 'z'},
   {L'ẞ', 'S'},
};
/******************************************************************************/
// CHARACTER
/******************************************************************************/
INLINE Int CharOrderFast(Char8 c) {return CharOrder8 [U8 (c)];}
INLINE Int CharOrderFast(Char  c) {return CharOrder16[U16(c)];}

UInt CharFlagFast(Char8 a, Char8 b) {return CharFlagFast(a)|CharFlagFast(b);}
UInt CharFlagFast(Char  a, Char  b) {return CharFlagFast(a)|CharFlagFast(b);}

INLINE Char  CaseDownFast(Char  c) {return _CaseDown[U16(c)];}
INLINE Char  CaseUpFast  (Char  c) {return _CaseUp  [U16(c)];}
INLINE Char8 CaseDownFast(Char8 c) {return  CaseDownFast(Char8To16Fast(c));}
INLINE Char8 CaseUpFast  (Char8 c) {return  CaseUpFast  (Char8To16Fast(c));}

CHAR_TYPE CharTypeFast(Char c) // don't INLINE because it's not a simple function
{
   UInt f=CharFlagFast(c);
   if(  f&(CHARF_ALPHA|CHARF_DIG|CHARF_UNDER))return CHART_CHAR ; // check this first in case '_' is both CHARF_UNDER and CHARF_SIGN
   if(  f& CHARF_SPACE                       )return CHART_SPACE;
   if(  f& CHARF_SIGN                        )return CHART_SIGN ;
                                              return CHART_NONE ;
}
INLINE CHAR_TYPE CharTypeFast(Char8 c) {return CharTypeFast(Char8To16Fast(c));}

INLINE Bool IsDigit(CChar8 c) {return c>='0' && c<='9';}
INLINE Bool IsDigit(CChar  c) {return c>='0' && c<='9';}

INLINE Bool EqualCSFast(Char8 a, Char8 b) {return a==b;}
INLINE Bool EqualCSFast(Char8 a, Char  b) {return Char8To16Fast(a)==b;}
INLINE Bool EqualCSFast(Char  a, Char8 b) {return a==Char8To16Fast(b);}
INLINE Bool EqualCSFast(Char  a, Char  b) {return a==b;}

INLINE Bool EqualCIFast(Char8 a, Char8 b) {return CharOrderFast(a)==CharOrderFast(b);}
INLINE Bool EqualCIFast(Char8 a, Char  b) {return CharOrderFast(a)==CharOrderFast(b);}
INLINE Bool EqualCIFast(Char  a, Char8 b) {return CharOrderFast(a)==CharOrderFast(b);}
INLINE Bool EqualCIFast(Char  a, Char  b) {return CharOrderFast(a)==CharOrderFast(b);}
/******************************************************************************/
Int CharOrder(Char8 c) {I(); return CharOrderFast(c);}
Int CharOrder(Char  c) {I(); return CharOrderFast(c);}

Char  Char8To16(Char8 c) {I(); return Char8To16Fast(c);}
Char8 Char16To8(Char  c) {I(); return Char16To8Fast(c);}

UInt CharFlag(Char  c) {I(); return CharFlagFast(c);}
UInt CharFlag(Char8 c) {I(); return CharFlagFast(c);}

Char  CaseDown(Char  c) {I(); return CaseDownFast(c);}
Char  CaseUp  (Char  c) {I(); return CaseUpFast  (c);}
Char8 CaseDown(Char8 c) {I(); return CaseDownFast(c);}
Char8 CaseUp  (Char8 c) {I(); return CaseUpFast  (c);}

CHAR_TYPE CharType(Char  c) {I(); return CharTypeFast(c);}
CHAR_TYPE CharType(Char8 c) {I(); return CharTypeFast(c);}

Bool WhiteChar(Char c) {return c==' ' || c=='\t' || c=='\n' || c=='\r' || c==FullWidthSpace || c==Nbsp;}

Int CompareCS(Char8 a, Char8 b) {return U8 (          a )-U8 (          b );}
Int CompareCS(Char8 a, Char  b) {return U16(Char8To16(a))-U16(          b );}
Int CompareCS(Char  a, Char8 b) {return U16(          a )-U16(Char8To16(b));}
Int CompareCS(Char  a, Char  b) {return U16(          a )-U16(          b );}

Int CompareCI(Char8 a, Char8 b) {I(); return CharOrderFast(a)-CharOrderFast(b);}
Int CompareCI(Char8 a, Char  b) {I(); return CharOrderFast(a)-CharOrderFast(b);}
Int CompareCI(Char  a, Char8 b) {I(); return CharOrderFast(a)-CharOrderFast(b);}
Int CompareCI(Char  a, Char  b) {I(); return CharOrderFast(a)-CharOrderFast(b);}

Int Compare(Char8 a, Char8 b, Bool case_sensitive) {return case_sensitive ? CompareCS(a, b) : CompareCI(a, b);}
Int Compare(Char8 a, Char  b, Bool case_sensitive) {return case_sensitive ? CompareCS(a, b) : CompareCI(a, b);}
Int Compare(Char  a, Char8 b, Bool case_sensitive) {return case_sensitive ? CompareCS(a, b) : CompareCI(a, b);}
Int Compare(Char  a, Char  b, Bool case_sensitive) {return case_sensitive ? CompareCS(a, b) : CompareCI(a, b);}

Bool EqualCS(Char8 a, Char8 b) {I(); return EqualCSFast(a, b);}
Bool EqualCS(Char8 a, Char  b) {I(); return EqualCSFast(a, b);}
Bool EqualCS(Char  a, Char8 b) {I(); return EqualCSFast(a, b);}
Bool EqualCS(Char  a, Char  b) {I(); return EqualCSFast(a, b);}

Bool EqualCI(Char8 a, Char8 b) {I(); return EqualCIFast(a, b);}
Bool EqualCI(Char8 a, Char  b) {I(); return EqualCIFast(a, b);}
Bool EqualCI(Char  a, Char8 b) {I(); return EqualCIFast(a, b);}
Bool EqualCI(Char  a, Char  b) {I(); return EqualCIFast(a, b);}

Bool Equal(Char8 a, Char8 b, Bool case_sensitive) {return case_sensitive ? EqualCS(a, b) : EqualCI(a, b);}
Bool Equal(Char8 a, Char  b, Bool case_sensitive) {return case_sensitive ? EqualCS(a, b) : EqualCI(a, b);}
Bool Equal(Char  a, Char8 b, Bool case_sensitive) {return case_sensitive ? EqualCS(a, b) : EqualCI(a, b);}
Bool Equal(Char  a, Char  b, Bool case_sensitive) {return case_sensitive ? EqualCS(a, b) : EqualCI(a, b);}

Int CharInt(Char c)
{
   if(c>='0' && c<='9')return c-'0';
   if(c>='a' && c<='f')return c-'a'+0xA;
   if(c>='A' && c<='F')return c-'A'+0xA;
   return -1;
}

CChar8* CharName(Char c)
{
   switch(c)
   {
      case L' ' : return "Space";
      case L'`' : return "Tilde";
      case L'\t': return "Tab";
      case L'\n': return "Enter";
   }
   return null;
}
/******************************************************************************/
// TEXT
/******************************************************************************/
Int Length(CChar8 *t)
{
   Int    length=0; if(t)for(; *t++; length++);
   return length;
}
Int Length(CChar *t)
{
   Int    length=0; if(t)for(; *t++; length++);
   return length;
}
Int Length(C wchar_t *t)
{
   Int    length=0; if(t)for(; *t++; length++);
   return length;
}
/******************************************************************************/
Bool HasUnicode(CChar  *t) {if(t)for(; *t; )if(HasUnicode(*t++))return true; return false;}
Bool HasUnicode(CChar8 *t) {if(t)for(; *t; )if(HasUnicode(*t++))return true; return false;}
Bool HasUnicode(C Str  &s) {FREPA(s)        if(HasUnicode(s[i]))return true; return false;} // keep this function to allow having '\0' chars in the middle
Bool HasUnicode(C Str8 &s) {FREPA(s)        if(HasUnicode(s[i]))return true; return false;} // keep this function to allow having '\0' chars in the middle
/******************************************************************************/
Char8* Set(Char8 *dest, CChar8 *src, Int dest_elms)
{
   Char8 *ret=dest;
   if(dest && dest_elms>0)
   {
      if(src)for(; --dest_elms && src[0]; )*dest++=*src++;
     *dest='\0';
   }
   return ret;
}
Char8* Set(Char8 *dest, CChar *src, Int dest_elms)
{
   Char8 *ret=dest;
   if(dest && dest_elms>0)
   {
      if(src)for(I(); --dest_elms && src[0]; )*dest++=Char16To8Fast(*src++);
     *dest='\0';
   }
   return ret;
}
Char* Set(Char *dest, CChar8 *src, Int dest_elms)
{
   Char *ret=dest;
   if(dest && dest_elms>0)
   {
      if(src)for(I(); --dest_elms && src[0]; )*dest++=Char8To16Fast(*src++);
     *dest='\0';
   }
   return ret;
}
Char* Set(Char *dest, CChar *src, Int dest_elms)
{
   Char *ret=dest;
   if(dest && dest_elms>0)
   {
      if(src)for(; --dest_elms && src[0]; )*dest++=*src++;
     *dest='\0';
   }
   return ret;
}
Char8* _Set(Char8 *dest, C wchar_t *src, Int dest_elms)
{
   Char8 *ret=dest;
   if(dest && dest_elms>0)
   {
      if(src)for(I(); --dest_elms && src[0]; )*dest++=Char16To8Fast(*src++);
     *dest='\0';
   }
   return ret;
}
Char* _Set(Char *dest, C wchar_t *src, Int dest_elms)
{
   Char *ret=dest;
   if(dest && dest_elms>0)
   {
      if(src)for(; --dest_elms && src[0]; )*dest++=*src++;
     *dest='\0';
   }
   return ret;
}
/****************************************************************************/
Int SetReturnLength(Char8 *dest, CChar8 *src, Int dest_elms)
{
   Int length=0;
   if(dest && dest_elms>0)
   {
      if(src)for(; --dest_elms && src[0]; length++)*dest++=*src++;
      *dest='\0';
   }
   return length;
}
Int SetReturnLength(Char *dest, CChar *src, Int dest_elms)
{
   Int length=0;
   if(dest && dest_elms>0)
   {
      if(src)for(; --dest_elms && src[0]; length++)*dest++=*src++;
      *dest='\0';
   }
   return length;
}
/****************************************************************************/
void MergePath(Char *dest, CChar *first, CChar *second, Int dest_elms)
{
   Int l=SetReturnLength(dest, first, dest_elms);
   if( l && l+1<dest_elms)if(dest[l-1]!='\\' && dest[l-1]!='/'){dest[l++]='\\'; dest[l]='\0';} // add tail slash if needed
   Set(dest+l, second, dest_elms-l);
}
void MergePath(Char8 *dest, CChar8 *first, CChar8 *second, Int dest_elms)
{
   Int l=SetReturnLength(dest, first, dest_elms);
   if( l && l+1<dest_elms)if(dest[l-1]!='\\' && dest[l-1]!='/'){dest[l++]='\\'; dest[l]='\0';} // add tail slash if needed
   Set(dest+l, second, dest_elms-l);
}
/****************************************************************************/
Char8* Append(Char8 *dest, CChar8 *src, Int dest_elms)
{
   Char8 *ret=dest;
   if(dest)
   {
      Int length=Length(dest);
      dest_elms-=length;
      dest     +=length;
      if(dest_elms>1 && src)
      {
         for(; --dest_elms && src[0]; )*dest++=*src++;
         *dest='\0';
      }
   }
   return ret;
}
Char* Append(Char *dest, CChar8 *src, Int dest_elms)
{
   Char *ret=dest;
   if(dest)
   {
      Int length=Length(dest);
      dest_elms-=length;
      dest     +=length;
      if(dest_elms>1 && src)
      {
         for(I(); --dest_elms && src[0]; )*dest++=Char8To16Fast(*src++);
         *dest='\0';
      }
   }
   return ret;
}
Char8* Append(Char8 *dest, CChar *src, Int dest_elms)
{
   Char8 *ret=dest;
   if(dest)
   {
      Int length=Length(dest);
      dest_elms-=length;
      dest     +=length;
      if(dest_elms>1 && src)
      {
         for(I(); --dest_elms && src[0]; )*dest++=Char16To8Fast(*src++);
         *dest='\0';
      }
   }
   return ret;
}
Char* Append(Char *dest, CChar *src, Int dest_elms)
{
   Char *ret=dest;
   if(dest)
   {
      Int length=Length(dest);
      dest_elms-=length;
      dest     +=length;
      if(dest_elms>1 && src)
      {
         for(; --dest_elms && src[0]; )*dest++=*src++;
         *dest='\0';
      }
   }
   return ret;
}
/****************************************************************************/
Int Compare(CChar8 *a, CChar8 *b, Bool case_sensitive)
{
   if(a && b)
   {
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=*a,
             b_order=*b;
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }else
         for(I(); ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int Compare(CChar8 *a, CChar *b, Bool case_sensitive)
{
   if(a && b)
   {
      I();
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=Char8To16Fast(*a),
             b_order=              *b ;
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }else
         for(; ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int Compare(CChar *a, CChar8 *b, Bool case_sensitive)
{
   if(a && b)
   {
      I();
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=              *a ,
             b_order=Char8To16Fast(*b);
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }else
         for(; ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int Compare(CChar *a, CChar *b, Bool case_sensitive)
{
   if(a && b)
   {
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=*a,
             b_order=*b;
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }else
         for(I(); ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if(a_order<b_order)return -1;
         if(a_order>b_order)return +1;
         if(!*a            )return  0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
/****************************************************************************/
Int ComparePath(CChar *a, CChar *b, Bool case_sensitive)
{
   if(a && b)
   {
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=*a,
             b_order=*b;
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
               return -1; // needed only for case sensitive as CharOrder has slashes first
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
               return +1; // needed only for case sensitive as CharOrder has slashes first
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }else
         for(I(); ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int ComparePath(CChar *a, CChar8 *b, Bool case_sensitive)
{
   if(a && b)
   {
      I();
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=             (*a),
             b_order=Char8To16Fast(*b);
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
               return -1; // needed only for case sensitive as CharOrder has slashes first
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
               return +1; // needed only for case sensitive as CharOrder has slashes first
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }else
         for(; ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int ComparePath(CChar8 *a, CChar *b, Bool case_sensitive)
{
   if(a && b)
   {
      I();
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=Char8To16Fast(*a),
             b_order=             (*b);
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
               return -1; // needed only for case sensitive as CharOrder has slashes first
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
               return +1; // needed only for case sensitive as CharOrder has slashes first
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }else
         for(; ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int ComparePath(CChar8 *a, CChar8 *b, Bool case_sensitive)
{
   if(a && b)
   {
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Int a_order=*a,
             b_order=*b;
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
               return -1; // needed only for case sensitive as CharOrder has slashes first
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
               return +1; // needed only for case sensitive as CharOrder has slashes first
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }else
         for(I(); ; a++, b++)
      {
         Int a_order=CharOrderFast(*a),
             b_order=CharOrderFast(*b);
         if( a_order!=b_order)
         {
            Bool as=IsSlash(a[0]),
                 bs=IsSlash(b[0]);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !b[0])return 0; // treat "c:/" and "c:" as the same
            }
            if(bs)
            {
               if(!b[1] && !a[0])return 0; // treat "c:" and "c:/" as the same
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!*a)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
/****************************************************************************/
static Int CompareNumberLength(CChar8 *a, CChar8 *b)
{
   for(;;)
   {
      Bool ad=IsDigit(*a++),
           bd=IsDigit(*b++);
      if(!ad)return bd ? -1 : 0; // !ad &&  bd -> -1, !ad && !bd -> 0
      if(!bd)return      +1    ; //  ad && !bd -> +1
   }
}
static Int CompareNumberLength(CChar *a, CChar *b)
{
   for(;;)
   {
      Bool ad=IsDigit(*a++),
           bd=IsDigit(*b++);
      if(!ad)return bd ? -1 : 0; // !ad &&  bd -> -1, !ad && !bd -> 0
      if(!bd)return      +1    ; //  ad && !bd -> +1
   }
}
/****************************************************************************/
Int CompareNumber(CChar *a, CChar *b, Bool case_sensitive)
{
   if(a && b)
   {
      CChar *a_start=a;
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Char ac=*a, bc=*b;
         if(ac!=bc)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            return (U16(ac)<U16(bc)) ? -1 : +1;
         }
         if(!ac)return 0;
      }else
         for(I(); ; a++, b++)
      {
         Char ac=*a, bc=*b;
         if(ac!=bc)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            Int a_order=CharOrderFast(ac),
                b_order=CharOrderFast(bc);
            if( a_order<b_order)return -1;
            if( a_order>b_order)return +1;
         }
         if(!ac)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int CompareNumber(CChar8 *a, CChar8 *b, Bool case_sensitive)
{
   if(a && b)
   {
      CChar8 *a_start=a;
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Char8 ac=*a, bc=*b;
         if(ac!=bc)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            return (U8(ac)<U8(bc)) ? -1 : +1;
         }
         if(!ac)return 0;
      }else
         for(I(); ; a++, b++)
      {
         Char8 ac=*a, bc=*b;
         if(ac!=bc)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            Int a_order=CharOrderFast(ac),
                b_order=CharOrderFast(bc);
            if( a_order<b_order)return -1;
            if( a_order>b_order)return +1;
         }
         if(!ac)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
/****************************************************************************/
Int ComparePathNumber(CChar *a, CChar *b, Bool case_sensitive)
{
   if(a && b)
   {
      CChar *a_start=a;
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Char ac=*a, bc=*b;
         if(ac!=bc)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            Bool as=IsSlash(ac),
                 bs=IsSlash(bc);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !bc)return 0; // treat "c:/" and "c:" as the same
               return -1; // needed only for case sensitive as CharOrder has slashes first
            }
            if(bs)
            {
               if(!b[1] && !ac)return 0; // treat "c:" and "c:/" as the same
               return +1; // needed only for case sensitive as CharOrder has slashes first
            }
            return (U16(ac)<U16(bc)) ? -1 : +1;
         }
         if(!ac)return 0;
      }else
         for(I(); ; a++, b++)
      {
         Char ac=*a, bc=*b;
         Int a_order=CharOrderFast(ac),
             b_order=CharOrderFast(bc);
         if( a_order!=b_order)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            Bool as=IsSlash(ac),
                 bs=IsSlash(bc);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !bc)return 0; // treat "c:/" and "c:" as the same
            }
            if(bs)
            {
               if(!b[1] && !ac)return 0; // treat "c:" and "c:/" as the same
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!ac)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
Int ComparePathNumber(CChar8 *a, CChar8 *b, Bool case_sensitive)
{
   if(a && b)
   {
      CChar8 *a_start=a;
      if(case_sensitive)
         for(; ; a++, b++)
      {
         Char8 ac=*a, bc=*b;
         if(ac!=bc)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            Bool as=IsSlash(ac),
                 bs=IsSlash(bc);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !bc)return 0; // treat "c:/" and "c:" as the same
               return -1; // needed only for case sensitive as CharOrder has slashes first
            }
            if(bs)
            {
               if(!b[1] && !ac)return 0; // treat "c:" and "c:/" as the same
               return +1; // needed only for case sensitive as CharOrder has slashes first
            }
            return (U8(ac)<U8(bc)) ? -1 : +1;
         }
         if(!ac)return 0;
      }else
         for(I(); ; a++, b++)
      {
         Char8 ac=*a, bc=*b;
         Int a_order=CharOrderFast(ac),
             b_order=CharOrderFast(bc);
         if( a_order!=b_order)
         {
            if(              IsDigit(ac   ) && IsDigit(bc   )  // if both are  digits
            || a!=a_start && IsDigit(a[-1]) && IsDigit(b[-1])) // if both were digits in previous step
               if(Int c=CompareNumberLength(a, b))return c;

            Bool as=IsSlash(ac),
                 bs=IsSlash(bc);
            if(as)
            {
               if(bs)continue; // if both have slashes
               if(!a[1] && !bc)return 0; // treat "c:/" and "c:" as the same
            }
            if(bs)
            {
               if(!b[1] && !ac)return 0; // treat "c:" and "c:/" as the same
            }
            return (a_order<b_order) ? -1 : +1;
         }
         if(!ac)return 0;
      }
   }
   if(a && a[0])return +1;
   if(b && b[0])return -1;
                return  0;
}
/****************************************************************************/
Bool Starts(CChar8 *t, CChar8 *start, Bool case_sensitive, Bool whole_words)
{
   if(t && start)
   {
      I();
      Char8 last_start='\0';
      if(case_sensitive)
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCSFast(*t, *start))return false; last_start=*start;
      }else
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCIFast(*t, *start))return false; last_start=*start;
      }
   }
   return false;
}
Bool Starts(CChar *t, CChar8 *start, Bool case_sensitive, Bool whole_words)
{
   if(t && start)
   {
      I();
      Char8 last_start='\0';
      if(case_sensitive)
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCSFast(*t, *start))return false; last_start=*start;
      }else
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCIFast(*t, *start))return false; last_start=*start;
      }
   }
   return false;
}
Bool Starts(CChar8 *t, CChar *start, Bool case_sensitive, Bool whole_words)
{
   if(t && start)
   {
      I();
      Char last_start='\0';
      if(case_sensitive)
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCSFast(*t, *start))return false; last_start=*start;
      }else
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCIFast(*t, *start))return false; last_start=*start;
      }
   }
   return false;
}
Bool Starts(CChar *t, CChar *start, Bool case_sensitive, Bool whole_words)
{
   if(t && start)
   {
      I();
      Char last_start='\0';
      if(case_sensitive)
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCSFast(*t, *start))return false; last_start=*start;
      }else
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR)return CharTypeFast(t[0])!=CHART_CHAR;
            return true;
         }
         if(!EqualCIFast(*t, *start))return false; last_start=*start;
      }
   }
   return false;
}
/****************************************************************************/
Bool StartsSkipSpace(CChar *t, CChar *start, Int &match_length, Bool case_sensitive, Bool whole_words=false)
{
   if(t && start)
   {
      I();
     CChar *t_start=t;
      Char   last_t='\0', last_start='\0';
      if(case_sensitive)
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR && CharTypeFast(t[0])==CHART_CHAR)return false;
            match_length=t-t_start;
            return true;
         }
         last_start=*start;
         Char c=*t;
         if(!EqualCSFast(c, last_start))
         {
            if(c==' ')for(t++; ; t++)
            {
               c=*t;
               if(c==' ')continue;
               if(!EqualCSFast(c, last_start)
               || CharTypeFast(last_t)==CHART_CHAR && CharTypeFast(c)==CHART_CHAR)break; // if both are chars, then we can't merge
               goto next_cs;
            }
            return false;
         }
      next_cs:
         last_t=c;
      }else
         for(; ; t++, start++)
      {
         if(!start[0])
         {
            if(whole_words && CharTypeFast(last_start)==CHART_CHAR && CharTypeFast(t[0])==CHART_CHAR)return false;
            match_length=t-t_start;
            return true;
         }
         last_start=*start;
         Char c=*t;
         if(!EqualCIFast(c, last_start))
         {
            if(c==' ')for(t++; ; t++)
            {
               c=*t;
               if(c==' ')continue;
               if(!EqualCIFast(c, last_start)
               || CharTypeFast(last_t)==CHART_CHAR && CharTypeFast(c)==CHART_CHAR)break; // if both are chars, then we can't merge
               goto next_ci;
            }
            return false;
         }
      next_ci:
         last_t=c;
      }
   }
   return false;
}
/****************************************************************************/
Bool Ends(CChar8 *t, CChar8 *end, Bool case_sensitive)
{
   Int t_len=Length(t  ),
       e_len=Length(end);
   return (e_len<=t_len) ? Equal(t+t_len-e_len, end, case_sensitive) : false;
}
Bool Ends(CChar *t, CChar8 *end, Bool case_sensitive)
{
   Int t_len=Length(t  ),
       e_len=Length(end);
   return (e_len<=t_len) ? Equal(t+t_len-e_len, end, case_sensitive) : false;
}
Bool Ends(CChar8 *t, CChar *end, Bool case_sensitive)
{
   Int t_len=Length(t  ),
       e_len=Length(end);
   return (e_len<=t_len) ? Equal(t+t_len-e_len, end, case_sensitive) : false;
}
Bool Ends(CChar *t, CChar *end, Bool case_sensitive)
{
   Int t_len=Length(t  ),
       e_len=Length(end);
   return (e_len<=t_len) ? Equal(t+t_len-e_len, end, case_sensitive) : false;
}
/****************************************************************************/
Bool StartsPath(CChar8 *t, CChar8 *start)
{
   if(!Is(start))return true;
   if(t)for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss       )continue; // both have slashes
            if(!start[0])return true;
         }
         if(ss && !start[1] && !t[0])return true;
         if(!start[0] && IsSlash(t[-1]))return true;
         return false;
      }
      if(!t[0])return true;
   }
   return false;
}
Bool StartsPath(CChar8 *t, CChar *start)
{
   if(!Is(start))return true;
   if(t)for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss       )continue; // both have slashes
            if(!start[0])return true;
         }
         if(ss && !start[1] && !t[0])return true;
         if(!start[0] && IsSlash(t[-1]))return true;
         return false;
      }
      if(!t[0])return true;
   }
   return false;
}
Bool StartsPath(CChar *t, CChar8 *start)
{
   if(!Is(start))return true;
   if(t)for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss       )continue; // both have slashes
            if(!start[0])return true;
         }
         if(ss && !start[1] && !t[0])return true;
         if(!start[0] && IsSlash(t[-1]))return true;
         return false;
      }
      if(!t[0])return true;
   }
   return false;
}
Bool StartsPath(CChar *t, CChar *start)
{
   if(!Is(start))return true;
   if(t)for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss       )continue; // both have slashes
            if(!start[0])return true;
         }
         if(ss && !start[1] && !t[0])return true;
         if(!start[0] && IsSlash(t[-1]))return true;
         return false;
      }
      if(!t[0])return true;
   }
   return false;
}
/****************************************************************************/
Bool Contains(CChar  *src, Char  c) {if(src)for(; src[0]; src++)if(src[0]==c)return true; return false;}
Bool Contains(CChar8 *src, Char8 c) {if(src)for(; src[0]; src++)if(src[0]==c)return true; return false;}
/****************************************************************************/
Bool Contains(CChar8 *src, CChar8 *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      if(!t[0])return true;

      I();
      Char8 last    ='\0';
      Int   order   =CharOrderFast(*t),
            t_length=Length(t);
      Bool  dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(;;)
      {
         Char8 c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))
         {
            return true;
         }
         last=c;
         src++;
      }
   }
   return false;
}
Bool Contains(CChar *src, CChar *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      if(!t[0])return true;

      I();
      Char last    ='\0';
      Int  order   =CharOrderFast(*t),
           t_length=Length(t);
      Bool dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(;;)
      {
         Char c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))
         {
            return true;
         }
         last=c;
         src++;
      }
   }
   return false;
}
Bool Contains(CChar *src, CChar8 *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      if(!t[0])return true;

      I();
      Char last    ='\0';
      Int  order   =CharOrderFast(*t),
           t_length=Length(t);
      Bool dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(;;)
      {
         Char c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))
         {
            return true;
         }
         last=c;
         src++;
      }
   }
   return false;
}
Bool Contains(CChar8 *src, CChar *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      if(!t[0])return true;

      I();
      Char8 last    ='\0';
      Int   order   =CharOrderFast(*t),
            t_length=Length(t);
      Bool  dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(;;)
      {
         Char8 c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))
         {
            return true;
         }
         last=c;
         src++;
      }
   }
   return false;
}
/****************************************************************************/
Bool ContainsAny(CChar *src, CChar *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      Memt<Char> word; for(CChar *start=t; ; )
      {
         Char c=*t++; if(c==' ' || c=='\0')
         {
            Int len_1=t-start; // this will include nul char
            if( len_1>1)       // ignore empty words (in case 't' has "  ")
            {
               word.clear().setNum(len_1); // clear first to avoid copying existing data in 'setNum'
               Set(word.data(), start, word.elms());
               if(Contains(src, word.data(), case_sensitive, whole_words))return true;
            }
            if(c=='\0')break;
            start=t;
         }
      }
   }
   return false;
}
/****************************************************************************/
Bool ContainsAll(CChar *src, CChar *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      Memt<Char> word; for(CChar *start=t; ; )
      {
         Char c=*t++; if(c==' ' || c=='\0')
         {
            Int len_1=t-start; // this will include nul char
            if( len_1>1)       // ignore empty words (in case 't' has "  ")
            {
               word.clear().setNum(len_1); // clear first to avoid copying existing data in 'setNum'
               Set(word.data(), start, word.elms());
               if(!Contains(src, word.data(), case_sensitive, whole_words))return false;
            }
            if(c=='\0')return true;
            start=t;
         }
      }
   }
   return false;
}
Bool ContainsAll(CChar *src, CChar8 *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      Memt<Char8> word; for(CChar8 *start=t; ; )
      {
         Char8 c=*t++; if(c==' ' || c=='\0')
         {
            Int len_1=t-start; // this will include nul char
            if( len_1>1)       // ignore empty words (in case 't' has "  ")
            {
               word.clear().setNum(len_1); // clear first to avoid copying existing data in 'setNum'
               Set(word.data(), start, word.elms());
               if(!Contains(src, word.data(), case_sensitive, whole_words))return false;
            }
            if(c=='\0')return true;
            start=t;
         }
      }
   }
   return false;
}
Bool ContainsAll(CChar8 *src, CChar *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      Memt<Char> word; for(CChar *start=t; ; )
      {
         Char c=*t++; if(c==' ' || c=='\0')
         {
            Int len_1=t-start; // this will include nul char
            if( len_1>1)       // ignore empty words (in case 't' has "  ")
            {
               word.clear().setNum(len_1); // clear first to avoid copying existing data in 'setNum'
               Set(word.data(), start, word.elms());
               if(!Contains(src, word.data(), case_sensitive, whole_words))return false;
            }
            if(c=='\0')return true;
            start=t;
         }
      }
   }
   return false;
}
Bool ContainsAll(CChar8 *src, CChar8 *t, Bool case_sensitive, Bool whole_words)
{
   if(src && t)
   {
      Memt<Char8> word; for(CChar8 *start=t; ; )
      {
         Char8 c=*t++; if(c==' ' || c=='\0')
         {
            Int len_1=t-start; // this will include nul char
            if( len_1>1)       // ignore empty words (in case 't' has "  ")
            {
               word.clear().setNum(len_1); // clear first to avoid copying existing data in 'setNum'
               Set(word.data(), start, word.elms());
               if(!Contains(src, word.data(), case_sensitive, whole_words))return false;
            }
            if(c=='\0')return true;
            start=t;
         }
      }
   }
   return false;
}
/****************************************************************************/
CChar * _SkipWhiteChars(CChar  *t) {if(t)for(; WhiteChar(*t); t++); return t;}
CChar8* _SkipWhiteChars(CChar8 *t) {if(t)for(; WhiteChar(*t); t++); return t;}
/****************************************************************************/
CChar8* _SkipStart(CChar8 *t, CChar8 *start)
{
   CChar8 *ret=t;
   if(t && start)for(I(); ; t++, start++)
   {
      if(!start[0])return t;
      if(!EqualCIFast(*t, *start))return ret;
   }
   return ret;
}
CChar8* _SkipStart(CChar8 *t, CChar *start)
{
   CChar8 *ret=t;
   if(t && start)for(I(); ; t++, start++)
   {
      if(!start[0])return t;
      if(!EqualCIFast(*t, *start))return ret;
   }
   return ret;
}
CChar* _SkipStart(CChar *t, CChar8 *start)
{
   CChar *ret=t;
   if(t && start)for(I(); ; t++, start++)
   {
      if(!start[0])return t;
      if(!EqualCIFast(*t, *start))return ret;
   }
   return ret;
}
CChar* _SkipStart(CChar *t, CChar *start)
{
   CChar *ret=t;
   if(t && start)for(I(); ; t++, start++)
   {
      if(!start[0])return t;
      if(!EqualCIFast(*t, *start))return ret;
   }
   return ret;
}
/****************************************************************************/
CChar8* _SkipStartPath(CChar8 *t, CChar8 *start)
{
   CChar8 *ret=t;
   if(t && Is(start))for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss)continue; // both have slashes
            if(!start[0])return t+1;
         }
         if(ss && !start[1] && !t[0])return null;
         if(!start[0] && IsSlash(t[-1]))return t; // here t[0] will never be '\0' because "start[0]=='\0' && !EqualCIFast(*t, *start)"
         return ret;
      }
      if(!t[0])return null;
   }
   return ret;
}
CChar* _SkipStartPath(CChar *t, CChar *start)
{
   CChar *ret=t;
   if(t && Is(start))for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss)continue; // both have slashes
            if(!start[0])return t+1;
         }
         if(ss && !start[1] && !t[0])return null;
         if(!start[0] && IsSlash(t[-1]))return t; // here t[0] will never be '\0' because "start[0]=='\0' && !EqualCIFast(*t, *start)"
         return ret;
      }
      if(!t[0])return null;
   }
   return ret;
}
CChar* _AfterPath(CChar *t, CChar *start)
{
   if(t && Is(start))for(I(); ; t++, start++)
   {
      if(!EqualCIFast(*t, *start))
      {
         Bool ts=IsSlash(t    [0]),
              ss=IsSlash(start[0]);
         if(ts)
         {
            if(ss)continue; // both have slashes
            if(!start[0])return t[1] ? t+1 : null; // !! return null instead of an empty string "", to have consistency so that '_AfterPath' will return null in all 4 cases: !_AfterPath(u"abc", u"abc") && !_AfterPath(u"abc/", u"abc") && !_AfterPath(u"abc", u"abc/") && !_AfterPath(u"abc/", u"abc/"), also in the engine where '_AfterPath' is used, there is a quick check of it being non null only !!
         }
         if(ss && !start[1] && !t[0])break;
         if(!start[0] && IsSlash(t[-1]))return t; // here t[0] will never be '\0' because "start[0]=='\0' && !EqualCIFast(*t, *start)"
         break;
      }
      if(!t[0])break;
   }
   return null;
}
/****************************************************************************/
Str SkipWhiteChars(C Str &t              ) {return _SkipWhiteChars(t       );}
Str SkipStartPath (C Str &t, C Str &start) {return _SkipStartPath (t, start);}
Str SkipStart     (C Str &t, C Str &start) {return _SkipStart     (t, start);}
Str SkipEnd       (C Str &t, C Str &end  )
{
   if(end.length()<=t.length() && Equal(t()+t.length()-end.length(), end))return Str(t).clip(t.length()-end.length()); return t;
}
/******************************************************************************/
Char* ReplaceSelf(Char *text, Char from, Char to)
{
   if(text)for(Char *src=text, *dest=text; ; )
   {
      Char c=*src++;
      if( !c      ){*dest='\0'; break;}
      if(  c==from)c=to;
      if(  c      )*dest++=c;
   }
   return text;
}
Char8* ReplaceSelf(Char8 *text, Char8 from, Char8 to)
{
   if(text)for(Char8 *src=text, *dest=text; ; )
   {
      Char8 c=*src++;
      if(  !c      ){*dest='\0'; break;}
      if(   c==from)c=to;
      if(   c      )*dest++=c;
   }
   return text;
}
/****************************************************************************/
Str Replace(C Str &text, Char from, Char to)
{
   Str s; s.reserve(text.length()); FREPA(text)
   {
      Char c=text()[i]; // () avoids range checks
      s+=((c==from) ? to : c);
   }
   return s;
}
/****************************************************************************/
Str Replace(C Str &text, C Str &from, C Str &to, Bool case_sensitive, Bool whole_words)
{
   if(text.is() && from.is())
   {
      I();
      Bool   dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(from.first())!=CHART_CHAR); dont_check[1]=(CharTypeFast(from.last())!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      Char   last='\0';
      CChar *t   =text;
      Str    out;
      for(;;)
      {
         if(Starts(t, from, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(t[from.length()])!=CHART_CHAR) : true))
         {
            out +=to;
            t   +=from.length();
            last ='\0';
         }else
         {
            last=*t++; if(!last)break;
            out+=last;
         }
      }
      return out;
   }
   return text;
}
/******************************************************************************/
Str Trim(C Str &text, Int pos, Int length)
{
   if(pos<0){length+=pos; pos=0;}  // clip left
   MIN(length, text.length()-pos); // clip right
   if( length>0) // this will also solve "pos>=text.length()"
   {
      Str s; s.reserve(length);
      CopyFastN(s._d.data(), text()+pos, length);
      s._d[s._length=length]='\0';
      return s;
   }
   return S;
}
Str8 Trim(C Str8 &text, Int pos, Int length)
{
   if(pos<0){length+=pos; pos=0;}  // clip left
   MIN(length, text.length()-pos); // clip right
   if( length>0) // this will also solve "pos>=text.length()"
   {
      Str8 s; s.reserve(length);
      CopyFastN(s._d.data(), text()+pos, length);
      s._d[s._length=length]='\0';
      return s;
   }
   return S;
}
/******************************************************************************/
Str CaseDown(C Str &t) {Str s; if(t.is()){s.reserve(t.length()); I(); REPA(t)s._d[i]=CaseDownFast(t()[i]); s._d[s._length=t.length()]='\0';} return s;} // () avoids range checks
Str CaseUp  (C Str &t) {Str s; if(t.is()){s.reserve(t.length()); I(); REPA(t)s._d[i]=CaseUpFast  (t()[i]); s._d[s._length=t.length()]='\0';} return s;} // () avoids range checks
/******************************************************************************/
void Split(MemPtr<Str> splits, C Str &string, Char separator)
{
   splits.clear().New();
   FREPA(string)
   {
      Char c=string[i];
      if(  c==separator)splits.New();else splits.last()+=c;
   }
}
Memc<Str> Split(C Str &string, Char separator) {Memc<Str> splits; Split(splits, string, separator); return splits;}
/******************************************************************************/
void Tokenize(MemPtr<Str> tokens, C Str &string)
{
   I();
   tokens.clear();
   CHAR_TYPE last_type=CHART_NONE;
   FREPA(string)
   {
      Char c=string[i]; CHAR_TYPE ct=CharTypeFast(c);
      if(ct==CHART_CHAR || ct==CHART_SIGN)
         ((ct==CHART_CHAR && last_type==ct) ? tokens.last() : tokens.New())+=c;
      last_type=ct;
   }
}
/******************************************************************************/
Int TextPosI(CChar8 *src, Char8 c)
{
   if(src)for(Int pos=0; ; pos++)
   {
      Char8 s=src[pos]; if(!s)break;
      if(s==c)return pos;
   }
   return -1;
}
Int TextPosI(CChar *src, Char c)
{
   if(src)for(Int pos=0; ; pos++)
   {
      Char s=src[pos]; if(!s)break;
      if(s==c)return pos;
   }
   return -1;
}
/****************************************************************************/
Int TextPosIN(CChar8 *src, Char8 c, Int i)
{
   if(src)for(Int found=0, pos=0; ; pos++)
   {
      Char8 s=src[pos]; if(!s)break;
      if(s==c)
      {
         if(found==i)return pos;
            found++;
      }
   }
   return -1;
}
Int TextPosIN(CChar *src, Char c, Int i)
{
   if(src)for(Int found=0, pos=0; ; pos++)
   {
      Char s=src[pos]; if(!s)break;
      if(s==c)
      {
         if(found==i)return pos;
            found++;
      }
   }
   return -1;
}
/****************************************************************************/
Int TextPosI(CChar *src, CChar *t, Bool case_sensitive, Bool whole_words)
{
   if(src && Is(t))
   {
      I();
      Char last    ='\0';
      Int  order   =CharOrderFast(*t),
           t_length=Length(t);
      Bool dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(Int pos=0; ; pos++)
      {
         Char c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))return pos;
         last=c;
         src++;
      }
   }
   return -1;
}
Int TextPosI(CChar *src, CChar8 *t, Bool case_sensitive, Bool whole_words)
{
   if(src && Is(t))
   {
      I();
      Char last    ='\0';
      Int  order   =CharOrderFast(*t),
           t_length=Length(t);
      Bool dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(Int pos=0; ; pos++)
      {
         Char c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))return pos;
         last=c;
         src++;
      }
   }
   return -1;
}
Int TextPosI(CChar8 *src, CChar8 *t, Bool case_sensitive, Bool whole_words)
{
   if(src && Is(t))
   {
      I();
      Char8 last    ='\0';
      Int   order   =CharOrderFast(*t),
            t_length=Length(t);
      Bool  dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(Int pos=0; ; pos++)
      {
         Char8 c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))return pos;
         last=c;
         src++;
      }
   }
   return -1;
}
/****************************************************************************/
Int TextPosIN(CChar8 *src, CChar8 *t, Int i, Bool case_sensitive, Bool whole_words)
{
   if(src && Is(t))
   {
      I();
      Char8 last    ='\0';
      Int   order   =CharOrderFast(*t),
            t_length=Length(t);
      Bool  dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(Int found=0, pos=0; ; pos++)
      {
         Char8 c=*src; if(!c)break;
         if(CharOrderFast(c)==order && Starts(src, t, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))
         {
            if(found==i)return pos;
               found++;
         }
         last=c;
         src++;
      }
   }
   return -1;
}
/****************************************************************************/
Int TextPosSkipSpaceI(CChar *src, CChar *t, Int &match_length, Bool case_sensitive, Bool whole_words)
{
   if(src && Is(t))
   {
      I();
      Char last    ='\0';
      Int  order   =CharOrderFast(*t),
           t_length=Length(t);
      Bool dont_check[2]; if(whole_words){dont_check[0]=(CharTypeFast(t[0])!=CHART_CHAR); dont_check[1]=(CharTypeFast(t[t_length-1])!=CHART_CHAR); if(dont_check[0] && dont_check[1])whole_words=false;}
      for(Int pos=0; ; pos++)
      {
         Char c=*src; if(!c)break;
         if(CharOrderFast(c)==order && StartsSkipSpace(src, t, match_length, case_sensitive) && (whole_words ? (dont_check[0] || CharTypeFast(last)!=CHART_CHAR) && (dont_check[1] || CharTypeFast(src[t_length])!=CHART_CHAR) : true))return pos;
         last=c;
         src++;
      }
   }
   return -1;
}
/****************************************************************************/
CChar * TextPos(CChar  *src,  Char   c                                       ) {Int p=TextPosI(src, c                             ); return (p<0) ? null : &src[p];}
CChar8* TextPos(CChar8 *src,  Char8  c                                       ) {Int p=TextPosI(src, c                             ); return (p<0) ? null : &src[p];}
CChar * TextPos(CChar  *src, CChar  *t, Bool case_sensitive, Bool whole_words) {Int p=TextPosI(src, t, case_sensitive, whole_words); return (p<0) ? null : &src[p];}
CChar * TextPos(CChar  *src, CChar8 *t, Bool case_sensitive, Bool whole_words) {Int p=TextPosI(src, t, case_sensitive, whole_words); return (p<0) ? null : &src[p];}
CChar8* TextPos(CChar8 *src, CChar8 *t, Bool case_sensitive, Bool whole_words) {Int p=TextPosI(src, t, case_sensitive, whole_words); return (p<0) ? null : &src[p];}
/******************************************************************************/
Str StrInside(C Str &str, C Str &from, C Str &to, Bool case_sensitive, Bool whole_words)
{
   if(CChar *inside=TextPos(str, from, case_sensitive, whole_words))
   {
      inside+=from.length();
      Int length=TextPosI(inside, to, case_sensitive, whole_words);
      if( length>0)
      {
         Str s; s.reserve(length); FREP(length)s+=inside[i];
         return s;
      }
   }
   return S;
}
/******************************************************************************/
// CONVERT
/******************************************************************************/
Str FromUTF8(CChar8 *text)
{
   Str out;
   if(text)for(;;)
   {
      Char c;
      Byte b0=(*text++);
      if(b0&(1<<7))
      {
         Byte b1=((*text++)&0x3F);
         if((b0&(1<<6)) && (b0&(1<<5)))
         {
            Byte b2=((*text++)&0x3F);
            if(b0&(1<<4))
            {
               Byte b3=((*text++)&0x3F);
               b0&=0x07;
               UInt u=(b3|(b2<<6)|(b1<<12)|(b0<<18));
               c=((u<=0xFFFF) ? u : '?');
            }else
            {
               b0&=0x0F;
               c=(b2|(b1<<6)|(b0<<12));
            }
         }else
         {
            b0&=0x1F;
            c=(b1|(b0<<6));
         }
      }else
      {
         c=b0;
      }

      if(c)out+=c;else break;
   }
   return out;
}
Str8 UTF8(C Str &text)
{
   Str8 out; out.reserve(text.length());
   FREPA(text)
   {
      Char c=text[i];

      if(c<=0x7F)out+=Char8(c);else
      {
         if(c<=0x7FF)
         {
            out+=Char8(0xC0 | (c>>6)); out+=Char8(0x80 | (c&0x3F));
         }else
         {
            out+=Char8(0xE0 | (c>>12)); out+=Char8(0x80 | ((c>>6)&0x3F)); out+=Char8(0x80 | (c&0x3F));
         }
      }
   }
   return out;
}
Str8 MultiByte(Int code_page, C Str &text)
{
#if WINDOWS
   if(text.is())
   {
      Memt<Char8> mb;
      mb.setNum(text.length()*2+1);
      Int l=WideCharToMultiByte(code_page, 0, text, text.length(), mb.data(), mb.elms(), null, null);
      if( l>0)
      {
         mb.setNum(l+1);
         mb[l]='\0';
         return mb.data();
      }
   }
#endif
   return S8;
}
Str  WindowsPath    (C Str &path) {return Replace(path, '/', '\\');}
Str     UnixPath    (C Str &path) {return Replace(path, '\\', '/');}
Str8    UnixPathUTF8(C Str &path) {return UTF8(UnixPath(path));}
/******************************************************************************/
Str FixNewLine(C Str &text)
{
#if 0 // slower
   return Replace(Replace(text, '\r', '\0'), "\n", "\r\n");
#else // faster
   Str s; s.reserve(text.length());
   FREPA(text)
   {
      Char c=text()[i]; // () avoids range checks
      if(c!='\r')
      if(c!='\n')s+=c;else s+="\r\n";
   }
   return s;
#endif
}
/******************************************************************************/
CChar8* TextBool(Bool b) {return b ? "true" : "false";}
CChar8* TextInt(Int i, Char8 (&temp)[256], Int digits, Int separate)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !i)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1-1; // 1 for '\0', 1 for '-'
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(UInt u=Abs(i); (digits<0) ? u : digits--; u/=10){if(!--s){*--text=' '; s=separate;} *--text=u%10+'0';} if(i<0)*--text='-';
   }
   return text;
}
CChar8* TextInt(Long i, Char8 (&temp)[256], Int digits, Int separate)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !i)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1-1; // 1 for '\0', 1 for '-'
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(ULong u=Abs(i); (digits<0) ? u : digits--; u/=10){if(!--s){*--text=' '; s=separate;} *--text=u%10+'0';} if(i<0)*--text='-';
   }
   return text;
}
CChar8* TextInt(UInt u, Char8 (&temp)[256], Int digits, Int separate)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !u)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1; // 1 for '\0'
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(; (digits<0) ? u : digits--; u/=10){if(!--s){*--text=' '; s=separate;} *--text=u%10+'0';}
   }
   return text;
}
CChar8* TextInt(ULong u, Char8 (&temp)[256], Int digits, Int separate)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !u)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1; // 1 for '\0'
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(; (digits<0) ? u : digits--; u/=10){if(!--s){*--text=' '; s=separate;} *--text=u%10+'0';}
   }
   return text;
}
CChar8* TextBin(UInt u, Char8 (&temp)[256], Int digits, Int separate, Bool prefix)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !u)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1-2; // 1 for '\0', 2 for prefix
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(; (digits<0) ? u : digits--; u>>=1){if(!--s){*--text=' '; s=separate;} *--text=(u&1)+'0';}
   }
   if(prefix){*--text='b'; *--text='0';}
   return text;
}
CChar8* TextBin(ULong u, Char8 (&temp)[256], Int digits, Int separate, Bool prefix)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !u)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1-2; // 1 for '\0', 2 for prefix
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(; (digits<0) ? u : digits--; u>>=1){if(!--s){*--text=' '; s=separate;} *--text=(u&1)+'0';}
   }
   if(prefix){*--text='b'; *--text='0';}
   return text;
}
CChar8* TextHex(UInt u, Char8 (&temp)[256], Int digits, Int separate, Bool prefix)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !u)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1-2; // 1 for '\0', 2 for prefix
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(; (digits<0) ? u : digits--; u>>=4){if(!--s){*--text=' '; s=separate;} *--text=Digits16[u&0xF];}
   }
   if(prefix){*--text='x'; *--text='0';}
   return text;
}
CChar8* TextHex(ULong u, Char8 (&temp)[256], Int digits, Int separate, Bool prefix)
{
   Char8 *text=temp+255; *text='\0';
   if(digits<0 && !u)*--text='0';else
   {
      Int s=0, max_digits=Elms(temp)-1-2; // 1 for '\0', 2 for prefix
      if(separate>0){s=separate+1; max_digits=max_digits*separate/s;} MIN(digits, max_digits);
      for(; (digits<0) ? u : digits--; u>>=4){if(!--s){*--text=' '; s=separate;} *--text=Digits16[u&0xF];}
   }
   if(prefix){*--text='x'; *--text='0';}
   return text;
}
StrO TextHexMem(CPtr data, Int size, Bool prefix)
{
   StrO s; s.reserve((prefix ? 2 : 0)+size*2); if(prefix)s="0x";
   Byte *src=(Byte*)data; REP(size)
   {
      Byte b=(src ? *src++ : 0);
      s+=Digits16[b>>4];
      s+=Digits16[b&15];
   }
   return s;
}
StrO TextHexMem(File &file, Bool prefix)
{
   StrO s; s.reserve((prefix ? 2 : 0)+file.left()*2); if(prefix)s="0x";
   for(; !file.end(); )
   {
      Byte b=file.getByte();
      s+=Digits16[b>>4];
      s+=Digits16[b&15];
   }
   return s;
}
CChar8* TextReal(Dbl r, Char8 (&temp)[256], Int precision, Int separate)
{
   if(Inf(r))return (r<0) ? "-Inf" : "Inf";
   if(NaN(r))return                  "NaN";

   Char8 *text=temp+128;
   UInt   u;
   Bool   non_zero=false,
          cut=(precision<0); precision=Abs(Mid(precision, -9, 15));
   Dbl    a  =Abs(r)+0.5*Pow(0.1, precision);

   // set fractional part
   if(precision)
   {
      Dbl frac=Frac(a);
      if(cut)
      {
         if(u=TruncU(frac*Pow(10.0, precision)))
         {
           *text++='.'; non_zero=true;
            text +=precision;
            Int written=0; REP(precision)
            {
               Byte c=u%10; u/=10;
               if(written || c){*--text=c+'0'; written++;}else text--;
            }
            text+=written;
         }
      }else
      if(precision<=9)
      {
         u=TruncU(frac*Pow(10.0, precision));
        *text++='.'; non_zero=true;
         Int   s    =((separate>0) ? (precision+separate-1)%separate+2 : 0),
               chars=((separate>0) ? (precision         -1)/separate   : 0) + precision;
         text+=chars; for(Int i=precision; i--; u/=10){if(!--s){*--text=' '; s=separate;} *--text=u%10+'0';}
         text+=chars;
      }else
      {
        *text++='.'; non_zero=true;
         Int s=((separate>0) ? separate+1 : 0);
         for(; precision--; ){if(!--s){*text++=' '; s=separate;} frac=Frac(frac)*10; *text++=TruncU(frac)%10+'0';}
      }
   }
   *text='\0';

   // set integer part
   text=temp+128;
   if(a<=UINT_MAX+0.5)
   {
      u=TruncU(a);
      Int s=((separate>0) ? separate+1 : 0);
      if(!u)*--text='0';else for(non_zero=true; u; u/=10){if(!--s){*--text=' '; s=separate;} *--text=u%10+'0';}
   }else
   {
      a=TruncD(a);
      Int s=((separate>0) ? separate+1 : 0);
      if(!a)*--text='0';else for(non_zero=true, u=0; a && u<128-2; u++) // leave 2 characters for potential separate and '-' sign
      {
         if(!--s){*--text=' '; s=separate; u++;}
         Dbl f=a*0.1; *--text=RoundPos(Frac(f)*10)+'0'; a=TruncD(f);
      }
   }
   if(non_zero && r<0)*--text='-';
   return text;
}
/******************************************************************************/
static Char8* TextHexExp(Char8 *text, Int exp) // !! assumes that 'exp' !=0 and >=-1024 and <=1024, only up to 4 decimal digits !!
{
   *text++='p';
   if(exp<0){*text++='-'; CHS(exp);}
   Char8 temp[4], *t=temp+4;
   for(UInt u=exp; u; u/=10)*--t=u%10+'0';
   for(; t<temp+4; )*text++=*t++;
   return text;
}
CChar8* _TextHex(Flt f, Char8 (&temp)[17]) // 1bit sign, 8bit exp, 23bit frac (1sign + 2prefix + 1lead + 1dot + 6hex_frac + 2exp_symbol_incl_minus + 3decimal_exp + 1nul = 17)
{  // !! Warning: if changing codes then watch out for 'temp' size !!
   UInt frac=((UInt&)f)&((1<<23)-1); Int exp=((((UInt&)f)>>23)&((1<<8)-1))-127;
   if(exp==128)return frac ? "NaN" : (f<0) ? "-Inf" : "Inf";
   Char8 *t=temp;
   if(f<0)*t++='-'; // sign
   *t++='0'; *t++='x'; // prefix
   if(exp==-127)
   {
      *t++='0';
      if(frac)exp=-126;else exp=0; // frac!=0 -> denormalized else real=0 so disable writing exponent
   }else *t++='1';
   if(frac)
   {
      *t++='.';
      frac<<=32-23; // move most significant bit to the end (#22 -> #31)
      do
      {
         UInt dig=(frac>>(32-4));
         *t++=Digits16[dig&15];
         frac<<=4;
      }while(frac);
   }
   if(exp)t=TextHexExp(t, exp);
   *t='\0';
   return temp;
}
CChar8* _TextHex(Dbl d, Char8 (&temp)[25]) // 1bit sign, 11bit exp, 52bit frac (1sign + 2prefix + 1lead + 1dot + 13hex_frac + 2exp_symbol_incl_minus + 4decimal_exp + 1nul = 25)
{  // !! Warning: if changing codes then watch out for 'temp' size !!
   ULong frac=((ULong&)d)&((1ull<<52)-1); Int exp=((((ULong&)d)>>52)&((1<<11)-1))-1023;
   if(exp==1024)return frac ? "NaN" : (d<0) ? "-Inf" : "Inf";
   Char8 *t=temp;
   if(d<0)*t++='-'; // sign
   *t++='0'; *t++='x'; // prefix
   if(exp==-1023)
   {
      *t++='0';
      if(frac)exp=-1022;else exp=0; // frac!=0 -> denormalized else real=0 so disable writing exponent
   }else *t++='1';
   if(frac)
   {
      *t++='.';
      frac<<=64-52; // move most significant bit to the end (#51 -> #63)
      do
      {
         UInt dig=(frac>>(64-4));
         *t++=Digits16[dig&15];
         frac<<=4;
      }while(frac);
   }
   if(exp)t=TextHexExp(t, exp);
   *t='\0';
   return temp;
}
/******************************************************************************/
StrO TextInt (Int   i, Int digits   , Int separate             ) {Char8 temp[256]; return TextInt (i, temp, digits   , separate        );}
StrO TextInt (Long  i, Int digits   , Int separate             ) {Char8 temp[256]; return TextInt (i, temp, digits   , separate        );}
StrO TextInt (UInt  u, Int digits   , Int separate             ) {Char8 temp[256]; return TextInt (u, temp, digits   , separate        );}
StrO TextInt (ULong u, Int digits   , Int separate             ) {Char8 temp[256]; return TextInt (u, temp, digits   , separate        );}
StrO TextBin (UInt  u, Int digits   , Int separate, Bool prefix) {Char8 temp[256]; return TextBin (u, temp, digits   , separate, prefix);}
StrO TextBin (ULong u, Int digits   , Int separate, Bool prefix) {Char8 temp[256]; return TextBin (u, temp, digits   , separate, prefix);}
StrO TextHex (UInt  u, Int digits   , Int separate, Bool prefix) {Char8 temp[256]; return TextHex (u, temp, digits   , separate, prefix);}
StrO TextHex (ULong u, Int digits   , Int separate, Bool prefix) {Char8 temp[256]; return TextHex (u, temp, digits   , separate, prefix);}
StrO TextHex (Flt   r                                          ) {return _TextHex(r);}
StrO TextHex (Dbl   r                                          ) {return _TextHex(r);}
StrO TextReal(Dbl   r, Int precision, Int separate             ) {Char8 temp[256]; return TextReal(r, temp, precision, separate        );}
/******************************************************************************/
#if 0
CChar * _TextPacked(  Flt   f, Char  (&temp)[ 7*1]=ConstCast(TempChar< 7*1>()).c);
CChar * _TextPacked(  Dbl   d, Char  (&temp)[12*1]=ConstCast(TempChar<12*1>()).c);
CChar * _TextPacked(C Vec2 &v, Char  (&temp)[ 7*2]=ConstCast(TempChar< 7*2>()).c);
CChar * _TextPacked(C Vec  &v, Char  (&temp)[ 7*3]=ConstCast(TempChar< 7*3>()).c);
CChar * _TextPacked(C Vec4 &v, Char  (&temp)[ 7*4]=ConstCast(TempChar< 7*4>()).c);

CChar* _TextPacked(Flt f, Char (&temp)[7]) // 1 for CharReal, 5 for Flt, 1 for nul
{
   temp[0]=CharReal; EncodeText(&f, SIZE(f), temp+1, Elms(temp)-1);
   return temp;
}
CChar* _TextPacked(Dbl d, Char (&temp)[12]) // 1 for CharReal, 10 for Dbl, 1 for nul
{
   temp[0]=CharReal; EncodeText(&d, SIZE(d), temp+1, Elms(temp)-1);
   return temp;
}

CChar* _TextPacked(C Vec2 &value, Char (&temp)[7*2])
{
  _TextPacked(value.x, (Char(&)[7])temp   ); temp[6]=',';
  _TextPacked(value.y, (Char(&)[7])temp[7]);
   return temp;
}
CChar* _TextPacked(C Vec &value, Char (&temp)[7*3])
{
  _TextPacked(value.x, (Char(&)[7])temp    ); temp[ 6]=',';
  _TextPacked(value.y, (Char(&)[7])temp[ 7]); temp[13]=',';
  _TextPacked(value.z, (Char(&)[7])temp[14]);
   return temp;
}
CChar* _TextPacked(C Vec4 &value, Char (&temp)[7*4])
{
  _TextPacked(value.x, (Char(&)[7])temp    ); temp[ 6]=',';
  _TextPacked(value.y, (Char(&)[7])temp[ 7]); temp[13]=',';
  _TextPacked(value.z, (Char(&)[7])temp[14]); temp[20]=',';
  _TextPacked(value.w, (Char(&)[7])temp[21]);
   return temp;
}

Str TextPacked(Flt r);
Str TextPacked(Dbl r);
Str TextPacked(Flt r) {return _TextPacked(r);}
Str TextPacked(Dbl r) {return _TextPacked(r);}
#endif
/******************************************************************************/
Bool TextHexMem(C Str &t, Ptr data, Int size) // this method sets all correct digits, incorrect sets to zero, returns false on fail however preserves those that were correct
{
   Bool ok=(size*2==t.length());
   if(Byte *dest=(Byte*)data)
   {
      Int i=0;
      for(Int src=0, process=Min(size, (t.length()+1)/2); i<process; ) // "(length+1)/2" because we want to process all existing characters so that "5?" gets converted to 0x50 even though '?' is invalid
      {
         Int hi=CharInt(t[src++]); if(!InRange(hi, 16)){              ok=false; break;} hi<<=4;
         Int lo=CharInt(t[src++]); if(!InRange(lo, 16)){dest[i++]=hi; ok=false; break;} // set 'dest' from 'hi' only and increase 'i' to mark it as processed
         dest[i++]=(lo|hi);
      }
      for(; i<size; )dest[i++]=0; // clear unprocessed with zeros
   }
   return ok;
}
/******************************************************************************/
enum TYPE
{
   T_NONE ,
#if !X64
   T_UINT ,
#endif
   T_ULONG,
   T_REAL ,
};
CChar8* TextValue(CChar8 *t, CalcValue &value, Bool allow_real)
{
   value.type=CVAL_NONE;
   if(t)
   {
      Dbl  r, frac_mul;
      UInt mode=10;
      TYPE type=T_NONE;
      Bool frac=false, sign=false;

      for(; t[0]==' '; t++); // skip spaces
      if(   t[0]=='-'){sign=true; t++;}
      if(   t[0]=='0')
      {
         if(t[1]=='b' || t[1]=='B'){t+=2; mode= 2;}else
         if(t[1]=='x' || t[1]=='X'){t+=2; mode=16;}
      }

   #if X64
      ULong ul=0, maxl=ULONG_MAX/mode;
   #else
      UInt  u=0, max=UINT_MAX/mode;
      ULong ul, maxl;
   #endif

      for(;;)
      {
         Int c=*t, i=CharInt(c);
         if(InRange(i, mode))
         {
            t++;
            switch(type)
            {
            #if !X64
               case T_NONE: type=T_UINT; // !! no break on purpose !!
               case T_UINT:
               {
                  UInt next=u*mode+i;
                  if(u>max && (next-i)/mode!=u){type=T_ULONG; ul=u; maxl=ULONG_MAX/mode; goto as_big;} // if we're close to limit then check for overflow, don't test "if(next<ul)" because in some cases this doesn't work
                  u=next;
               }break;
            #endif

            #if X64
               case T_NONE : type=T_ULONG; // !! no break on purpose !!
            #else
               as_big      :
            #endif
               case T_ULONG:
               {
                  ULong next=ul*mode+i;
                  if(ul>maxl && (next-i)/mode!=ul){type=T_REAL; r=ul; goto as_real;} // if we're close to limit then check for overflow, don't test "if(next<ul)" because in some cases this doesn't work
                  ul=next;
               }break;

               case T_REAL:
               {
                  if(!frac){as_real: r*=mode; r+=i;}else
                                     r+=(frac_mul/=mode)*i;
               }break;
            }
         }else
         {
            if(allow_real)switch(c)
            {
               case '.':
               {
                  if(frac)break; frac=true; frac_mul=1; t++;
                  if(type!=T_REAL) // convert to Real
                  {
                  #if X64
                     r=ul;
                  #else
                     if(type==T_ULONG)r=ul;else r=u; // T_NONE or T_UINT
                  #endif
                     type=T_REAL;
                  }
               }goto next;

               case 'e':
               case 'E':
               case 'p':
               case 'P':
               {
                  t++;
                  if(type!=T_REAL) // convert to Real
                  {
                  #if X64
                     r=ul;
                  #else
                     if(type==T_ULONG)r=ul;else r=u; // T_NONE or T_UINT
                  #endif
                     type=T_REAL;
                  }

                  Int  exp =0;
                  Bool sign=false;
                  if(t[0]=='-'){sign=true; t++;}else
                  if(t[0]=='+'){           t++;}
                  for(;;)
                  {
                     Int c=*t-'0'; if(!InRange(c, 10))break; // both 'e' and 'p' operate on decimal exponents
                     t++; exp*=10; exp+=c;
                  }
                  if(exp)
                  {
                     if(sign)CHS(exp);
                     Dbl base=((c=='e' || c=='E') ? 10 : 2), pow=Pow(base, exp);
                     r*=pow;
                  }
               }break;
            }
            break; // unexpected character
         }
      next:;
      }
      switch(type)
      {
      #if !X64
         case T_UINT:
         {
            if(SIZE(value.i)<SIZE(ULong) && u>=0x80000000+sign){ul=u; goto set_long;} // Int would be invalid, u>=(sign ? 0x80000001 : 0x80000000)
            value.type=CVAL_INT; value.i=u; if(sign)CHS(value.i);
         }break;
      #endif

         case T_ULONG:
         {
            if(allow_real && ul>=0x8000000000000000+sign){r=ul; goto set_real;} // Long would be invalid, u>=(sign ? 0x8000000000000001 : 0x8000000000000000), however check this only if we 'allow_real' because the value could still be correct if casted to ULong, however as real it would lose precision, some functions rely on this behavior, for example 'UID.fromCString'
      #if !X64
         set_long:
      #endif
            value.type=CVAL_INT; value.i=ul; if(sign)CHS(value.i);
         }break;

         case T_REAL:
         {
         set_real:
            value.type=CVAL_REAL; value.r=r; if(sign)CHS(value.r);
         }break;
      }
   }
   return t;
}
/******************************************************************************/
CChar* TextValue(CChar *t, CalcValue &value, Bool allow_real)
{
   value.type=CVAL_NONE;
   if(t)
   {
      Dbl  r, frac_mul;
      UInt mode=10;
      TYPE type=T_NONE;
      Bool frac=false, sign=false;

      for(; t[0]==' '; t++); // skip spaces
      if(   t[0]=='-'){sign=true; t++;}
      if(   t[0]=='0')
      {
         if(t[1]=='b' || t[1]=='B'){t+=2; mode= 2;}else
         if(t[1]=='x' || t[1]=='X'){t+=2; mode=16;}
      }

   #if X64
      ULong ul=0, maxl=ULONG_MAX/mode;
   #else
      UInt  u=0, max=UINT_MAX/mode;
      ULong ul, maxl;
   #endif

      for(;;)
      {
         Int c=*t, i=CharInt(c);
         if(InRange(i, mode))
         {
            t++;
            switch(type)
            {
            #if !X64
               case T_NONE: type=T_UINT; // !! no break on purpose !!
               case T_UINT:
               {
                  UInt next=u*mode+i;
                  if(u>max && (next-i)/mode!=u){type=T_ULONG; ul=u; maxl=ULONG_MAX/mode; goto as_big;} // if we're close to limit then check for overflow, don't test "if(next<ul)" because in some cases this doesn't work
                  u=next;
               }break;
            #endif

            #if X64
               case T_NONE : type=T_ULONG; // !! no break on purpose !!
            #else
               as_big      :
            #endif
               case T_ULONG:
               {
                  ULong next=ul*mode+i;
                  if(ul>maxl && (next-i)/mode!=ul){type=T_REAL; r=ul; goto as_real;} // if we're close to limit then check for overflow, don't test "if(next<ul)" because in some cases this doesn't work
                  ul=next;
               }break;

               case T_REAL:
               {
                  if(!frac){as_real: r*=mode; r+=i;}else
                                     r+=(frac_mul/=mode)*i;
               }break;
            }
         }else
         {
            if(allow_real)switch(c)
            {
               case '.':
               {
                  if(frac)break; frac=true; frac_mul=1; t++;
                  if(type!=T_REAL) // convert to Real
                  {
                  #if X64
                     r=ul;
                  #else
                     if(type==T_ULONG)r=ul;else r=u; // T_NONE or T_UINT
                  #endif
                     type=T_REAL;
                  }
               }goto next;

               case 'e':
               case 'E':
               case 'p':
               case 'P':
               {
                  t++;
                  if(type!=T_REAL) // convert to Real
                  {
                  #if X64
                     r=ul;
                  #else
                     if(type==T_ULONG)r=ul;else r=u; // T_NONE or T_UINT
                  #endif
                     type=T_REAL;
                  }

                  Int  exp =0;
                  Bool sign=false;
                  if(t[0]=='-'){sign=true; t++;}else
                  if(t[0]=='+'){           t++;}
                  for(;;)
                  {
                     Int c=*t-'0'; if(!InRange(c, 10))break; // both 'e' and 'p' operate on decimal exponents
                     t++; exp*=10; exp+=c;
                  }
                  if(exp)
                  {
                     if(sign)CHS(exp);
                     Dbl base=((c=='e' || c=='E') ? 10 : 2), pow=Pow(base, exp);
                     r*=pow;
                  }
               }break;
            }
            break; // unexpected character
         }
      next:;
      }
      switch(type)
      {
      #if !X64
         case T_UINT:
         {
            if(SIZE(value.i)<SIZE(ULong) && u>=0x80000000+sign){ul=u; goto set_long;} // Int would be invalid, u>=(sign ? 0x80000001 : 0x80000000)
            value.type=CVAL_INT; value.i=u; if(sign)CHS(value.i);
         }break;
      #endif

         case T_ULONG:
         {
            if(allow_real && ul>=0x8000000000000000+sign){r=ul; goto set_real;} // Long would be invalid, u>=(sign ? 0x8000000000000001 : 0x8000000000000000), however check this only if we 'allow_real' because the value could still be correct if casted to ULong, however as real it would lose precision, some functions rely on this behavior, for example 'UID.fromCString'
      #if !X64
         set_long:
      #endif
            value.type=CVAL_INT; value.i=ul; if(sign)CHS(value.i);
         }break;

         case T_REAL:
         {
         set_real:
            value.type=CVAL_REAL; value.r=r; if(sign)CHS(value.r);
         }break;
      }
   }
   return t;
}
/******************************************************************************/
Long TextLong(CChar8 *t)
{
   if(t)
   {
      UInt  mode=10;
      ULong u=0;
      Dbl   r, frac_mul;
      Bool  frac=false,
            sign=false;

      for(; t[0]==' '; t++); // skip spaces
      if(   t[0]=='-'){sign=true; t++;}
      if(   t[0]=='0')
      {
         if(t[1]=='b' || t[1]=='B'){t+=2; mode= 2;}else
         if(t[1]=='x' || t[1]=='X'){t+=2; mode=16;}
      }
      for(;;)
      {
         Int c=*t, i=CharInt(c);
         if(InRange(i, mode))
         {
            t++;
            if(!frac){u*=mode; u+=i;}else
                      r+=(frac_mul/=mode)*i;
         }else
         if(c=='.')
         {
            if(frac)break; frac=true; r=0; frac_mul=1; t++;
         }else break;
      }
      if(frac && r>=0.5)u++; // use rounding
      return sign ? -Long(u) : u;
   }
   return 0;
}
Long TextLong(CChar *t)
{
   if(t)
   {
      UInt  mode=10;
      ULong u=0;
      Dbl   r, frac_mul;
      Bool  frac=false,
            sign=false;

      for(; t[0]==' '; t++); // skip spaces
      if(   t[0]=='-'){sign=true; t++;}
      if(   t[0]=='0')
      {
         if(t[1]=='b' || t[1]=='B'){t+=2; mode= 2;}else
         if(t[1]=='x' || t[1]=='X'){t+=2; mode=16;}
      }
      for(;;)
      {
         Int c=*t, i=CharInt(c);
         if(InRange(i, mode))
         {
            t++;
            if(!frac){u*=mode; u+=i;}else
                      r+=(frac_mul/=mode)*i;
         }else
         if(c=='.')
         {
            if(frac)break; frac=true; r=0; frac_mul=1; t++;
         }else break;
      }
      if(frac && r>=0.5)u++; // use rounding
      return sign ? -Long(u) : u;
   }
   return 0;
}
ULong TextULong(CChar  *t) {return TextLong(t);}
ULong TextULong(CChar8 *t) {return TextLong(t);}
/******************************************************************************/
Bool   TextBool  (CChar8 *t) {return TextInt (t)!=0 || Equal(t, "true");}
Bool   TextBool  (CChar  *t) {return TextInt (t)!=0 || Equal(t, "true");}
Bool   TextBool1 (CChar8 *t) {return TextBool(t) || !Is(t);}
Bool   TextBool1 (CChar  *t) {return TextBool(t) || !Is(t);}
Int    TextInt   (CChar8 *t) {CalcValue x         ; TextValue(t, x); return x.asInt ();}
Int    TextInt   (CChar  *t) {CalcValue x         ; TextValue(t, x); return x.asInt ();}
UInt   TextUInt  (CChar8 *t) {CalcValue x         ; TextValue(t, x); return x.asUInt();}
UInt   TextUInt  (CChar  *t) {CalcValue x         ; TextValue(t, x); return x.asUInt();}
Flt    TextFlt   (CChar8 *t) {CalcValue x         ; TextValue(t, x); return x.asFlt ();}
Flt    TextFlt   (CChar  *t) {CalcValue x         ; TextValue(t, x); return x.asFlt ();}
Dbl    TextDbl   (CChar8 *t) {CalcValue x         ; TextValue(t, x); return x.asDbl ();}
Dbl    TextDbl   (CChar  *t) {CalcValue x         ; TextValue(t, x); return x.asDbl ();}
Vec2   TextVec2  (CChar8 *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return Vec2  (x.asFlt(), y.asFlt()                      );}
Vec2   TextVec2  (CChar  *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return Vec2  (x.asFlt(), y.asFlt()                      );}
VecD2  TextVecD2 (CChar8 *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecD2 (x.asDbl(), y.asDbl()                      );}
VecD2  TextVecD2 (CChar  *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecD2 (x.asDbl(), y.asDbl()                      );}
VecI2  TextVecI2 (CChar8 *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecI2 (x.asInt(), y.asInt()                      );}
VecI2  TextVecI2 (CChar  *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecI2 (x.asInt(), y.asInt()                      );}
VecB2  TextVecB2 (CChar8 *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecB2 (x.asInt(), y.asInt()                      );}
VecB2  TextVecB2 (CChar  *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecB2 (x.asInt(), y.asInt()                      );}
VecSB2 TextVecSB2(CChar8 *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecSB2(x.asInt(), y.asInt()                      );}
VecSB2 TextVecSB2(CChar  *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecSB2(x.asInt(), y.asInt()                      );}
VecUS2 TextVecUS2(CChar8 *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecUS2(x.asInt(), y.asInt()                      );}
VecUS2 TextVecUS2(CChar  *t) {CalcValue x, y      ; TextValue(                                        _SkipChar(TextValue(t, x)), y)          ; return VecUS2(x.asInt(), y.asInt()                      );}
Vec    TextVec   (CChar8 *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return Vec   (x.asFlt(), y.asFlt(), z.asFlt()           );}
Vec    TextVec   (CChar  *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return Vec   (x.asFlt(), y.asFlt(), z.asFlt()           );}
VecD   TextVecD  (CChar8 *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecD  (x.asDbl(), y.asDbl(), z.asDbl()           );}
VecD   TextVecD  (CChar  *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecD  (x.asDbl(), y.asDbl(), z.asDbl()           );}
VecI   TextVecI  (CChar8 *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecI  (x.asInt(), y.asInt(), z.asInt()           );}
VecI   TextVecI  (CChar  *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecI  (x.asInt(), y.asInt(), z.asInt()           );}
VecB   TextVecB  (CChar8 *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecB  (x.asInt(), y.asInt(), z.asInt()           );}
VecB   TextVecB  (CChar  *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecB  (x.asInt(), y.asInt(), z.asInt()           );}
VecSB  TextVecSB (CChar8 *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecSB (x.asInt(), y.asInt(), z.asInt()           );}
VecSB  TextVecSB (CChar  *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecSB (x.asInt(), y.asInt(), z.asInt()           );}
VecUS  TextVecUS (CChar8 *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecUS (x.asInt(), y.asInt(), z.asInt()           );}
VecUS  TextVecUS (CChar  *t) {CalcValue x, y, z   ; TextValue(_SkipChar(TextValue(                    _SkipChar(TextValue(t, x)), y)), z)     ; return VecUS (x.asInt(), y.asInt(), z.asInt()           );}
Vec4   TextVec4  (CChar8 *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return Vec4  (x.asFlt(), y.asFlt(), z.asFlt(), w.asFlt());}
Vec4   TextVec4  (CChar  *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return Vec4  (x.asFlt(), y.asFlt(), z.asFlt(), w.asFlt());}
VecD4  TextVecD4 (CChar8 *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecD4 (x.asDbl(), y.asDbl(), z.asDbl(), w.asDbl());}
VecD4  TextVecD4 (CChar  *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecD4 (x.asDbl(), y.asDbl(), z.asDbl(), w.asDbl());}
VecI4  TextVecI4 (CChar8 *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecI4 (x.asInt(), y.asInt(), z.asInt(), w.asInt());}
VecI4  TextVecI4 (CChar  *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecI4 (x.asInt(), y.asInt(), z.asInt(), w.asInt());}
VecB4  TextVecB4 (CChar8 *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecB4 (x.asInt(), y.asInt(), z.asInt(), w.asInt());}
VecB4  TextVecB4 (CChar  *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecB4 (x.asInt(), y.asInt(), z.asInt(), w.asInt());}
VecSB4 TextVecSB4(CChar8 *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecSB4(x.asInt(), y.asInt(), z.asInt(), w.asInt());}
VecSB4 TextVecSB4(CChar  *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return VecSB4(x.asInt(), y.asInt(), z.asInt(), w.asInt());}
Color  TextColor (CChar8 *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return Color (x.asInt(), y.asInt(), z.asInt(), w.asInt());}
Color  TextColor (CChar  *t) {CalcValue x, y, z, w; TextValue(_SkipChar(TextValue(_SkipChar(TextValue(_SkipChar(TextValue(t, x)), y)), z)), w); return Color (x.asInt(), y.asInt(), z.asInt(), w.asInt());}
UID    TextUID   (CChar8 *t) {UID id; id.fromText(t); return id;}
UID    TextUID   (CChar  *t) {UID id; id.fromText(t); return id;}
/******************************************************************************/
// STRING
/******************************************************************************/
Str8::Str8(                                ) {   _length=         0 ;                                                                                 }
Str ::Str (                                ) {   _length=         0 ;                                                                                 }
Str8::Str8(Char8   c                       ) {if(_length=  (c!='\0')){_d.setNum(length()+1                  ); _d[0]=          c ; _d[1]='\0';       }}
Str ::Str (Char    c                       ) {if(_length=  (c!='\0')){_d.setNum(length()+1                  ); _d[0]=          c ; _d[1]='\0';       }}
Str8::Str8(Char    c                       ) {if(_length=  (c!='\0')){_d.setNum(length()+1                  ); _d[0]=Char16To8(c); _d[1]='\0';       }}
Str ::Str (Char8   c                       ) {if(_length=  (c!='\0')){_d.setNum(length()+1                  ); _d[0]=Char8To16(c); _d[1]='\0';       }}
Str8::Str8(CChar8 *t                       ) {if(_length=  Length(t)){_d.setNum(length()+1                  ); CopyFastN(_d.data(),   t,  _d.elms());}}
Str ::Str (CChar  *t                       ) {if(_length=  Length(t)){_d.setNum(length()+1                  ); CopyFastN(_d.data(),   t,  _d.elms());}}
Str8::Str8(CChar  *t                       ) {if(_length=  Length(t)){_d.setNum(length()+1                  ); Set      (_d.data(),   t,  _d.elms());}}
Str8::Str8(C wchar_t *t                    ) {if(_length=  Length(t)){_d.setNum(length()+1                  );_Set      (_d.data(),   t,  _d.elms());}}
Str ::Str (C wchar_t *t                    ) {if(_length=  Length(t)){_d.setNum(length()+1                  );_Set      (_d.data(),   t,  _d.elms());}}
Str ::Str (CChar8 *t                       ) {if(_length=  Length(t)){_d.setNum(length()+1                  ); Set      (_d.data(),   t,  _d.elms());}}
Str8::Str8(C Str8 &s                       ) {if(_length=s.length( )){_d.setNum(length()+1                  ); CopyFastN(_d.data(), s(),  _d.elms());}}
Str ::Str (C Str  &s                       ) {if(_length=s.length( )){_d.setNum(length()+1                  ); CopyFastN(_d.data(), s(),  _d.elms());}}
Str8::Str8(C Str8 &s, Int additional_length) {   _length=s.length( ); _d.setNum(length()+1+additional_length); CopyN    (_d.data(), s(), length()+1); }
Str ::Str (C Str  &s, Int additional_length) {   _length=s.length( ); _d.setNum(length()+1+additional_length); CopyN    (_d.data(), s(), length()+1); }
Str ::Str (C Str8 &s, Int additional_length) {   _length=s.length( ); _d.setNum(length()+1+additional_length); I(); FREP (length())_d[i]=Char8To16Fast(s ()[i]); _d[length()]='\0'; } // don't use 'Set' to allow copying '\0' chars in the middle, use () to avoid range checks
Str8::Str8(C Str  &s                       ) {if(_length=s.length( )){_d.setNum(length()+1                  ); I(); FREPA(    _d  )_d[i]=Char16To8Fast(s._d[i]);                   }} // don't use 'Set' to allow copying '\0' chars in the middle
Str ::Str (C Str8 &s                       ) {if(_length=s.length( )){_d.setNum(length()+1                  ); I(); FREPA(    _d  )_d[i]=Char8To16Fast(s._d[i]);                   }} // don't use 'Set' to allow copying '\0' chars in the middle
Str8::Str8(C BStr &s                       ) {if(_length=s.length( )){_d.setNum(length()+1                  ); I(); FREP (length())_d[i]=Char16To8Fast(s ()[i]); _d[length()]='\0';}} // don't use 'Set' to allow copying '\0' chars in the middle, borrowed string may not be nul-terminated, use () to avoid range checks
Str ::Str (C BStr &s                       ) {if(_length=s.length( )){_d.setNum(length()+1                  ); CopyFastN(_d.data(), s(), length()  );            _d[length()]='\0';}} // don't use 'Set' to allow copying '\0' chars in the middle, borrowed string may not be nul-terminated
Str8::Str8(Bool    b                       ) {   _length=         1 ; _d.setNum(         2                  ); _d[0]=(b ? '1' : '0'); _d[1]='\0';}
Str ::Str (Bool    b                       ) {   _length=         1 ; _d.setNum(         2                  ); _d[0]=(b ? '1' : '0'); _d[1]='\0';}
Str8::Str8(SByte   i                       ) : Str8(TextInt(    Int(i), ConstCast(TempChar8<256>()).c)) {}
Str ::Str (SByte   i                       ) : Str (TextInt(    Int(i), ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(Int     i                       ) : Str8(TextInt(        i , ConstCast(TempChar8<256>()).c)) {}
Str ::Str (Int     i                       ) : Str (TextInt(        i , ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(Long    i                       ) : Str8(TextInt(        i , ConstCast(TempChar8<256>()).c)) {}
Str ::Str (Long    i                       ) : Str (TextInt(        i , ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(Byte    u                       ) : Str8(TextInt(   UInt(u), ConstCast(TempChar8<256>()).c)) {}
Str ::Str (Byte    u                       ) : Str (TextInt(   UInt(u), ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(UInt    u                       ) : Str8(TextInt(        u , ConstCast(TempChar8<256>()).c)) {}
Str ::Str (UInt    u                       ) : Str (TextInt(        u , ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(ULong   u                       ) : Str8(TextInt(        u , ConstCast(TempChar8<256>()).c)) {}
Str ::Str (ULong   u                       ) : Str (TextInt(        u , ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(Flt     f                       ) : Str8(TextFlt(        f , ConstCast(TempChar8<256>()).c)) {}
Str ::Str (Flt     f                       ) : Str (TextFlt(        f , ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(Dbl     d                       ) : Str8(TextDbl(        d , ConstCast(TempChar8<256>()).c)) {}
Str ::Str (Dbl     d                       ) : Str (TextDbl(        d , ConstCast(TempChar8<256>()).c)) {}
Str8::Str8(CPtr    p                       ) : Str8(TextHex(UIntPtr(p), ConstCast(TempChar8<256>()).c, SIZE(p)*2, 0, true)) {}
Str ::Str (CPtr    p                       ) : Str (TextHex(UIntPtr(p), ConstCast(TempChar8<256>()).c, SIZE(p)*2, 0, true)) {}

Str8::Str8(C Vec2   &v) : Str8() {T=v;}
Str ::Str (C Vec2   &v) : Str () {T=v;}
Str8::Str8(C VecD2  &v) : Str8() {T=v;}
Str ::Str (C VecD2  &v) : Str () {T=v;}
Str8::Str8(C VecI2  &v) : Str8() {T=v;}
Str ::Str (C VecI2  &v) : Str () {T=v;}
Str8::Str8(C VecB2  &v) : Str8() {T=v;}
Str ::Str (C VecB2  &v) : Str () {T=v;}
Str8::Str8(C VecSB2 &v) : Str8() {T=v;}
Str ::Str (C VecSB2 &v) : Str () {T=v;}
Str8::Str8(C VecUS2 &v) : Str8() {T=v;}
Str ::Str (C VecUS2 &v) : Str () {T=v;}
Str8::Str8(C Vec    &v) : Str8() {T=v;}
Str ::Str (C Vec    &v) : Str () {T=v;}
Str8::Str8(C VecD   &v) : Str8() {T=v;}
Str ::Str (C VecD   &v) : Str () {T=v;}
Str8::Str8(C VecI   &v) : Str8() {T=v;}
Str ::Str (C VecI   &v) : Str () {T=v;}
Str8::Str8(C VecB   &v) : Str8() {T=v;}
Str ::Str (C VecB   &v) : Str () {T=v;}
Str8::Str8(C VecSB  &v) : Str8() {T=v;}
Str ::Str (C VecSB  &v) : Str () {T=v;}
Str8::Str8(C VecUS  &v) : Str8() {T=v;}
Str ::Str (C VecUS  &v) : Str () {T=v;}
Str8::Str8(C Vec4   &v) : Str8() {T=v;}
Str ::Str (C Vec4   &v) : Str () {T=v;}
Str8::Str8(C VecD4  &v) : Str8() {T=v;}
Str ::Str (C VecD4  &v) : Str () {T=v;}
Str8::Str8(C VecI4  &v) : Str8() {T=v;}
Str ::Str (C VecI4  &v) : Str () {T=v;}
Str8::Str8(C VecB4  &v) : Str8() {T=v;}
Str ::Str (C VecB4  &v) : Str () {T=v;}
Str8::Str8(C VecSB4 &v) : Str8() {T=v;}
Str ::Str (C VecSB4 &v) : Str () {T=v;}
/******************************************************************************/
Char8 Str8::operator[](Int i)C {return InRange(i, T) ? _d[i] : '\0';}
Char  Str ::operator[](Int i)C {return InRange(i, T) ? _d[i] : '\0';}

Str8& Str8::del() {_d.del(); _length=0; return T;}
Str & Str ::del() {_d.del(); _length=0; return T;}

Str8& Str8::clear() {if(_d.elms())_d[0]='\0'; _length=0; return T;}
Str & Str ::clear() {if(_d.elms())_d[0]='\0'; _length=0; return T;}

Str8& Str8::space(Int num) {if(length() && last()!='\n' && last()!=' ')REP(num)T+=' '; return T;}
Str & Str ::space(Int num) {if(length() && last()!='\n' && last()!=' ')REP(num)T+=' '; return T;}

Str8& Str8::line(Int num) {if(length() && last()!='\n')REP(num)T+='\n'; return T;}
Str & Str ::line(Int num) {if(length() && last()!='\n')REP(num)T+='\n'; return T;}

Str8& Str8::insert(Int i, Char8 c)
{
   if(c)
   {
      Clamp(i, 0, length());
      reserve(length()+1);
      REPD(left, length()-i)_d[i+1+left ]=_d[i+left]; // copy text after 'i'
                            _d[i        ]= c        ; // copy 'c'  into  'i'
                            _d[++_length]='\0';
   }
   return T;
}
Str& Str::insert(Int i, Char c)
{
   if(c)
   {
      Clamp(i, 0, length());
      reserve(length()+1);
      REPD(left, length()-i)_d[i+1+left ]=_d[i+left]; // copy text after 'i'
                            _d[i        ]= c        ; // copy 'c'  into  'i'
                            _d[++_length]='\0';
   }
   return T;
}

Str8& Str8::insert(Int i, C Str8 &text)
{
   if(text.length())
   {
      Clamp(i, 0, length());
      reserve(length()+text.length());
      REPD(left,      length()-i)_d[i+text.length()+left  ]=  _d[i+left]; // copy  text  after 'i'
      REPD(left, text.length()  )_d[i+              left  ]=text[  left]; // copy 'text' into  'i'
                                 _d[_length+=text.length()]='\0';
   }
   return T;
}
Str& Str::insert(Int i, C Str &text)
{
   if(text.length())
   {
      Clamp(i, 0, length());
      reserve(length()+text.length());
      REPD(left,      length()-i)_d[i+text.length()+left  ]=  _d[i+left]; // copy  text  after 'i'
      REPD(left, text.length()  )_d[i+              left  ]=text[  left]; // copy 'text' into  'i'
                                 _d[_length+=text.length()]='\0';
   }
   return T;
}

Str8& Str8::remove(Int i, Int num)
{
   if(i    <       0){num+=i; i=0;}
   if(i+num>length())num=length()-i;
   if(  num>=      1)
   {
      REPD(left, length()-(i+num)+1){_d[i]=_d[i+num]; i++;}
     _length-=num;
   }
   return T;
}
Str& Str::remove(Int i, Int num)
{
   if(i    <       0){num+=i; i=0;}
   if(i+num>length())num=length()-i;
   if(  num>=      1)
   {
      REPD(left, length()-(i+num)+1){_d[i]=_d[i+num]; i++;}
     _length-=num;
   }
   return T;
}

Str8& Str8::removeLast(Int num) {if(length() && num>0)_d[MAX(_length-=num, 0)]=0; return T;}
Str & Str ::removeLast(Int num) {if(length() && num>0)_d[MAX(_length-=num, 0)]=0; return T;}

Str8& Str8::clip(Int length)
{
   if(length<0         )length=0;
   if(length<T.length())_d[T._length=length]=0;
   return T;
}
Str& Str::clip(Int length)
{
   if(length<0         )length=0;
   if(length<T.length())_d[T._length=length]=0;
   return T;
}

Str8& Str8::trim(Int pos, Int length) {clip(pos+length).remove(0, pos); return T;}
Str & Str ::trim(Int pos, Int length) {clip(pos+length).remove(0, pos); return T;}
/******************************************************************************/
Str8& Str8::reserve(Int length)
{
   if(length>0)
   {
      Int size=length+1; // 1 extra for '\0'
      if( size>_d.elms()) // increase only
      {
         Bool empty=(_d.elms()<=0);
         if(!T.length())_d.clear(); // if string is empty then clear so 'setNum' doesn't need to copy old contents
        _d.setNum(size);
         if(empty)_d[0]='\0'; // we need to initialize the first character as zero if it was empty before
      }
   }
   return T;
}
Str& Str::reserve(Int length)
{
   if(length>0)
   {
      Int size=length+1; // 1 extra for '\0'
      if( size>_d.elms()) // increase only
      {
         Bool empty=(_d.elms()<=0);
         if(!T.length())_d.clear(); // if string is empty then clear so 'setNum' doesn't need to copy old contents
        _d.setNum(size);
         if(empty)_d[0]='\0'; // we need to initialize the first character as zero if it was empty before
      }
   }
   return T;
}
/******************************************************************************/
Str8& Str8::reverse() {ReverseOrder(_d.data(), length()); return T;}
Str & Str ::reverse() {ReverseOrder(_d.data(), length()); return T;}

Str8& Str8::replace(Char8 src, Char8 dest) {if(src && src!=dest)REPA(T)if(_d[i]==src)if(dest)_d[i]=dest;else remove(i); return T;}
Str & Str ::replace(Char  src, Char  dest) {if(src && src!=dest)REPA(T)if(_d[i]==src)if(dest)_d[i]=dest;else remove(i); return T;}

Str8& Str8::setChar(Int i, Char8 c)
{
   if(c)
   {
      if(InRange(i, T))_d[i]=c;else
      if(i==length()  ) T  +=c;
   }else clip(i);
   return T;
}
Str& Str::setChar(Int i, Char c)
{
   if(c)
   {
      if(InRange(i, T))_d[i]=c;else
      if(i==length()  ) T  +=c;
   }else clip(i);
   return T;
}

Str8& Str8::tailSlash(Bool on)
{
   if(on)
   {
      if(!IsSlash(last()) && is())T+='\\';
   }else
   {
      if(IsSlash(last()) && length()>1)removeLast();
   }
   return T;
}
Str& Str::tailSlash(Bool on)
{
   if(on)
   {
      if(!IsSlash(last()) && is())T+='\\';
   }else
   {
      if(IsSlash(last()) && length()>1)removeLast();
   }
   return T;
}

Str8& Str8::removeOuterWhiteChars()
{
   Int n=0;  REPA(T)if(WhiteChar(T[i]))n++;else break; removeLast(n);
       n=0; FREPA(T)if(WhiteChar(T[i]))n++;else break; remove    (0, n);
   return T;
}
Str& Str::removeOuterWhiteChars()
{
   Int n=0;  REPA(T)if(WhiteChar(T[i]))n++;else break; removeLast(n);
       n=0; FREPA(T)if(WhiteChar(T[i]))n++;else break; remove    (0, n);
   return T;
}
/******************************************************************************/
// OPERATOR=
/******************************************************************************/
Str8& Str8::operator=(CChar8 *t)
{
   if(T()!=t)
   if(!Is(t))clear();else
   {
      Int l=Length(t)+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      MoveFastN(_d.data(), t, l); // 't' can be part of 'T'
     _length=l-1;
   }
   return T;
}
Str8& Str8::operator=(CChar *t)
{
   if(!Is(t))clear();else
   {
      Int l=Length(t)+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      Set(  _d.data(), t, l);
     _length=l-1;
   }
   return T;
}
Str8& Str8::operator=(C wchar_t *t)
{
   if(!Is(t))clear();else
   {
      Int l=Length(t)+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
     _Set(  _d.data(), t, l);
     _length=l-1;
   }
   return T;
}
Str& Str::operator=(CChar8 *t)
{
   if(!Is(t))clear();else
   {
      Int l=Length(t)+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      Set(  _d.data(), t, l);
     _length=l-1;
   }
   return T;
}
Str& Str::operator=(CChar *t)
{
   if(T()!=t)
   if(!Is(t))clear();else
   {
      Int l=Length(t)+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      MoveFastN(_d.data(), t, l); // 't' can be part of 'T'
     _length=l-1;
   }
   return T;
}
Str& Str::operator=(C wchar_t *t)
{
   if(!Is(t))clear();else
   {
      Int l=Length(t)+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
     _Set(  _d.data(), t, l);
     _length=l-1;
   }
   return T;
}
/******************************************************************************/
Str8& Str8::operator=(C Str8 &s)
{
   if(this!=&s)
   if(!s.is())clear();else
   {
      Int l=s.length()+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      CopyFastN(_d.data(), s(), l);
     _length=l-1;
   }
   return T;
}
Str8& Str8::operator=(C Str &s)
{
   if(!s.is())clear();else
   {
      Int l=s.length()+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      I(); FREP(l)_d[i]=Char16To8Fast(s._d[i]); // don't use 'Set' to allow copying '\0' chars in the middle
     _length=l-1;
   }
   return T;
}
Str& Str::operator=(C Str8 &s)
{
   if(!s.is())clear();else
   {
      Int l=s.length()+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      I(); FREP(l)_d[i]=Char8To16Fast(s._d[i]); // don't use 'Set' to allow copying '\0' chars in the middle
     _length=l-1;
   }
   return T;
}
Str& Str::operator=(C Str &s)
{
   if(this!=&s)
   if(!s.is())clear();else
   {
      Int l=s.length()+1;
      if( l>_d.elms())_d.clear().setNum(l); // clear first to avoid copying existing data in 'setNum'
      CopyFastN(_d.data(), s(), l);
     _length=l-1;
   }
   return T;
}
/******************************************************************************/
Str8& Str8::operator=(Char8 c)
{
   if(!c)clear();else
   {
      if(_d.elms()<2)_d.setNum(EXTRA);
     _d[0]  =c;
     _d[1]  ='\0';
     _length=1;
   }
   return T;
}
Str8& Str8::operator=(Char c)
{
   if(!c)clear();else
   {
      if(_d.elms()<2)_d.setNum(EXTRA);
     _d[0]  =Char16To8(c);
     _d[1]  ='\0';
     _length=1;
   }
   return T;
}
Str& Str::operator=(Char8 c)
{
   if(!c)clear();else
   {
      if(_d.elms()<2)_d.setNum(EXTRA);
     _d[0]  =Char8To16(c);
     _d[1]  ='\0';
     _length=1;
   }
   return T;
}
Str& Str::operator=(Char c)
{
   if(!c)clear();else
   {
      if(_d.elms()<2)_d.setNum(EXTRA);
     _d[0]  =c;
     _d[1]  ='\0';
     _length=1;
   }
   return T;
}
/******************************************************************************/
// OPERATOR+=
/******************************************************************************/
Str8& Str8::operator+=(CChar8 *t)
{
   if(Is(t))
   {
      Int length_src =Length(t),
          length_dest=length()+length_src+1;
      if( length_dest>_d.elms())
      {
         UIntPtr offset=t-T();
        _d.setNum(length_dest+EXTRA);
         if(offset<UIntPtr(length()))t=T()+offset; // if adding text from self
      }
      MoveFastN(_d.data()+length(), t, length_src+1); // if 't' belongs to self then the last '\0' will overlap
     _length+=length_src;
   }
   return T;
}
Str8& Str8::operator+=(CChar *t)
{
   if(Is(t))
   {
      Int length_src =Length(t),
          length_dest=length()+    length_src+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
      Set(  _d.data()+length(), t, length_src+1);
     _length+=length_src;
   }
   return T;
}
Str8& Str8::operator+=(C wchar_t *t)
{
   if(Is(t))
   {
      Int length_src =Length(t),
          length_dest=length()+    length_src+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
     _Set(  _d.data()+length(), t, length_src+1);
     _length+=length_src;
   }
   return T;
}
Str& Str::operator+=(CChar8 *t)
{
   if(Is(t))
   {
      Int length_src =Length(t),
          length_dest=length()+    length_src+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
      Set(  _d.data()+length(), t, length_src+1);
     _length+=length_src;
   }
   return T;
}
Str& Str::operator+=(CChar *t)
{
   if(Is(t))
   {
      Int length_src =Length(t),
          length_dest=length()+length_src+1;
      if( length_dest>_d.elms())
      {
         UIntPtr offset=t-T();
        _d.setNum(length_dest+EXTRA);
         if(offset<UIntPtr(length()))t=T()+offset; // if adding text from self
      }
      MoveFastN(_d.data()+length(), t, length_src+1); // if 't' belongs to self then the last '\0' will overlap
     _length+=length_src;
   }
   return T;
}
Str& Str::operator+=(C wchar_t *t)
{
   if(Is(t))
   {
      Int length_src =Length(t),
          length_dest=length()+    length_src+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
     _Set(  _d.data()+length(), t, length_src+1);
     _length+=length_src;
   }
   return T;
}
/******************************************************************************/
Str8& Str8::operator+=(C Str8 &s)
{
   if(s.is())
   {
      Int     length_dest=length()+      s.length()+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
      MoveFastN(_d.data()+length(), s(), s.length()+1); // if 's' is this then the last '\0' will overlap
     _length+=s.length();
   }
   return T;
}
Str8& Str8::operator+=(C Str &s)
{
   if(s.is())
   {
      Int length_dest=length()+s.length()+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
      I(); FREP(s.length()+1)_d[length()+i]=Char16To8Fast(s[i]); // don't use 'Set' to allow copying '\0' chars in the middle - Set(_d.data()+length(), s(), s.length()+1)
     _length+=s.length();
   }
   return T;
}
Str& Str::operator+=(C Str8 &s)
{
   if(s.is())
   {
      Int length_dest=length()+s.length()+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
      I(); FREP(s.length()+1)_d[length()+i]=Char8To16Fast(s[i]); // don't use 'Set' to allow copying '\0' chars in the middle - Set(_d.data()+length(), s(), s.length()+1)
     _length+=s.length();
   }
   return T;
}
Str& Str::operator+=(C Str &s)
{
   if(s.is())
   {
      Int     length_dest=length()+      s.length()+1; if(length_dest>_d.elms())_d.setNum(length_dest+EXTRA);
      MoveFastN(_d.data()+length(), s(), s.length()+1); // if 's' is this then the last '\0' will overlap
     _length+=s.length();
   }
   return T;
}
/******************************************************************************/
Str8& Str8::operator+=(Char8 c)
{
   if(c)
   {
      if(length()+1>=_d.elms())_d.addNum(EXTRA);
     _d[_length++]=c;
     _d[_length  ]='\0';
   }
   return T;
}
Str8& Str8::operator+=(Char c)
{
   if(c)
   {
      if(length()+1>=_d.elms())_d.addNum(EXTRA);
     _d[_length++]=Char16To8(c);
     _d[_length  ]='\0';
   }
   return T;
}
Str& Str::operator+=(Char8 c)
{
   if(c)
   {
      if(length()+1>=_d.elms())_d.addNum(EXTRA);
     _d[_length++]=Char8To16(c);
     _d[_length  ]='\0';
   }
   return T;
}
Str& Str::operator+=(Char c)
{
   if(c)
   {
      if(length()+1>=_d.elms())_d.addNum(EXTRA);
     _d[_length++]=c;
     _d[_length  ]='\0';
   }
   return T;
}
/******************************************************************************/
void Str8::alwaysAppend(Char8 c)
{
   if(length()+1>=_d.elms())_d.addNum(EXTRA);
  _d[_length++]=c;
  _d[_length  ]='\0';
}
void Str::alwaysAppend(Char c)
{
   if(length()+1>=_d.elms())_d.addNum(EXTRA);
  _d[_length++]=c;
  _d[_length  ]='\0';
}
/******************************************************************************/
Str8& Str8::operator=(C BStr &s)
{
   clear().reserve(s.length()); _length=s.length(); // change length after calling 'reserve'
   I(); FREPA(T)_d[i]=Char16To8Fast(s()[i]); // () to avoid range checks
   if(_d.elms())_d[length()]='\0';
   return T;
}
Str& Str::operator=(C BStr &s)
{
   clear().reserve(s.length()); _length=s.length(); // change length after calling 'reserve'
   CopyFastN(_d.data(), s(), length());
   if(_d.elms())_d[length()]='\0';
   return T;
}
Str8& Str8::operator+=(C BStr &s)
{
   if(s.is())
   {
      reserve(length()+s.length()+EXTRA);
      I(); FREPA(s)_d[length()+i]=Char16To8Fast(s()[i]); // () to avoid range checks
     _length+=s.length();
      if(_d.elms())_d[length()]='\0';
   }
   return T;
}
Str& Str::operator+=(C BStr &s)
{
   if(s.is())
   {
      reserve(length()+s.length()+EXTRA);
      CopyFastN(_d.data()+length(), s(), s.length());
     _length+=s.length();
      if(_d.elms())_d[length()]='\0';
   }
   return T;
}
/******************************************************************************/
// OPERATOR = +=
/******************************************************************************/
Str8& Str8::operator =(Bool b) {T =(b ? '1' : '0'); return T;}
Str & Str ::operator =(Bool b) {T =(b ? '1' : '0'); return T;}
Str8& Str8::operator+=(Bool b) {T+=(b ? '1' : '0'); return T;}
Str & Str ::operator+=(Bool b) {T+=(b ? '1' : '0'); return T;}

Str8& Str8::operator =(SByte i) {Char8 temp[256]; T =TextInt(    Int(i), temp                    ); return T;}
Str8& Str8::operator+=(SByte i) {Char8 temp[256]; T+=TextInt(    Int(i), temp                    ); return T;}
Str8& Str8::operator =(Int   i) {Char8 temp[256]; T =TextInt(        i , temp                    ); return T;}
Str8& Str8::operator+=(Int   i) {Char8 temp[256]; T+=TextInt(        i , temp                    ); return T;}
Str8& Str8::operator =(Long  i) {Char8 temp[256]; T =TextInt(        i , temp                    ); return T;}
Str8& Str8::operator+=(Long  i) {Char8 temp[256]; T+=TextInt(        i , temp                    ); return T;}
Str8& Str8::operator =(Byte  u) {Char8 temp[256]; T =TextInt(   UInt(u), temp                    ); return T;}
Str8& Str8::operator+=(Byte  u) {Char8 temp[256]; T+=TextInt(   UInt(u), temp                    ); return T;}
Str8& Str8::operator =(UInt  u) {Char8 temp[256]; T =TextInt(        u , temp                    ); return T;}
Str8& Str8::operator+=(UInt  u) {Char8 temp[256]; T+=TextInt(        u , temp                    ); return T;}
Str8& Str8::operator =(ULong u) {Char8 temp[256]; T =TextInt(        u , temp                    ); return T;}
Str8& Str8::operator+=(ULong u) {Char8 temp[256]; T+=TextInt(        u , temp                    ); return T;}
Str8& Str8::operator =(Flt   f) {Char8 temp[256]; T =TextFlt(        f , temp                    ); return T;}
Str8& Str8::operator+=(Flt   f) {Char8 temp[256]; T+=TextFlt(        f , temp                    ); return T;}
Str8& Str8::operator =(Dbl   d) {Char8 temp[256]; T =TextDbl(        d , temp                    ); return T;}
Str8& Str8::operator+=(Dbl   d) {Char8 temp[256]; T+=TextDbl(        d , temp                    ); return T;}
Str8& Str8::operator =(CPtr  p) {Char8 temp[256]; T =TextHex(UIntPtr(p), temp, SIZE(p)*2, 0, true); return T;}
Str8& Str8::operator+=(CPtr  p) {Char8 temp[256]; T+=TextHex(UIntPtr(p), temp, SIZE(p)*2, 0, true); return T;}

Str& Str::operator =(SByte i) {Char8 temp[256]; T =TextInt(    Int(i), temp                    ); return T;}
Str& Str::operator+=(SByte i) {Char8 temp[256]; T+=TextInt(    Int(i), temp                    ); return T;}
Str& Str::operator =(Int   i) {Char8 temp[256]; T =TextInt(        i , temp                    ); return T;}
Str& Str::operator+=(Int   i) {Char8 temp[256]; T+=TextInt(        i , temp                    ); return T;}
Str& Str::operator =(Long  i) {Char8 temp[256]; T =TextInt(        i , temp                    ); return T;}
Str& Str::operator+=(Long  i) {Char8 temp[256]; T+=TextInt(        i , temp                    ); return T;}
Str& Str::operator =(Byte  u) {Char8 temp[256]; T =TextInt(   UInt(u), temp                    ); return T;}
Str& Str::operator+=(Byte  u) {Char8 temp[256]; T+=TextInt(   UInt(u), temp                    ); return T;}
Str& Str::operator =(UInt  u) {Char8 temp[256]; T =TextInt(        u , temp                    ); return T;}
Str& Str::operator+=(UInt  u) {Char8 temp[256]; T+=TextInt(        u , temp                    ); return T;}
Str& Str::operator =(ULong u) {Char8 temp[256]; T =TextInt(        u , temp                    ); return T;}
Str& Str::operator+=(ULong u) {Char8 temp[256]; T+=TextInt(        u , temp                    ); return T;}
Str& Str::operator =(Flt   f) {Char8 temp[256]; T =TextFlt(        f , temp                    ); return T;}
Str& Str::operator+=(Flt   f) {Char8 temp[256]; T+=TextFlt(        f , temp                    ); return T;}
Str& Str::operator =(Dbl   d) {Char8 temp[256]; T =TextDbl(        d , temp                    ); return T;}
Str& Str::operator+=(Dbl   d) {Char8 temp[256]; T+=TextDbl(        d , temp                    ); return T;}
Str& Str::operator =(CPtr  p) {Char8 temp[256]; T =TextHex(UIntPtr(p), temp, SIZE(p)*2, 0, true); return T;}
Str& Str::operator+=(CPtr  p) {Char8 temp[256]; T+=TextHex(UIntPtr(p), temp, SIZE(p)*2, 0, true); return T;}

Str8& Str8::operator =(C Vec2   &v) {Char8 temp[256]; T =TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); return T;}
Str8& Str8::operator+=(C Vec2   &v) {Char8 temp[256]; T+=TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); return T;}
Str8& Str8::operator =(C VecD2  &v) {Char8 temp[256]; T =TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); return T;}
Str8& Str8::operator+=(C VecD2  &v) {Char8 temp[256]; T+=TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); return T;}
Str8& Str8::operator =(C VecI2  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator+=(C VecI2  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator =(C VecB2  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator+=(C VecB2  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator =(C VecSB2 &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator+=(C VecSB2 &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator =(C VecUS2 &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator+=(C VecUS2 &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str8& Str8::operator =(C Vec    &v) {Char8 temp[256]; T =TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); return T;}
Str8& Str8::operator+=(C Vec    &v) {Char8 temp[256]; T+=TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); return T;}
Str8& Str8::operator =(C VecD   &v) {Char8 temp[256]; T =TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); return T;}
Str8& Str8::operator+=(C VecD   &v) {Char8 temp[256]; T+=TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); return T;}
Str8& Str8::operator =(C VecI   &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator+=(C VecI   &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator =(C VecB   &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator+=(C VecB   &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator =(C VecSB  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator+=(C VecSB  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator =(C VecUS  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator+=(C VecUS  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str8& Str8::operator =(C Vec4   &v) {Char8 temp[256]; T =TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); T+=", "; T+=TextFlt(v.w, temp); return T;}
Str8& Str8::operator+=(C Vec4   &v) {Char8 temp[256]; T+=TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); T+=", "; T+=TextFlt(v.w, temp); return T;}
Str8& Str8::operator =(C VecD4  &v) {Char8 temp[256]; T =TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); T+=", "; T+=TextDbl(v.w, temp); return T;}
Str8& Str8::operator+=(C VecD4  &v) {Char8 temp[256]; T+=TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); T+=", "; T+=TextDbl(v.w, temp); return T;}
Str8& Str8::operator =(C VecI4  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str8& Str8::operator+=(C VecI4  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str8& Str8::operator =(C VecB4  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str8& Str8::operator+=(C VecB4  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str8& Str8::operator =(C VecSB4 &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str8& Str8::operator+=(C VecSB4 &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}

Str& Str::operator =(C Vec2   &v) {Char8 temp[256]; T =TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); return T;}
Str& Str::operator+=(C Vec2   &v) {Char8 temp[256]; T+=TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); return T;}
Str& Str::operator =(C VecD2  &v) {Char8 temp[256]; T =TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); return T;}
Str& Str::operator+=(C VecD2  &v) {Char8 temp[256]; T+=TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); return T;}
Str& Str::operator =(C VecI2  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator+=(C VecI2  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator =(C VecB2  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator+=(C VecB2  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator =(C VecSB2 &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator+=(C VecSB2 &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator =(C VecUS2 &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator+=(C VecUS2 &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); return T;}
Str& Str::operator =(C Vec    &v) {Char8 temp[256]; T =TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); return T;}
Str& Str::operator+=(C Vec    &v) {Char8 temp[256]; T+=TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); return T;}
Str& Str::operator =(C VecD   &v) {Char8 temp[256]; T =TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); return T;}
Str& Str::operator+=(C VecD   &v) {Char8 temp[256]; T+=TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); return T;}
Str& Str::operator =(C VecI   &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator+=(C VecI   &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator =(C VecB   &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator+=(C VecB   &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator =(C VecSB  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator+=(C VecSB  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator =(C VecUS  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator+=(C VecUS  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); return T;}
Str& Str::operator =(C Vec4   &v) {Char8 temp[256]; T =TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); T+=", "; T+=TextFlt(v.w, temp); return T;}
Str& Str::operator+=(C Vec4   &v) {Char8 temp[256]; T+=TextFlt(v.x, temp); T+=", "; T+=TextFlt(v.y, temp); T+=", "; T+=TextFlt(v.z, temp); T+=", "; T+=TextFlt(v.w, temp); return T;}
Str& Str::operator =(C VecD4  &v) {Char8 temp[256]; T =TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); T+=", "; T+=TextDbl(v.w, temp); return T;}
Str& Str::operator+=(C VecD4  &v) {Char8 temp[256]; T+=TextDbl(v.x, temp); T+=", "; T+=TextDbl(v.y, temp); T+=", "; T+=TextDbl(v.z, temp); T+=", "; T+=TextDbl(v.w, temp); return T;}
Str& Str::operator =(C VecI4  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str& Str::operator+=(C VecI4  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str& Str::operator =(C VecB4  &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str& Str::operator+=(C VecB4  &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str& Str::operator =(C VecSB4 &v) {Char8 temp[256]; T =TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
Str& Str::operator+=(C VecSB4 &v) {Char8 temp[256]; T+=TextInt(v.x, temp); T+=", "; T+=TextInt(v.y, temp); T+=", "; T+=TextInt(v.z, temp); T+=", "; T+=TextInt(v.w, temp); return T;}
/******************************************************************************/
Str8 Str8::operator+(CChar8 *t)C
{
   if(!Is(t))return T;
   Int length=Length(t); Str8 s(T, length+EXTRA); CopyFastN(s._d.data()+s.length(), t, 1+length); s._length+=length;
   return s;
}
Str Str8::operator+(CChar *t)C
{
   if(!Is(t))return T;
   Int length=Length(t); Str s(T, length+EXTRA); CopyFastN(s._d.data()+s.length(), t, 1+length); s._length+=length;
   return s;
}
Str Str::operator+(CChar8 *t)C
{
   if(!Is(t))return T;
   Int length=Length(t); Str s(T, length+EXTRA); Set(s._d.data()+s.length(), t, 1+length); s._length+=length;
   return s;
}
Str Str::operator+(CChar *t)C
{
   if(!Is(t))return T;
   Int length=Length(t); Str s(T, length+EXTRA); CopyFastN(s._d.data()+s.length(), t, 1+length); s._length+=length;
   return s;
}
Str Str8::operator+(C wchar_t *t)C
{
   if(!Is(t))return T;
   Int length=Length(t); Str s(T, length+EXTRA); _Set(s._d.data()+s.length(), t, 1+length); s._length+=length;
   return s;
}
Str Str::operator+(C wchar_t *t)C
{
   if(!Is(t))return T;
   Int length=Length(t); Str s(T, length+EXTRA); _Set(s._d.data()+s.length(), t, 1+length); s._length+=length;
   return s;
}
/******************************************************************************/
#define  SBYTEC         (1+3)  // sbyte  chars "-128"
#define   BYTEC            3   // byte   chars "255"
#define USHORTC            5   // ushort chars "65535"
#define    INTC        (1+10)  // int    chars "-2147483647"
#define   UINTC           10   // uint   chars "4294967295"
#define   LONGC        (1+19)  // long   chars "-9223372036854775807"
#define  ULONGC           20   // ulong  chars "18446744073709551615"
#define    FLTC     (1+5+1+3)  // float  chars "-65535.123"            (approximation)
#define    DBLC    (1+10+1+9)  // double chars "-2147483647.123456789" (approximation)
#define    PTRC (2+(X64?16:8)) // Ptr    chars X64 ? "0x1234567812345678" : "0x12345678"
#define   COMMA            2   // comma  chars ", "

Str8 Str8::operator+(C Str8   &s)C {return RValue(Str8(T,          s.length() + EXTRA)+=s);}
Str  Str8::operator+(C Str    &s)C {return RValue(Str (T,          s.length() + EXTRA)+=s);}
Str  Str ::operator+(C Str8   &s)C {return RValue(Str (T,          s.length() + EXTRA)+=s);}
Str  Str ::operator+(C Str    &s)C {return RValue(Str (T,          s.length() + EXTRA)+=s);}
Str  Str8::operator+(C BStr   &s)C {return RValue(Str (T,          s.length() + EXTRA)+=s);}
Str  Str ::operator+(C BStr   &s)C {return RValue(Str (T,          s.length() + EXTRA)+=s);}
Str8 Str8::operator+(  Char8   c)C {return RValue(Str8(T,                       EXTRA)+=c);}
Str  Str8::operator+(  Char    c)C {return RValue(Str (T,                       EXTRA)+=c);}
Str  Str ::operator+(  Char8   c)C {return RValue(Str (T,                       EXTRA)+=c);}
Str  Str ::operator+(  Char    c)C {return RValue(Str (T,                       EXTRA)+=c);}
Str8 Str8::operator+(  Bool    b)C {return RValue(Str8(T,                       EXTRA)+=b);}
Str  Str ::operator+(  Bool    b)C {return RValue(Str (T,                       EXTRA)+=b);}
Str8 Str8::operator+(  SByte   i)C {return RValue(Str8(T,    SBYTEC +           EXTRA)+=i);}
Str  Str ::operator+(  SByte   i)C {return RValue(Str (T,    SBYTEC +           EXTRA)+=i);}
Str8 Str8::operator+(  Int     i)C {return RValue(Str8(T,      INTC +           EXTRA)+=i);}
Str  Str ::operator+(  Int     i)C {return RValue(Str (T,      INTC +           EXTRA)+=i);}
Str8 Str8::operator+(  Long    i)C {return RValue(Str8(T,     LONGC +           EXTRA)+=i);}
Str  Str ::operator+(  Long    i)C {return RValue(Str (T,     LONGC +           EXTRA)+=i);}
Str8 Str8::operator+(  Byte    u)C {return RValue(Str8(T,     BYTEC +           EXTRA)+=u);}
Str  Str ::operator+(  Byte    u)C {return RValue(Str (T,     BYTEC +           EXTRA)+=u);}
Str8 Str8::operator+(  UInt    u)C {return RValue(Str8(T,     UINTC +           EXTRA)+=u);}
Str  Str ::operator+(  UInt    u)C {return RValue(Str (T,     UINTC +           EXTRA)+=u);}
Str8 Str8::operator+(  ULong   u)C {return RValue(Str8(T,    ULONGC +           EXTRA)+=u);}
Str  Str ::operator+(  ULong   u)C {return RValue(Str (T,    ULONGC +           EXTRA)+=u);}
Str8 Str8::operator+(  Flt     f)C {return RValue(Str8(T,      FLTC +           EXTRA)+=f);}
Str  Str ::operator+(  Flt     f)C {return RValue(Str (T,      FLTC +           EXTRA)+=f);}
Str8 Str8::operator+(  Dbl     d)C {return RValue(Str8(T,      DBLC +           EXTRA)+=d);}
Str  Str ::operator+(  Dbl     d)C {return RValue(Str (T,      DBLC +           EXTRA)+=d);}
Str8 Str8::operator+(  CPtr    p)C {return RValue(Str8(T,      PTRC +           EXTRA)+=p);}
Str  Str ::operator+(  CPtr    p)C {return RValue(Str (T,      PTRC +           EXTRA)+=p);}
Str8 Str8::operator+(C Vec2   &v)C {return RValue(Str8(T, 2*   FLTC +   COMMA + EXTRA)+=v);}
Str  Str ::operator+(C Vec2   &v)C {return RValue(Str (T, 2*   FLTC +   COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecD2  &v)C {return RValue(Str8(T, 2*   DBLC +   COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecD2  &v)C {return RValue(Str (T, 2*   DBLC +   COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecI2  &v)C {return RValue(Str8(T, 2*   INTC +   COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecI2  &v)C {return RValue(Str (T, 2*   INTC +   COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecB2  &v)C {return RValue(Str8(T, 2*  BYTEC +   COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecB2  &v)C {return RValue(Str (T, 2*  BYTEC +   COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecSB2 &v)C {return RValue(Str8(T, 2* SBYTEC +   COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecSB2 &v)C {return RValue(Str (T, 2* SBYTEC +   COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecUS2 &v)C {return RValue(Str8(T, 2*USHORTC +   COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecUS2 &v)C {return RValue(Str (T, 2*USHORTC +   COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C Vec    &v)C {return RValue(Str8(T, 3*   FLTC + 2*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C Vec    &v)C {return RValue(Str (T, 3*   FLTC + 2*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecD   &v)C {return RValue(Str8(T, 3*   DBLC + 2*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecD   &v)C {return RValue(Str (T, 3*   DBLC + 2*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecI   &v)C {return RValue(Str8(T, 3*   INTC + 2*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecI   &v)C {return RValue(Str (T, 3*   INTC + 2*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecB   &v)C {return RValue(Str8(T, 3*  BYTEC + 2*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecB   &v)C {return RValue(Str (T, 3*  BYTEC + 2*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecSB  &v)C {return RValue(Str8(T, 3* SBYTEC + 2*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecSB  &v)C {return RValue(Str (T, 3* SBYTEC + 2*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecUS  &v)C {return RValue(Str8(T, 3*USHORTC + 2*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecUS  &v)C {return RValue(Str (T, 3*USHORTC + 2*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C Vec4   &v)C {return RValue(Str8(T, 4*   FLTC + 3*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C Vec4   &v)C {return RValue(Str (T, 4*   FLTC + 3*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecD4  &v)C {return RValue(Str8(T, 4*   DBLC + 3*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecD4  &v)C {return RValue(Str (T, 4*   DBLC + 3*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecI4  &v)C {return RValue(Str8(T, 4*   INTC + 3*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecI4  &v)C {return RValue(Str (T, 4*   INTC + 3*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecB4  &v)C {return RValue(Str8(T, 4*  BYTEC + 3*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecB4  &v)C {return RValue(Str (T, 4*  BYTEC + 3*COMMA + EXTRA)+=v);}
Str8 Str8::operator+(C VecSB4 &v)C {return RValue(Str8(T, 4* SBYTEC + 3*COMMA + EXTRA)+=v);}
Str  Str ::operator+(C VecSB4 &v)C {return RValue(Str (T, 4* SBYTEC + 3*COMMA + EXTRA)+=v);}
/******************************************************************************/
// STRING LIBRARY
/******************************************************************************/
static inline Int CompareSCI(C Str &a, C Str &b) {return Compare(a(), b(), false);}
static inline Int CompareSCS(C Str &a, C Str &b) {return Compare(a(), b(), true );}

static Int (&GetCompare(Bool case_sensitive, Bool paths)) (C Str &a, C Str &b)
{
   return paths ? (case_sensitive ? ComparePathCS : ComparePathCI) : (case_sensitive ? CompareSCS : CompareSCI);
}

void StrLibrary::del()
{
   Free(_index);
   Free(_data );
  _elms=_size=0;
}
void StrLibrary::create(C MemPtr<Str> &strings, Bool case_sensitive, Bool paths)
{
   del();

   T._case_sensitive=case_sensitive;
   T._paths         =paths;

   Int     (&compare)(C Str &a, C Str &b)=GetCompare(case_sensitive, paths);
   Memt<Str> cleaned;
#if 0 // check if already added during adding (requires all vs all checking)
   cleaned.New(); // insert empty string at start so saving empty strings will require only 1 byte
   REPA(strings)if(strings[i].is())
   {
      REPAD(j, cleaned)if(!compare(strings[i], cleaned[j]))goto present;
      cleaned.add(strings[i]);
   present:;
   }
   cleaned.sort(case_sensitive ? CompareCS : CompareCI);
#else // add all, then sort, then check neighbors only, much faster
   cleaned=strings; cleaned.New(); // insert empty string so saving empty strings will require only 1 byte
   cleaned.sort(compare);
   REPA(cleaned)if(i)if(!compare(cleaned[i], cleaned[i-1]))cleaned.remove(i, true); // remove duplicates
#endif

   REPA(cleaned)
   {
      if(HasUnicode(cleaned[i]))_size+=SIZE(Char )*(cleaned[i].length()+1);
      else                      _size+=SIZE(Char8)*(cleaned[i].length()+1);
   }
  _elms=cleaned.elms();
   Alloc(_data , _size);
   Alloc(_index, _elms);
   Int data_pos=0;
   FREPA(cleaned) // go from start
   {
      if(HasUnicode(cleaned[i])){Set((Char *)(_data+data_pos), cleaned[i], cleaned[i].length()+1); _index[i]=data_pos^SIGN_BIT; data_pos+=SIZE(Char )*(cleaned[i].length()+1);}
      else                      {Set((Char8*)(_data+data_pos), cleaned[i], cleaned[i].length()+1); _index[i]=data_pos         ; data_pos+=SIZE(Char8)*(cleaned[i].length()+1);}
   }
}
StrLibrary::StrLibrary(                                                       )                {Zero(T);}
StrLibrary::StrLibrary(C MemPtr<Str> &strings, Bool case_sensitive, Bool paths) : StrLibrary() {create(strings, case_sensitive, paths);}
/******************************************************************************/
Str StrLibrary::elm(Int i)C
{
   if(InRange(i, elms()))
   {
      i=_index[i];
      if(i&SIGN_BIT)return (CChar *)(_data+(i^SIGN_BIT));
                    return (CChar8*)(_data+ i          );
   }
   return S;
}
/******************************************************************************/
void StrLibrary::putStr(File &f, C Str &str)C
{
   Int (&compare)(C Str &a, C Str &b)=GetCompare(_case_sensitive, _paths);
   Int l=0, r=_elms, found=-1; // -1 is 0xFFFFFFFF
   for(; l<r; )
   {
      Int mid       =UInt(l+r)/2,
          data_index=_index[mid],
          c         =((data_index&SIGN_BIT) ? compare((CChar*)(_data+(data_index^SIGN_BIT)), str) : compare((CChar8*)(_data+data_index), str));
      if( c<0)l=mid+1;else
      if( c>0)r=mid  ;else {found=mid; break;}
   }
   // normally we could use 0x100, 0x10000, 0x1000000 but we need to reserve the last element for a custom string
   ASSERT(SIZE(found)>=4);
   if(_elms<=0x0000FF)f.put   (&found, 1);else
   if(_elms<=0x00FFFF)f.put   (&found, 2);else
   if(_elms<=0xFFFFFF)f.put   (&found, 3);else
                      f.put   (&found, 4);
   if(found<0        )f.putStr( str     );
}
void StrLibrary::getStr(File &f, Str &str)C
{
   Int index=0; ASSERT(SIZE(index)>=4);
   if(_elms<=0x0000FF)f.getFast(&index, 1);else
   if(_elms<=0x00FFFF)f.getFast(&index, 2);else
   if(_elms<=0xFFFFFF)f.getFast(&index, 3);else
                      f.getFast(&index, 4);
   if(InRange(index, _elms))
   {
      Int data_index=T._index[index];
      if( data_index&SIGN_BIT)str=(CChar *)(_data+(data_index^SIGN_BIT));
      else                    str=(CChar8*)(_data+ data_index          );
   }else f.getStr(str);
}
/******************************************************************************/
Bool StrLibrary::save(File &f)C
{
   f.putMulti(Byte(0), Byte((_case_sensitive ? 1 : 0)|(_paths ? 2 : 0)), _elms, _size); // version, flag
   f.putN(_index, _elms);
   f.putN(_data , _size);
   return f.ok();
}
Bool StrLibrary::load(File &f)
{
   del(); switch(f.decUIntV()) // version
   {
      case 0:
      {
         Byte flag; f.getMulti(flag, _elms, _size); _case_sensitive=FlagTest(flag, 1); _paths=FlagTest(flag, 2);
         f.getFastN(Alloc(_index, _elms), _elms);
         f.getFastN(Alloc(_data , _size), _size);
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// BORROWED STRING
/******************************************************************************/
BStr& BStr::set        (C BStr &s            ) {T=s; return T;}
BStr& BStr::setCustom  (C Str  &s            ) {return setCustom(s(), s.length());}
BStr& BStr::setCustom  (CChar  *t, Int length) {if(length<0)length=Length(t); del(); if(length){T._custom=true ; Char *d; Alloc(d, length); CopyFastN(d, t, length); T._d=d; T._length=length;} return T;}
BStr& BStr::setBorrowed(CChar  *t, Int length) {if(length<0)length=Length(t); del(); if(length){T._custom=false;                                                     T._d=t; T._length=length;} return T;}

BStr& BStr::clear (     ) {if( _custom){Free(_d); _custom=false;}else _d=null; _length=0; return T;}
BStr& BStr::extend(Int l) {if(!_custom)_length+=l; return T;}

Bool BStr::operator==(CChar   c)C {return _length==1 && _d[0]==          c ;}
Bool BStr::operator==(CChar8  c)C {return _length==1 && _d[0]==Char8To16(c);}
Bool BStr::operator==(CChar  *t)C {if(t){     FREP(_length)if(_d[i]!=              *t++ )return false; return *t=='\0';} return _length==0;}
Bool BStr::operator==(CChar8 *t)C {if(t){I(); FREP(_length)if(_d[i]!=Char8To16Fast(*t++))return false; return *t=='\0';} return _length==0;}
Bool BStr::operator==(C Str  &s)C {if(_length!=s.length())return false;      FREP(_length)if(_d[i]!=              s   [i] )return false; return true;}
Bool BStr::operator==(C Str8 &s)C {if(_length!=s.length())return false; I(); FREP(_length)if(_d[i]!=Char8To16Fast(s   [i]))return false; return true;}
Bool BStr::operator==(C BStr &s)C {if(_length!=s.length())return false;      FREP(_length)if(_d[i]!=              s._d[i] )return false; return true;}

BStr& BStr::operator=(C BStr &src)
{
   if(this!=&src)
   {
      if(src._custom)setCustom  (src(), src.length());
      else           setBorrowed(src(), src.length());
   }
   return T;
}
BStr::BStr(C BStr &src) : BStr() {T=src;}
/******************************************************************************/
Int CompareCS(C Str &a, C BStr &b)
{
   FREPA(b)
   {
      Int ord0=a[i],
          ord1=b[i];
      if(ord0<ord1)return -1;
      if(ord0>ord1)return +1;
   }
   return (a.length()>b.length()) ? +1 : 0; // if 'a' is longer then return +1 (because 'a' should be placed after 'b'), in other case they are equal so return 0
}
/******************************************************************************/
// MAIN
/******************************************************************************/
#if DEBUG && 0
#pragma message("!! Warning: Use this only for debugging !!")
#if !WINDOWS
#error This should be generated only on Windows
#endif
static void GenMap()
{
   Initialized=true;
   setlocale(LC_ALL, ""); // needed for 'towlower', 'towupper', 'iswalpha', ..

   Memc<VecI2> ranges;
   FREP(65536)if(iswalpha(i))if(ranges.elms() && ranges.last().y==i-1)ranges.last().y++;else ranges.New()=i;
   Str s;
   s+="static const U16 Alphas[][2]={";
   FREPA(ranges){if(i)s+=", "; s+=ranges[i].x; s+=','; s+=ranges[i].y;}
   s+="};\n";

   Memc<VecI2> dus;
   FREP(65536)
   {
      U16 l=towlower(i), u=towupper(i);
      if( l!=i)dus.binaryInclude(VecI2(l, i), Compare);
      if( i!=u)dus.binaryInclude(VecI2(i, u), Compare);
   }
   s+="static const U16 DownUps[][2]={";
   FREPA(dus){if(i)s+=", "; s+=dus[i].x; s+=','; s+=dus[i].y;}
   s+="};\n";

#if WINDOWS
   ClipSet(s);
#endif
   Exit(S+"GenMap:"+ranges.elms()+' '+dus.elms());
}
#endif
static void InitStr()
{
#if USE_STD || LINUX // Linux needs this for Unicode Keyboard Input 'XwcLookupString'
   setlocale(LC_ALL, ""); // needed for 'towlower', 'towupper', 'iswalpha', ..
#endif

   // Char 8<->16 conversions
   {
      SetMem(_Char16To8, '?', SIZE(_Char16To8));
      REP(256)
      {
         Char8 c=i;
      #if WINDOWS
         wchar_t w;
         MultiByteToWideChar(CP_ACP, 0, &c, 1, &w, 1);
         // many 'c' characters can point to the same 'w', however we need "Char16To8(Char8To16(c))==c", so the characters that get repeated, we need to store them as original, these are:
         if(w=='?'        // occurs in Chinese Traditional
         || U16(w)==12539 // occurs in Japanese
         )w=c;
        _Char8To16[U8 (c)]=w;
        _Char16To8[U16(w)]=c;
      #else // on other platforms accented characters use UTF8 (one accented character may use multiple chars, so single char of value >=128 does not have a code page like on Windows), just use direct copy, this is needed for 'CreateShortcut' Linux version where UTF8 is saved using 'Str'
        _Char8To16[U8(c)]=c;
        _Char16To8[U8(c)]=c;
      #endif
      }
   }

   // Case Up/Down
   {
   #if USE_STD
      REPAO(_CaseDown)=towlower(i);
      REPAO(_CaseUp  )=towupper(i);
   #else
      REPAO(_CaseDown)=i;
      REPAO(_CaseUp  )=i;
      REPA ( DownUps ){U16 d=DownUps[i][0], u=DownUps[i][1]; _CaseDown[u]=d; _CaseUp[d]=u;}
   #endif

   #define SET(l, h) {_CaseDown[U16(L##h)]=L##l; _CaseUp[U16(L##l)]=L##h;}
      SET('ß', 'ẞ');
   #undef SET
   }

   // Character Order - do not make any changes to the character order as it should remain frozen! because this order affects the sort order of how files are stored in PAK files
   {
   #define SET( x   ) CharOrder16[U16(L##x)]=o;                           o++
   #define SET2(l, h) CharOrder16[U16(L##l)]=o; CharOrder16[U16(L##h)]=o; o++
      U16 o=0;
      SET('\0');
      SET('\r');
      SET('\n');

      SET('/' ); // set slash and backslash as first (this is important in order for 'ComparePath' to sort paths correctly)
      SET('\\');
      SET('|' ); // keep close to '/\', file names can't have '|' anyway

      SET('.'); // prioritize '.' so file names "file.ext" are sorted before "file*.ext"
      SET(','); // keep close to '.'

      SET('!'); SET('@'); SET('#'); SET('$'); SET('%'); SET('^'); SET('&'); SET('*');
      SET('(');
      SET(')');
      SET('[');
      SET(']');
      SET('{');
      SET('}');
      SET('<');
      SET('>');
      SET('~');
      SET('-');
      SET('+');
      SET('=');
      SET(';');
      SET(':');
      SET('`');
      SET('\'');
      SET('"');
      SET('?');

      SET(' ' ); // insert before 012..abc.. so "file 2.ext" is before "file2.ext"
      SET('\t'); // treat tabs as big spaces
      SET('_' ); // keep close to ' ' but after

      SET('0'); SET('1'); SET('2'); SET('3'); SET('4'); SET('5'); SET('6'); SET('7'); SET('8'); SET('9');

      SET2('a', 'A');
      SET2('ą', 'Ą'); // polish
      SET2('à', 'À');
      SET2('á', 'Á');
      SET2('â', 'Â');
      SET2('ã', 'Ã');
      SET2('ä', 'Ä');
      SET2('å', 'Å');
      SET2('æ', 'Æ');
      SET2('b', 'B');
      SET2('c', 'C');
      SET2('ć', 'Ć'); // polish
      SET2('ç', 'Ç');
      SET2('d', 'D');
      SET2('e', 'E');
      SET2('ę', 'Ę'); // polish
      SET2('è', 'È');
      SET2('é', 'É');
      SET2('ê', 'Ê');
      SET2('ë', 'Ë');
      SET2('f', 'F');
      SET2('g', 'G');
      SET2('h', 'H');
      SET2('i', 'I');
      SET2('ì', 'Ì');
      SET2('í', 'Í');
      SET2('î', 'Î');
      SET2('ï', 'Ï');
      SET2('j', 'J');
      SET2('k', 'K');
      SET2('l', 'L');
      SET2('ł', 'Ł'); // polish
      SET2('m', 'M');
      SET2('n', 'N');
      SET2('ń', 'Ń'); // polish
      SET2('ñ', 'Ñ');
      SET2('o', 'O');
      SET2('ò', 'Ò');
      SET2('ó', 'Ó'); // polish
      SET2('ô', 'Ô');
      SET2('õ', 'Õ');
      SET2('ö', 'Ö');
      SET2('ø', 'Ø');
      SET2('p', 'P');
      SET2('q', 'Q');
      SET2('r', 'R');
      SET2('ŕ', 'Ŕ');
      SET2('s', 'S');
      SET2('ś', 'Ś'); // polish
      SET2('ß', 'ẞ');
      SET2('t', 'T');
      SET2('u', 'U');
      SET2('ù', 'Ù');
      SET2('ú', 'Ú');
      SET2('û', 'Û');
      SET2('ü', 'Ü');
      SET2('v', 'V');
      SET2('w', 'W');
      SET2('x', 'X');
      SET2('y', 'Y');
      SET2('ý', 'Ý');
      SET2('z', 'Z');
      SET2('ż', 'Ż'); // polish
      SET2('ź', 'Ź'); // polish
   #undef SET2
   #undef SET

      // remaining characters
      for(Int i=1; i<Elms(CharOrder16); i++)if(!CharOrder16[i])CharOrder16[i]=o++; // if the character doesn't have order set, then set (skip 0 '\0')

      // Char8
      REPAO(CharOrder8)=CharOrderFast(Char8To16Fast(i));
   }

   // Char Flag
   {
      // set custom
   #define SET(c, f) _CharFlag[U16(c)]|=f;
                                   SET(L' '          , CHARF_SPACE);
                                   SET(Nbsp          , CHARF_SPACE);
                                   SET(FullWidthSpace, CHARF_SPACE);
                                   SET('\t'          , CHARF_SPACE);
                                   SET(L'_'          , CHARF_UNDER);
                                   SET(L'0'          , CHARF_DIG2|CHARF_DIG10|CHARF_DIG16);
                                   SET(L'1'          , CHARF_DIG2|CHARF_DIG10|CHARF_DIG16);
      for(Int i=L'2'; i<=L'9'; i++)SET(i             ,            CHARF_DIG10|CHARF_DIG16);
      for(Int i=L'a'; i<=L'f'; i++)SET(i             ,                        CHARF_DIG16);
      for(Int i=L'A'; i<=L'F'; i++)SET(i             ,                        CHARF_DIG16);
             REP(Elms(Combining)-1)SET(Combining[i]  , CHARF_COMBINING); // -1 to skip null char
             REP(Elms(Stack    )-1)SET(Stack    [i]  , CHARF_STACK    ); // -1 to skip null char
   #undef  SET

      // set sign
   #define SET(c) _CharFlag[U16(c)]|=CHARF_SIGN;
      SET('_');
      SET('`');
      SET('~');
      SET('!');
      SET('@');
      SET('#');
      SET('$');
      SET('%');
      SET('^');
      SET('&');
      SET('-');
      SET('+');
      SET('*');
      SET('=');
      SET('(');
      SET(')');
      SET('[');
      SET(']');
      SET('{');
      SET('}');
      SET('<');
      SET('>');
      SET(';');
      SET(':');
      SET('\'');
      SET('"');
      SET(',');
      SET('.');
      SET('/');
      SET('|');
      SET('\\');
      SET('?');
      SET(CharBullet);
      SET(CharCopyright);
      SET(CharDegree);
      SET(    Ellipsis);
      SET(CharPermil);
      SET(CharSection);
      SET(CharStar);
      SET(CharTrademark);
   #undef SET

   #if !USE_STD
      FREPA(Alphas)
         for(Int c=Alphas[i][0], to=Alphas[i][1]; c<=to; c++)_CharFlag[c]|=CHARF_ALPHA;
   #endif

      REP(65536)
      {
         Char c=i;
         UInt f=_CharFlag[i];

      #if USE_STD
         if(iswalpha(c))f|=CHARF_ALPHA;
      #endif

         if(CaseDownFast(c)!=c)f|=CHARF_UP  ;/*else
         if(CaseUpFast  (c)!=c)f|=CHARF_DOWN; CHARF_DOWN is not defined*/

         if(!(f&CHARF_ALPHA) || c=='_')f|=CHARF_FONT_SPACE; // everything except characters, and always include '_'
        _CharFlag[i]=f;
      }
   }

#if DEBUG && 0 // creation of 'CharReplaces'
   Memt<CharReplace> replaces;
#define SET(sl, su, dl, du) replaces.New().set(L##sl, dl); replaces.New().set(L##su, du);
   SET('ą', 'Ą', 'a', 'A'); // polish
   SET('à', 'À', 'a', 'A');
   SET('á', 'Á', 'a', 'A');
   SET('â', 'Â', 'a', 'A');
   SET('ã', 'Ã', 'a', 'A');
   SET('ä', 'Ä', 'a', 'A');
   SET('å', 'Å', 'a', 'A');
   SET('æ', 'Æ', 'a', 'A');
   SET('ć', 'Ć', 'c', 'C'); // polish
   SET('ç', 'Ç', 'c', 'C');
   SET('ę', 'Ę', 'e', 'E'); // polish
   SET('è', 'È', 'e', 'E');
   SET('é', 'É', 'e', 'E');
   SET('ê', 'Ê', 'e', 'E');
   SET('ë', 'Ë', 'e', 'E');
   SET('ì', 'Ì', 'i', 'I');
   SET('í', 'Í', 'i', 'I');
   SET('î', 'Î', 'i', 'I');
   SET('ï', 'Ï', 'i', 'I');
   SET('ł', 'Ł', 'l', 'L'); // polish
   SET('ń', 'Ń', 'n', 'N'); // polish
   SET('ñ', 'Ñ', 'n', 'N');
   SET('ò', 'Ò', 'o', 'O');
   SET('ó', 'Ó', 'o', 'O'); // polish
   SET('ô', 'Ô', 'o', 'O');
   SET('õ', 'Õ', 'o', 'O');
   SET('ö', 'Ö', 'o', 'O');
   SET('ø', 'Ø', 'o', 'O');
   SET('ŕ', 'Ŕ', 'r', 'R');
   SET('ś', 'Ś', 's', 'S'); // polish
   SET('ß', 'ẞ', 's', 'S');
   SET('ù', 'Ù', 'u', 'U');
   SET('ú', 'Ú', 'u', 'U');
   SET('û', 'Û', 'u', 'U');
   SET('ü', 'Ü', 'u', 'U');
   SET('ý', 'Ý', 'y', 'Y');
   SET('ż', 'Ż', 'z', 'Z'); // polish
   SET('ź', 'Ź', 'z', 'Z'); // polish
#undef SET
   replaces.sort(CharReplace::Compare);
   Str s; FREPA(replaces)s+=S+"   {L'"+replaces[i].src+"', '"+replaces[i].dest+"'},\n"; ClipSet(s);
#endif
}
Char RemoveAccent(Char c)
{
   if(CharReplace *cr=BinaryFind(CharReplaces, Elms(CharReplaces), c, CharReplace::Compare))return cr->dest;
   return c;
}
Font& Font::removeAccent(Bool permanent)
{
   REPA(CharReplaces)replace(CharReplaces[i].src, CharReplaces[i].dest, permanent);
   return T;
}
/******************************************************************************/
#if APPLE
static Bool HasUnicode(NSString *str)
{
   if(str)REP([str length])if(U16([str characterAtIndex:i])>=128)return true;
   return false;
}
/******************************************************************************/
NSString* AppleString(C Str &str)
{
   ASSERT(SIZE(unichar)==SIZE(Char));
   return [[NSString alloc] initWithCharacters:(const unichar*)(str() ? str() : u"") length:str.length()];
}
Str AppleString(NSString *str)
{
   if(str)
      if(NSString *s=(HasUnicode(str) ? [str precomposedStringWithCanonicalMapping] : str))
   {
      Str temp;
      if(Int length=[s length])
      {
         ASSERT(SIZE(unichar)==SIZE(Char));
         temp.reserve(length); [s getCharacters:(unichar*)temp()]; temp._d[temp._length=length]='\0';
      }
    //if(s!=str)[s release]; don't release this as crashes may occur
      return temp;
   }
   return S;
}
/******************************************************************************/
NSURLAuto::NSURLAuto(C Str &str) : NSStringAuto(UnixPath(Replace(str, " ", "%20"))) // 'NSURL' fails for urls with spaces (don't use '+' because that didn't work on Mac when opening a link with spaces)
{
   url=(NSStringAuto::operator()() ? [NSURL URLWithString:T] : null); // !! use 'URLWithString' only, because we don't release it in destructor !!
}
#elif ANDROID
Str JNI::operator()(jstring str)C
{
   if(str && T)
   #if 1
      if(Int length=T->GetStringLength(str))
         if(C jchar *text=T->GetStringChars(str, null))
            {Str s; s.reserve(length); FREP(length)s+=((CChar*)text)[i]; T->ReleaseStringChars(str, text); return s;}
   #else
      if(CChar8 *utf=T->GetStringUTFChars(str, null))
         {C Str &s=FromUTF8(utf); T->ReleaseStringUTFChars(str, utf); return s;}
   #endif
   return S;
}
/*Str JString::str()C
{
   if(T)
   #if 1
      if(Int length=_jni->GetStringLength(T))
         if(C jchar *text=_jni->GetStringChars(T, null))
            {Str s; s.reserve(length); FREP(length)s+=((CChar*)text)[i]; _jni->ReleaseStringChars(T, text); return s;}
   #else
      if(CChar8 *utf=_jni->GetStringUTFChars(T, null))
         {C Str &s=FromUTF8(utf); _jni->ReleaseStringUTFChars(T, utf); return s;}
   #endif
   return S;
}*/
#endif
/******************************************************************************/
}
/******************************************************************************/
