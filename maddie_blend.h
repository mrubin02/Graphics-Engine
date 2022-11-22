#include "GPixel.h"
#include "GColor.h"
#include "GRect.h"
#include "GBitmap.h"
#include "GBlendMode.h"
#include "GTypes.h"

GPixel zero = GPixel_PackARGB(0,0,0,0);

GPixel pack_color(GColor color){ 
    int a = round(color.a * 255); 
    int r = round(color.r * color.a  * 255); 
    int g = round(color.g * color.a  * 255); 
    int b = round(color.b * color.a  * 255); 
    GPixel pixelColor = GPixel_PackARGB(a, r, g,  b);
    return pixelColor;
}

unsigned div255(unsigned x){
    return (x* 65793 + (1<<23) ) >> 24;
}

int src_over(int s, int d, int sa){
    int x = div255((255-sa) * d); 
    return s + x;
}

int dst_over(int s, int d, int da){
    int x = div255((255-da)*s);
    return d + x; 
}

int src_(int s, int d){
    return s; 
}

int dst_(int s, int d){
    return d;
}

int src_in(int s, int da){
    return div255(s*da);
}

int dst_in(int d, int sa){
    return div255(sa*d);
}

int src_out(int s, int da){
    return div255(s*(255-da));
}

int dst_out(int d, int sa){
    return div255(d*(255-sa));
}

int src_top(int s, int d, int sa, int da){
    return div255(da*s) + div255((255-sa) * d);
}

int dst_top(int s, int d, int sa, int da){
    return div255(sa*d) + div255((255-da) * s);
}

int xor_(int s, int d, int sa, int da){
    return div255((255-sa)*d) + div255((255-da)*s);
}

GPixel kClear(GPixel src, GPixel dst){
    return GPixel_PackARGB(0, 0, 0, 0);
}

