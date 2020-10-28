#include "EasyColor.h"
EasyColor::EasyColor()
{

}

EasyColor::HSVRGB::HSVRGB()
{

}

EasyColor::CMYKRGB::CMYKRGB()
{

}

EasyColor::HSLRGB::HSLRGB()
{

}

long EasyColor::HSVRGB::map(long x, long in_min, long in_max, long out_min, long out_max)
{
    long divisor = (in_max - in_min);
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return (x - in_min) * (out_max - out_min) / divisor + out_min;
}

rgb EasyColor::HSVRGB::HSVtoRGB(hsv in, rgb out)
{
    if (in.h > 360) in.h -= 360;
    if (in.h < 0)   in.h += 360;

    in.h = constrain(in.h, 0, 360);
    in.s = constrain((in.s/100.0), 0, 1); //changed to work with LVGL
    in.v = constrain((in.v/100.0), 0, 1); //changed to work with LVGL

    float c = in.v * in.s;
    float x = c * (1 - fabsf (fmod ((in.h / HUE_ANGLE), 2) - 1));
    float m = in.v - c;
    float rp, gp, bp;
  
    int a = in.h / 60;


    switch (a){
        case 0:
            rp = c;
            gp = x;
            bp = 0;
            break;

        case 1:
            rp = x;
            gp = c;
            bp = 0;
            break;

        case 2:
            rp = 0;
            gp = c;
            bp = x;
            break;

        case 3:
            rp = 0;
            gp = x;
            bp = c;
            break;

        case 4:
            rp = x;
            gp = 0;
            bp = c;
            break;

        default:
            rp = c;
            gp = 0;
            bp = x;
            break;
    }

    out.r = (rp + m) * 255;
    out.g = (gp + m) * 255;
    out.b = (bp + m) * 255;
  
    Serial.println(out.r);
    Serial.println(out.g);
    Serial.println(out.b);
    return out;
}

hsv EasyColor::HSVRGB::RGBtoHSV(rgb in, hsv out)
{
    float max_rgb  = max(max(in.r, in.g), in.b);
    float rgb_min  = min(min(in.r, in.g), in.b);
    float deltaRGB = max_rgb - rgb_min;
  
    if(deltaRGB > 0) {
        if(max_rgb == in.r) {
            out.h = 60 * (fmod(((in.g - in.b) / deltaRGB), 6));
        }
        else if(max_rgb == in.g) {
            out.h = 60 * (((in.b - in.r) / deltaRGB) + 2);
        }
        else if(max_rgb == in.b) {
            out.h = 60 * (((in.r - in.g) / deltaRGB) + 4);
        }
    
        if(max_rgb > 0) {
            out.s = (deltaRGB / max_rgb) * 100;
        }
        else {
            out.s = 0;
        }
    
        out.v = map(max_rgb,0,255,0,100);
        }
        else {
            out.h = 0;
            out.s = 0;
            out.v = map(max_rgb,0,255,0,100);
    }
  
    if(out.h < 0) {
        out.h = 360 + out.h;
    }
    return out;
}

rgb EasyColor::CMYKRGB::CMYKtoRGB(cmyk in, rgb out)
{
    /*
    A reversa também é simples e será utilizada para manipular o CMYK direto no display.
    O RGB é inteiro, mas o CMYK precisa ser passado de 0 à 1 novamente. No artigo tem uma
    imagem de exemplo do cálculo na calculadora, mas segue um exemplo:
    C = 43
    M = 30
    Y = 10
    K = 10
    R = 255 * (1-(43/100))*(1-(10/100)) = 130,815 ; arredondar para cima quando > 0.5
    ==================================
    The R,G,B values are given in the range of 0..255.
    The red (R) color is calculated from the cyan (C) and black (K) colors:
    R = 255 × (1-C) × (1-K)
    The green color (G) is calculated from the magenta (M) and black (K) colors:
    G = 255 × (1-M) × (1-K)
    The blue color (B) is calculated from the yellow (Y) and black (K) colors:
    B = 255 × (1-Y) × (1-K)
    */
    //uint8_t exemplo = round(255.0 * (1.0-((float)43/100.0))*(1.0-((float)10/100.0)));
    out.r = round(RGB_MAX * (ONE_DOT-((float)in.c/HUNDRED)) * (ONE_DOT-((float)in.k/HUNDRED)));
    out.g = round(RGB_MAX * (ONE_DOT-((float)in.m/HUNDRED)) * (ONE_DOT-((float)in.k/HUNDRED)));
    out.b = round(RGB_MAX * (ONE_DOT-((float)in.y/HUNDRED)) * (ONE_DOT-((float)in.k/HUNDRED)));

    return out;
}

