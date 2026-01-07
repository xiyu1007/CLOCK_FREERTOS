#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct
{
    const char *name;
    const uint8_t *model;
} font_chinese_t;

typedef struct
{
    uint16_t size;
    const uint8_t *ascii_model;
    const font_chinese_t *chinese;
} font_t;

extern const font_t font16;
extern const font_t font22;
extern const font_t font24;
extern const font_t font32;
extern const font_t font44;
extern const font_t font48;


// maple: semibold/ bold
// 16 maple
// 16 maple bold
// 24 maple bold
// 32 maple bold
// 54 maple bold
// 54 maple semibold
// 64 maple extrablod
// 76 maple extrablod
#endif /* __FONT_H__ */