GPixel blend_srcover(GPixel src, GPixel dst){
    int sa = GPixel_GetA(src); 
    int r = src_over(GPixel_GetR(src), GPixel_GetR(dst), sa);
    int g = src_over(GPixel_GetG(src), GPixel_GetG(dst), sa);
    int b = src_over(GPixel_GetB(src), GPixel_GetB(dst), sa);
    int a = src_over(GPixel_GetA(src), GPixel_GetA(dst), sa);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_dstover(GPixel src, GPixel dst){
    int da = GPixel_GetA(dst); 
    int r = dst_over(GPixel_GetR(src), GPixel_GetR(dst), da);
    int g = dst_over(GPixel_GetG(src), GPixel_GetG(dst), da);
    int b = dst_over(GPixel_GetB(src), GPixel_GetB(dst), da);
    int a = dst_over(GPixel_GetA(src), GPixel_GetA(dst), da);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_src(GPixel src, GPixel dst){
    int r = src_(GPixel_GetR(src), GPixel_GetR(dst));
    int g = src_(GPixel_GetG(src), GPixel_GetG(dst));
    int b = src_(GPixel_GetB(src), GPixel_GetB(dst));
    int a = src_(GPixel_GetA(src), GPixel_GetA(dst));
    return GPixel_PackARGB(a, r, g,  b);
}


GPixel blend_dst(GPixel src, GPixel dst){
    int r = dst_(GPixel_GetR(src), GPixel_GetR(dst));
    int g = dst_(GPixel_GetG(src), GPixel_GetG(dst));
    int b = dst_(GPixel_GetB(src), GPixel_GetB(dst));
    int a = dst_(GPixel_GetA(src), GPixel_GetA(dst));
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_srcin(GPixel src, GPixel dst){
    int da = GPixel_GetA(dst); 
    int r = src_in(GPixel_GetR(src), da);
    int g = src_in(GPixel_GetG(src), da);
    int b = src_in(GPixel_GetB(src), da);
    int a = src_in(GPixel_GetA(src), da);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_dstin(GPixel src, GPixel dst){
    int sa = GPixel_GetA(src); 
    int r = dst_in(GPixel_GetR(dst), sa);
    int g = dst_in(GPixel_GetG(dst), sa);
    int b = dst_in(GPixel_GetB(dst), sa);
    int a = dst_in(GPixel_GetA(dst), sa);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_srcout(GPixel src, GPixel dst){
    int da = GPixel_GetA(dst); 
    int r = src_out(GPixel_GetR(src), da);
    int g = src_out(GPixel_GetG(src), da);
    int b = src_out(GPixel_GetB(src), da);
    int a = src_out(GPixel_GetA(src), da);
    return GPixel_PackARGB(a, r, g,  b);
}


GPixel blend_dstout(GPixel src, GPixel dst){
    int sa = GPixel_GetA(src); 
    int r = dst_out(GPixel_GetR(dst), sa);
    int g = dst_out(GPixel_GetG(dst), sa);
    int b = dst_out(GPixel_GetB(dst), sa);
    int a = dst_out(GPixel_GetA(dst), sa);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_srctop(GPixel src, GPixel dst){
    int sa = GPixel_GetA(src); 
    int da = GPixel_GetA(dst); 
    int r = src_top(GPixel_GetR(src), GPixel_GetR(dst), sa, da);
    int g = src_top(GPixel_GetG(src), GPixel_GetG(dst), sa, da);
    int b = src_top(GPixel_GetB(src), GPixel_GetB(dst), sa, da);
    int a = src_top(GPixel_GetA(src), GPixel_GetA(dst), sa, da);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_dsttop(GPixel src, GPixel dst){
    int sa = GPixel_GetA(src); 
    int da = GPixel_GetA(dst); 
    int r = dst_top(GPixel_GetR(src), GPixel_GetR(dst), sa, da);
    int g = dst_top(GPixel_GetG(src), GPixel_GetG(dst), sa, da);
    int b = dst_top(GPixel_GetB(src), GPixel_GetB(dst), sa, da);
    int a = dst_top(GPixel_GetA(src), GPixel_GetA(dst), sa, da);
    return GPixel_PackARGB(a, r, g,  b);
}

GPixel blend_xor(GPixel src, GPixel dst){
    int sa = GPixel_GetA(src); 
    int da = GPixel_GetA(dst); 
    int r = xor_(GPixel_GetR(src), GPixel_GetR(dst), sa, da);
    int g = xor_(GPixel_GetG(src), GPixel_GetG(dst), sa, da);
    int b = xor_(GPixel_GetB(src), GPixel_GetB(dst), sa, da);
    int a = xor_(GPixel_GetA(src), GPixel_GetA(dst), sa, da);
    return GPixel_PackARGB(a, r, g,  b);
}


GPixel call_blend(GBlendMode mode, GPixel src, GPixel dst){
    float dst_A = GPixel_GetA(dst);
    if (dst_A == 255){
        if(mode==GBlendMode::kDstOver){return dst;}
        if(mode==GBlendMode::kSrcIn){return src;}
        if(mode==GBlendMode::kSrcOut){return zero;}
        if(mode==GBlendMode::kSrcATop){return blend_srcover(src,dst);}
        if(mode==GBlendMode::kDstATop){return blend_dstin(src,dst);}
        if(mode==GBlendMode::kXor){return blend_dstout(src,dst);}
    }
    if(dst_A == 0){
        if(mode==GBlendMode::kDstOver){return src;}
        if(mode==GBlendMode::kSrcOver){return src;}
        if(mode==GBlendMode::kDst){return zero;}
        if(mode==GBlendMode::kSrcIn){return zero;}
        if(mode==GBlendMode::kDstIn){return zero;}
        if(mode==GBlendMode::kDstOut){return zero;}
        if(mode==GBlendMode::kSrcOut){return src;}
        if(mode==GBlendMode::kSrcATop){return zero;}
        if(mode==GBlendMode::kDstATop){return src;}
        if(mode==GBlendMode::kXor){return src;}
    }

    switch(mode){
        case GBlendMode::kClear:
            return zero;
        case GBlendMode::kSrc:
            return src;
        case GBlendMode::kDst:
            return dst;
        case GBlendMode::kSrcOver:
            return blend_srcover(src,dst);
        case GBlendMode::kDstOver:
            return blend_dstover(src,dst);
        case GBlendMode::kSrcIn:
            return blend_srcin(src,dst);
        case GBlendMode::kDstIn:
            return blend_dstin(src,dst);
        case GBlendMode::kSrcOut:
            return blend_srcout(src,dst);
        case GBlendMode::kDstOut:
            return blend_dstout(src,dst);
        case GBlendMode::kSrcATop:
            return blend_srctop(src,dst);
        case GBlendMode::kDstATop:
            return blend_dsttop(src,dst);
        case GBlendMode::kXor:
            return blend_xor(src,dst);
    }
}

/* change 1 alpha to 255, new function to simplify blend mode before loop*/
GBlendMode simple_blend(const GBlendMode mode, float src_A){
    
    if(src_A ==1){
        if(mode==GBlendMode::kSrcOver){return GBlendMode::kSrc;}
        if(mode==GBlendMode::kDstIn){return GBlendMode::kDst;}
        if(mode==GBlendMode::kDstOut){return GBlendMode::kClear;}
        if(mode==GBlendMode::kSrcATop){return GBlendMode::kSrcIn;}
        if(mode==GBlendMode::kDstATop){return GBlendMode::kDstOver;}
        if(mode==GBlendMode::kXor){return GBlendMode::kSrcOut;}
    }
    if(src_A == 0){
        if(mode==GBlendMode::kSrcOver){return GBlendMode::kDst;}
        if(mode==GBlendMode::kDstOver){return GBlendMode::kDst;}
        if(mode==GBlendMode::kSrc){return GBlendMode::kClear;}
        if(mode==GBlendMode::kSrcIn){return GBlendMode::kClear;}
        if(mode==GBlendMode::kDstIn){return GBlendMode::kClear;}
        if(mode==GBlendMode::kSrcOut){return GBlendMode::kClear;}
        if(mode==GBlendMode::kDstOut){return GBlendMode::kDst;}
        if(mode==GBlendMode::kSrcATop){return GBlendMode::kDst;}
        if(mode==GBlendMode::kDstATop){return GBlendMode::kClear;}
        if(mode==GBlendMode::kXor){return GBlendMode::kDst;}
    }
    return mode;
}