cmyk EasyColor::CMYKRGB::RGBtoCMYK(rgb in, cmyk out)
{
    float Rfrac = (float)in.r/(float)255;
    float Gfrac = (float)in.g/(float)255;
    float Bfrac = (float)in.b/(float)255;

    float K = 1-max({Rfrac,Gfrac,Bfrac});

    float C = (1-Rfrac-K)/(1-K);
    float M = (1-Gfrac-K)/(1-K);
    float Y = (1-Bfrac-K)/(1-K);

    out.c = C*100;
    out.m = M*100;
    out.y = Y*100;
    out.k = K*100;

    return out;
}

//HSL in range 0-1. Returns RGB888
rgb EasyColor::HSLRGB::HSLtoRGB(hsl in, rgb out)
{
    double v;
    double R,G,B;

    R = in.l; //it's 'L', not 1
    G = in.l;
    B = in.l;

    v = (in.l <= 0.5) ? (in.l * (1.0 + in.s)) : (in.l + in.s - in.l * in.s);

    if (v > 0){
        double m;
        double sv;
        int sextant;
        double fract, vsf, mid1, mid2;

        m       = in.l + in.l - v;
        sv      = (v - m ) / v;
        in.h   *= 6.0;
        sextant = (int)in.h;
        fract   = in.h - sextant;
        vsf     = v * sv * fract;
        mid1    = m + vsf;
        mid2    = v - vsf;

        switch (sextant){
            case 0:
                R = v;
                G = mid1;
                B = m;
                break;
            case 1:
                R = mid2;
                G = v;
                B = m;
                break;
            case 2:
                R = m;
                G = v;
                B = mid1;
                break;
            case 3:
                R = m;
                G = mid2;
                B = v;
                break;
            case 4:
                R = mid1;
                G = m;
                B = v;
                break;
            case 5:
                R = v;
                G = m;
                B = mid2;
                break;
        }
    }
    out.r = (uint8_t) R*255.0;
    out.g = (uint8_t) G*255.0;
    out.b = (uint8_t) B*255.0;

    return out;

}
//RGB888. Returns HSL in range 0-1
hsl EasyColor::HSLRGB::RGBtoHSL(rgb in, hsl out)
{
    double r = in.r/255.0;
    double g = in.g/255.0;
    double b = in.g/255.0;
    
    double v;
    double m;
    double vm;
    double r2, g2, b2;

    out.h = 0; // default to black
    out.s = 0;
    out.l = 0;

    v = max(r,g);
    v = max(v,b);
    m = min(r,g);
    m = min(m,b);

    out.l = (m + v) / 2.0;

    if (out.l <= 0.0){
        return out;
    }

    vm = v - m;
    out.s = vm;

    if (out.s > 0.0){
        out.s /= (out.l <= 0.5) ? (v + m ) : (2.0 - v - m) ;
    }
    else{
        return out;
    }

    r2 = (v - in.r) / vm;
    g2 = (v - in.g) / vm;
    b2 = (v - in.b) / vm;

    if (in.r == v){
        out.h = (in.g == m ? 5.0 + b2 : 1.0 - g2);
    }
    else if (in.g == v){
        out.h = (in.b == m ? 1.0 + r2 : 3.0 - b2);
    }
    else{
        out.h = (in.r == m ? 3.0 + g2 : 5.0 - r2);
    }
    out.h /= 6.0;
    return out;
}


uint16_t EasyColor::RGB24toRGB16(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t blue  = (uint16_t)((b >> 3) & 0x1FU); //8 - 3 = 5
    uint8_t green = (uint16_t)((g >> 2) & 0x3FU); //8 - 2 = 6
    uint8_t red   = (uint16_t)((r >> 3) & 0x1FU); //8 - 3 = 5
    return (red << (16-5)) | (green << (16-(5+6))) | blue << (16-(5+6+5));
}

rgb EasyColor::RGB16toRGB24(uint16_t RGB16)
{
    rgb out;
    out.r = (RGB16 & 0b1111100000000000) >> 8;
    out.g = (RGB16 & 0b11111100000) >> 3;
    out.b = (RGB16 & 0b11111) << 3;
    return out; 
}