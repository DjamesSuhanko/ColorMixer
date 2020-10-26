/* Nested classes in EasyColor class. Check it.
Formulas reference: https://www.rapidtables.com/convert/color/
*/
#include <Arduino.h>
#include <math.h>

/*
extern "C" {
#include <stdlib.h>
#include "esp_system.h"
}
*/
#define EASY_COLOR_MAKE(r8, g8, b8) ((easy_color_t){{(uint16_t)((b8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x3FU), (uint16_t)((r8 >> 3) & 0x1FU)}})
#define HUE_ANGLE 360
#define RGB_MAX   255.0
#define ONE_DOT   1.0
#define HUNDRED   100.0

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb565;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

typedef struct {
    int c;
    int m;
    int y;
    int k;
} cmyk;

typedef struct {
    double h;
    double s;
    double v;
} hsv;

typedef struct {
    double h;
    double s;
    double l;
} hsl;

class EasyColor{
    public:
        EasyColor();

        uint16_t RGB24toRGB16(uint8_t r, uint8_t g, uint8_t b);
        rgb RGB16toRGB24(uint16_t RGB16);

        class HSVRGB{
            public:
                HSVRGB();

                long map(long x, long in_min, long in_max, long out_min, long out_max);
               
                rgb HSVtoRGB(hsv in, rgb out);
                hsv RGBtoHSV(rgb in, hsv out);
        };

        class CMYKRGB{
            public:
                CMYKRGB();

                rgb CMYKtoRGB(cmyk in, rgb out);
                cmyk RGBtoCMYK(rgb in, cmyk out);
        };

        class HSLRGB{
            public:
                HSLRGB();

                rgb HSLtoRGB(hsl in, rgb out);
                hsl RGBtoHSL(rgb in, hsl out);
        };
};