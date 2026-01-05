#include "image.h"

extern const unsigned char gImage_image_init[153600];
extern const unsigned char gImage_image_home[153600];

extern const unsigned char gImage_tmp_cold[3672];
extern const unsigned char gImage_tmp_hot[3672];
extern const unsigned char gImage_tmp_warm[3672];

extern const unsigned char gImage_we_sunny[5832];
extern const unsigned char gImage_we_snowy[5832];
extern const unsigned char gImage_we_cloudy[5832];
extern const unsigned char gImage_we_rainy[5832];
extern const unsigned char gImage_we_heavy_rain[5832];

extern const unsigned char gImage_we_late_night[5832];
extern const unsigned char gImage_we_night[5832];
extern const unsigned char gImage_we_unknown[5832];

extern const unsigned char gImage_wifi_no[1352];
extern const unsigned char gImage_wifi_yes[1352];

const image_t img_tmp_cold = {
	.width  = 34,
	.height = 54,
	.data   = gImage_tmp_cold,
};
const image_t img_tmp_hot = {
	.width  = 34,
	.height = 54,
	.data   = gImage_tmp_hot,
};
const image_t img_tmp_warm = {
	.width  = 34,
	.height = 54,
	.data   = gImage_tmp_warm,
};

const image_t img_we_sunny = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_sunny,
};
const image_t img_we_snowy = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_snowy,
};
const image_t img_we_cloudy = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_cloudy,
};
const image_t img_we_rainy = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_rainy,
};
const image_t img_we_heavy_rain = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_heavy_rain,
};
const image_t img_we_late_night = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_late_night,
};
const image_t img_we_night = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_night,
};
const image_t img_we_unknown = {
	.width  = 54,
	.height = 54,
	.data   = gImage_we_unknown,
};
const image_t img_wifi_no = {
	.width  = 26,
	.height = 26,
	.data   = gImage_wifi_no,
};
const image_t img_wifi_yes = {
	.width  = 26,
	.height = 26,
	.data   = gImage_wifi_yes,
};

const image_t img_init = {
	.width  = 240,
	.height = 320,
	.data   = gImage_image_init,
};
const image_t img_home = {
	.width  = 240,
	.height = 320,
	.data   = gImage_image_home,
};
