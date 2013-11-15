/* Name: main.c
 * Author: Jonathan M. Guberman
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "TinyWireM.h"        // I2C library for ATtiny AVR
#include "nunchuck_funcs.h"   // Wii Nunchuck helper functions
#include <avr/pgmspace.h>

 volatile unsigned long accumulator, phase, volume;
 //volatile unsigned char volume;
 unsigned char outvalue;
 

/*
const char squarewave[256] = {
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
 -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 
 };

 const char trianglewave[256] = {
 -127 , -125 , -123 , -121 , -119 , -117 , -115 , -113 , -111 , -109 , -107 , -105 , -103 , -101 , -99 , -97 , 
 -95 , -93 , -91 , -89 , -87 , -85 , -83 , -81 , -79 , -77 , -75 , -73 , -71 , -69 , -67 , -65 , 
 -63 , -61 , -59 , -57 , -55 , -53 , -51 , -49 , -47 , -45 , -43 , -41 , -39 , -37 , -35 , -33 , 
 -31 , -29 , -27 , -25 , -23 , -21 , -19 , -17 , -15 , -13 , -11 , -9 , -7 , -5 , -3 , -1 , 
 1 , 3 , 5 , 7 , 9 , 11 , 13 , 15 , 17 , 19 , 21 , 23 , 25 , 27 , 29 , 31 , 
 33 , 35 , 37 , 39 , 41 , 43 , 45 , 47 , 49 , 51 , 53 , 55 , 57 , 59 , 61 , 63 , 
 65 , 67 , 69 , 71 , 73 , 75 , 77 , 79 , 81 , 83 , 85 , 87 , 89 , 91 , 93 , 95 , 
 97 , 99 , 101 , 103 , 105 , 107 , 109 , 111 , 113 , 115 , 117 , 119 , 121 , 123 , 125 , 127 , 
 127 , 127 , 125 , 123 , 121 , 119 , 117 , 115 , 113 , 111 , 109 , 107 , 105 , 103 , 101 , 99 , 
 97 , 95 , 93 , 91 , 89 , 87 , 85 , 83 , 81 , 79 , 77 , 75 , 73 , 71 , 69 , 67 , 
 65 , 63 , 61 , 59 , 57 , 55 , 53 , 51 , 49 , 47 , 45 , 43 , 41 , 39 , 37 , 35 , 
 33 , 31 , 29 , 27 , 25 , 23 , 21 , 19 , 17 , 15 , 13 , 11 , 9 , 7 , 5 , 3 , 
 1 , -1 , -3 , -5 , -7 , -9 , -11 , -13 , -15 , -17 , -19 , -21 , -23 , -25 , -27 , -29 , 
 -31 , -33 , -35 , -37 , -39 , -41 , -43 , -45 , -47 , -49 , -51 , -53 , -55 , -57 , -59 , -61 , 
 -63 , -65 , -67 , -69 , -71 , -73 , -75 , -77 , -79 , -81 , -83 , -85 , -87 , -89 , -91 , -93 , 
 -95 , -97 , -99 , -101 , -103 , -105 , -107 , -109 , -111 , -113 , -115 , -117 , -119 , -121 , -123 , -125 
 };

 const char sawtoothwave[256] = {
 -127 , -127 , -126 , -125 , -124 , -123 , -122 , -121 , -120 , -119 , -118 , -117 , -116 , -115 , -114 , -113 , 
 -112 , -111 , -110 , -109 , -108 , -107 , -106 , -105 , -104 , -103 , -102 , -101 , -100 , -99 , -98 , -97 , 
 -96 , -95 , -94 , -93 , -92 , -91 , -90 , -89 , -88 , -87 , -86 , -85 , -84 , -83 , -82 , -81 , 
 -80 , -79 , -78 , -77 , -76 , -75 , -74 , -73 , -72 , -71 , -70 , -69 , -68 , -67 , -66 , -65 , 
 -64 , -63 , -62 , -61 , -60 , -59 , -58 , -57 , -56 , -55 , -54 , -53 , -52 , -51 , -50 , -49 , 
 -48 , -47 , -46 , -45 , -44 , -43 , -42 , -41 , -40 , -39 , -38 , -37 , -36 , -35 , -34 , -33 , 
 -32 , -31 , -30 , -29 , -28 , -27 , -26 , -25 , -24 , -23 , -22 , -21 , -20 , -19 , -18 , -17 , 
 -16 , -15 , -14 , -13 , -12 , -11 , -10 , -9 , -8 , -7 , -6 , -5 , -4 , -3 , -2 , -1 , 
 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10 , 11 , 12 , 13 , 14 , 15 , 
 16 , 17 , 18 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 30 , 31 , 
 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 
 48 , 49 , 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 60 , 61 , 62 , 63 , 
 64 , 65 , 66 , 67 , 68 , 69 , 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 
 80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 90 , 91 , 92 , 93 , 94 , 95 , 
 96 , 97 , 98 , 99 , 100 , 101 , 102 , 103 , 104 , 105 , 106 , 107 , 108 , 109 , 110 , 111 , 
 112 , 113 , 114 , 115 , 116 , 117 , 118 , 119 , 120 , 121 , 122 , 123 , 124 , 125 , 126 , 127 
 };

 const char sinewave[256] = {
 0 , 3 , 6 , 9 , 12 , 15 , 18 , 21 , 24 , 27 , 30 , 33 , 36 , 39 , 42 , 45 , 
 48 , 51 , 54 , 57 , 59 , 62 , 65 , 67 , 70 , 73 , 75 , 78 , 80 , 82 , 85 , 87 , 
 89 , 91 , 94 , 96 , 98 , 100 , 102 , 103 , 105 , 107 , 108 , 110 , 112 , 113 , 114 , 116 , 
 117 , 118 , 119 , 120 , 121 , 122 , 123 , 123 , 124 , 125 , 125 , 126 , 126 , 126 , 126 , 126 , 
 127 , 126 , 126 , 126 , 126 , 126 , 125 , 125 , 124 , 123 , 123 , 122 , 121 , 120 , 119 , 118 , 
 117 , 116 , 114 , 113 , 112 , 110 , 108 , 107 , 105 , 103 , 102 , 100 , 98 , 96 , 94 , 91 , 
 89 , 87 , 85 , 82 , 80 , 78 , 75 , 73 , 70 , 67 , 65 , 62 , 59 , 57 , 54 , 51 , 
 48 , 45 , 42 , 39 , 36 , 33 , 30 , 27 , 24 , 21 , 18 , 15 , 12 , 9 , 6 , 3 , 
 0 , -3 , -6 , -9 , -12 , -15 , -18 , -21 , -24 , -27 , -30 , -33 , -36 , -39 , -42 , -45 , 
 -48 , -51 , -54 , -57 , -59 , -62 , -65 , -67 , -70 , -73 , -75 , -78 , -80 , -82 , -85 , -87 , 
 -89 , -91 , -94 , -96 , -98 , -100 , -102 , -103 , -105 , -107 , -108 , -110 , -112 , -113 , -114 , -116 , 
 -117 , -118 , -119 , -120 , -121 , -122 , -123 , -123 , -124 , -125 , -125 , -126 , -126 , -126 , -126 , -126 , 
 -127 , -126 , -126 , -126 , -126 , -126 , -125 , -125 , -124 , -123 , -123 , -122 , -121 , -120 , -119 , -118 , 
 -117 , -116 , -114 , -113 , -112 , -110 , -108 , -107 , -105 , -103 , -102 , -100 , -98 , -96 , -94 , -91 , 
 -89 , -87 , -85 , -82 , -80 , -78 , -75 , -73 , -70 , -67 , -65 , -62 , -59 , -57 , -54 , -51 , 
 -48 , -45 , -42 , -39 , -36 , -33 , -30 , -27 , -24 , -21 , -18 , -15 , -12 , -9 , -6 , -3 
 };
*/

 const uint16_t compressed_cents[1024] PROGMEM = {
   0, 15, 30, 44, 59, 74, 89, 104, 119, 134, 149, 165, 180, 195, 210, 225, 241, 256, 271, 287, 302, 318, 333, 349, 365, 380, 396, 412, 427, 443, 459, 475, 491, 507, 523, 539, 555, 571, 587, 603, 619, 636, 652, 668, 685, 701, 717, 734, 750, 767, 784, 800, 817, 834, 850, 867, 884, 901, 918, 935, 952, 969, 986, 1003, 1020, 1037, 1055, 1072, 1089, 1107, 1124, 1142, 1159, 1177, 1194, 1212, 1229, 1247, 1265, 1283, 1300, 1318, 1336, 1354, 1372, 1390, 1408, 1426, 1445, 1463, 1481, 1499, 1518, 1536, 1554, 1573, 1591, 1610, 1629, 1647, 1666, 1685, 1703, 1722, 1741, 1760, 1779, 1798, 1817, 1836, 1855, 1875, 1894, 1913, 1932, 1952, 1971, 1991, 2010, 2030, 2049, 2069, 2089, 2108, 2128, 2148, 2168, 2188, 2208, 2228, 2248, 2268, 2288, 2308, 2329, 2349, 2369, 2390, 2410, 2431, 2451, 2472, 2492, 2513, 2534, 2555, 2575, 2596, 2617, 2638, 2659, 2680, 2702, 2723, 2744, 2765, 2787, 2808, 2829, 2851, 2873, 2894, 2916, 2937, 2959, 2981, 3003, 3025, 3047, 3069, 3091, 3113, 3135, 3157, 3180, 3202, 3224, 3247, 3269, 3292, 3314, 3337, 3360, 3383, 3405, 3428, 3451, 3474, 3497, 3520, 3543, 3567, 3590, 3613, 3636, 3660, 3683, 3707, 3730, 3754, 3778, 3802, 3825, 3849, 3873, 3897, 3921, 3945, 3969, 3994, 4018, 4042, 4067, 4091, 4115, 4140, 4165, 4189, 4214, 4239, 4264, 4289, 4313, 4338, 4364, 4389, 4414, 4439, 4464, 4490, 4515, 4541, 4566, 4592, 4618, 4643, 4669, 4695, 4721, 4747, 4773, 4799, 4825, 4852, 4878, 4904, 4931, 4957, 4984, 5010, 5037, 5064, 5091, 5117, 5144, 5171, 5198, 5226, 5253, 5280, 5307, 5335, 5362, 5390, 5417, 5445, 5473, 5500, 5528, 5556, 5584, 5612, 5640, 5668, 5697, 5725, 5753, 5782, 5810, 5839, 5867, 5896, 5925, 5954, 5983, 6012, 6041, 6070, 6099, 6128, 6158, 6187, 6216, 6246, 6276, 6305, 6335, 6365, 6395, 6425, 6455, 6485, 6515, 6545, 6576, 6606, 6636, 6667, 6698, 6728, 6759, 6790, 6821, 6852, 6883, 6914, 6945, 6976, 7008, 7039, 7071, 7102, 7134, 7166, 7197, 7229, 7261, 7293, 7325, 7357, 7390, 7422, 7454, 7487, 7519, 7552, 7585, 7618, 7650, 7683, 7716, 7750, 7783, 7816, 7849, 7883, 7916, 7950, 7983, 8017, 8051, 8085, 8119, 8153, 8187, 8221, 8256, 8290, 8324, 8359, 8394, 8428, 8463, 8498, 8533, 8568, 8603, 8638, 8674, 8709, 8744, 8780, 8816, 8851, 8887, 8923, 8959, 8995, 9031, 9067, 9104, 9140, 9177, 9213, 9250, 9286, 9323, 9360, 9397, 9434, 9471, 9509, 9546, 9584, 9621, 9659, 9696, 9734, 9772, 9810, 9848, 9886, 9925, 9963, 10001, 10040, 10079, 10117, 10156, 10195, 10234, 10273, 10312, 10351, 10391, 10430, 10470, 10509, 10549, 10589, 10629, 10669, 10709, 10749, 10790, 10830, 10870, 10911, 10952, 10992, 11033, 11074, 11115, 11157, 11198, 11239, 11281, 11322, 11364, 11406, 11448, 11489, 11532, 11574, 11616, 11658, 11701, 11743, 11786, 11829, 11872, 11915, 11958, 12001, 12044, 12087, 12131, 12175, 12218, 12262, 12306, 12350, 12394, 12438, 12483, 12527, 12571, 12616, 12661, 12706, 12751, 12796, 12841, 12886, 12931, 12977, 13023, 13068, 13114, 13160, 13206, 13252, 13298, 13345, 13391, 13438, 13484, 13531, 13578, 13625, 13672, 13720, 13767, 13814, 13862, 13910, 13957, 14005, 14053, 14102, 14150, 14198, 14247, 14295, 14344, 14393, 14442, 14491, 14540, 14589, 14639, 14688, 14738, 14788, 14838, 14888, 14938, 14988, 15038, 15089, 15139, 15190, 15241, 15292, 15343, 15394, 15446, 15497, 15549, 15600, 15652, 15704, 15756, 15808, 15861, 15913, 15966, 16018, 16071, 16124, 16177, 16230, 16284, 16337, 16391, 16444, 16498, 16552, 16606, 16660, 16715, 16769, 16824, 16879, 16933, 16988, 17043, 17099, 17154, 17210, 17265, 17321, 17377, 17433, 17489, 17545, 17602, 17658, 17715, 17772, 17829, 17886, 17943, 18001, 18058, 18116, 18174, 18232, 18290, 18348, 18406, 18465, 18523, 18582, 18641, 18700, 18759, 18819, 18878, 18938, 18997, 19057, 19117, 19177, 19238, 19298, 19359, 19419, 19480, 19541, 19603, 19664, 19725, 19787, 19849, 19911, 19973, 20035, 20097, 20160, 20222, 20285, 20348, 20411, 20474, 20538, 20601, 20665, 20729, 20793, 20857, 20921, 20986, 21050, 21115, 21180, 21245, 21310, 21376, 21441, 21507, 21573, 21639, 21705, 21771, 21838, 21904, 21971, 22038, 22105, 22173, 22240, 22308, 22375, 22443, 22511, 22579, 22648, 22716, 22785, 22854, 22923, 22992, 23062, 23131, 23201, 23271, 23341, 23411, 23481, 23552, 23623, 23693, 23764, 23836, 23907, 23979, 24050, 24122, 24194, 24266, 24339, 24411, 24484, 24557, 24630, 24703, 24777, 24850, 24924, 24998, 25072, 25147, 25221, 25296, 25371, 25446, 25521, 25596, 25672, 25747, 25823, 25899, 25976, 26052, 26129, 26206, 26283, 26360, 26437, 26515, 26592, 26670, 26748, 26827, 26905, 26984, 27063, 27142, 27221, 27300, 27380, 27460, 27540, 27620, 27700, 27781, 27862, 27943, 28024, 28105, 28186, 28268, 28350, 28432, 28514, 28597, 28680, 28762, 28846, 28929, 29012, 29096, 29180, 29264, 29348, 29433, 29517, 29602, 29687, 29772, 29858, 29944, 30029, 30116, 30202, 30288, 30375, 30462, 30549, 30636, 30724, 30812, 30900, 30988, 31076, 31165, 31253, 31342, 31432, 31521, 31611, 31700, 31790, 31881, 31971, 32062, 32153, 32244, 32335, 32427, 32519, 32611, 32703, 32795, 32888, 32981, 33074, 33167, 33261, 33354, 33448, 33542, 33637, 33732, 33826, 33922, 34017, 34112, 34208, 34304, 34400, 34497, 34594, 34690, 34788, 34885, 34983, 35080, 35178, 35277, 35375, 35474, 35573, 35672, 35772, 35872, 35971, 36072, 36172, 36273, 36374, 36475, 36576, 36678, 36780, 36882, 36984, 37087, 37190, 37293, 37396, 37499, 37603, 37707, 37812, 37916, 38021, 38126, 38231, 38337, 38443, 38549, 38655, 38762, 38868, 38976, 39083, 39190, 39298, 39406, 39515, 39623, 39732, 39841, 39951, 40060, 40170, 40280, 40391, 40501, 40612, 40724, 40835, 40947, 41059, 41171, 41284, 41396, 41509, 41623, 41736, 41850, 41964, 42079, 42193, 42308, 42424, 42539, 42655, 42771, 42887, 43004, 43121, 43238, 43355, 43473, 43591, 43709, 43828, 43947, 44066, 44185, 44305, 44425, 44545, 44666, 44787, 44908, 45029, 45151, 45273, 45395, 45518, 45641, 45764, 45887, 46011, 46135, 46259, 46384, 46509, 46634, 46759, 46885, 47011, 47138, 47264, 47391, 47519, 47646, 47774, 47902, 48031, 48160, 48289, 48418, 48548, 48678, 48808, 48939, 49070, 49201, 49333, 49465, 49597, 49730, 49862, 49996, 50129, 50263, 50397, 50531, 50666, 50801, 50937, 51072, 51208, 51345, 51482, 51619, 51756, 51894, 52032, 52170, 52308, 52447, 52587, 52726, 52866, 53007, 53147, 53288, 53430, 53571, 53713, 53855, 53998, 54141, 54284, 54428, 54572, 54716, 54861, 55006, 55152, 55297, 55443, 55590, 55737, 55884, 56031, 56179, 56327, 56476, 56624, 56774, 56923, 57073, 57223, 57374, 57525, 57677, 57828, 57980, 58133, 58286, 58439, 58592, 58746, 58900, 59055, 59210, 59365, 59521, 59677, 59834, 59991, 60148, 60305, 60463, 60622, 60780, 60939, 61099, 61259, 61419, 61580, 61741, 61902, 62064, 62226, 62388, 62551, 62715, 62878, 63042, 63207, 63372, 63537, 63702, 63869, 64035, 64202
 };
