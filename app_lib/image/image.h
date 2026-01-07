#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>

typedef struct
{
	uint16_t       width;
	uint16_t       height;
	const uint8_t* data;
} image_t;

extern const image_t img_init;
extern const image_t img_home;
extern const image_t img_tmp_cold;
extern const image_t img_tmp_hot;
extern const image_t img_tmp_warm;
extern const image_t img_we_sunny;
extern const image_t img_we_snowy;
extern const image_t img_we_cloudy;
extern const image_t img_we_rainy;
extern const image_t img_we_heavy_rain;
extern const image_t img_we_late_night;
extern const image_t img_we_night;
extern const image_t img_we_unknown;
extern const image_t img_wifi_no;
extern const image_t img_wifi_yes;

#endif /* __IMAGE_H__ */