#define DECOMPRESS_OFFSET 49799
#define DECOMPRESS_FACTOR 3

 unsigned char joyx, joyy, zbut, accx, accy;
 uint8_t ext_id[6];

ISR (TIMER0_COMPA_vect){
    accumulator += phase;
    
    outvalue = volume*((accumulator >> 31) ? 1 : -1) + 127;
    
    OCR1B = outvalue;
}

int main(void)
{  
    PLLCSR |= _BV(PLLE);               // Enable 64 MHz PLL
    _delay_ms(10);            // Stabilize
    while(!(PLLCSR & _BV(PLOCK)));     // Wait for it...
    PLLCSR |= _BV(PCKE);               // Timer1 source = PLL

    // Set up Timer/Counter1 for PWM output
    TIMSK  = 0;                        // Timer interrupts OFF
    TCCR1  = _BV(CS10);                // 1:1 prescale
    GTCCR  = _BV(PWM1B) | _BV(COM1B1); // PWM B, clear on match
    OCR1C  = 255;                      // Full 8-bit PWM cycle
    OCR1B  = 127;                      // 50% duty at start

    DDRB = _BV(PORTB4);                // Enable PWM output pin
    
    // INTERRUPT SETUP
    TCCR0B = _BV(CS01); 	//CLK_IO/8
    TCCR0A = _BV(WGM01);	// clear on match
    OCR0A = 40; // Approximately 24kHz sampling rate
    TIMSK = (1<<OCIE0A);
    
    // init the DDS phase increment
    // for a 32-bit DDS accumulator, running at Fclock:
    // phase = 2^32*Fout/Fclock (where Fclock is the refresh rate)
    // FIX MATH LATER
    // phase = (long)(167503.724544*660.0);    

    // Enable interrupts for sound generation
    sei();

    do{
      _delay_ms(100);
      nunchuck_init(ext_id);
    } while(ext_id[2] != 0xA4);
    //if(ext_id[0] == 0 && ext_id[1] == 0 && ext_id[2] == 0xA4 && ext_id[3] == 0x20 && ext_id[4] == 0x01 && ext_id[5] == 0x01)
    {
      for(;;){
        if(nunchuck_get_data()){
          if(nunchuck_zbutton() == 0){
            /* Read the "compressed" phase from memory,
             * multiply it by the division factor,
             * then add the offset back on,
             * and finally multiply by the central note (i.e. << 9 is multiplying by 512, approximately treble C)
             */
            phase = (((long)pgm_read_word(&compressed_cents[nunchuck_accelx() + (nunchuck_joyx() >> 2)  - 32]) << DECOMPRESS_FACTOR) + DECOMPRESS_OFFSET) << 9;
          }
          if(nunchuck_cbutton() == 0){
            volume = (255-75) - (nunchuck_accely() >> 2); // Reversed so that "down" is mute and "up" is loud
          }
        } else {
          volume = 0;
        }
      }
    };
    
    while(true){
      volume = 0;
    };
    
    return 0;   /* never reached */
}
