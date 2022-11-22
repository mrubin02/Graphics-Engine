#include "GMatrix.h"
#include "GColor.h"
#include "GMath.h"
#include "GPoint.h"
#include "GRect.h"
#include "GShader.h"

float div255_2(unsigned x){
    return (x* 65793 + (1<<23) ) >> 24;
}

GPixel pack_C(GColor color){ 
    int a = round(color.a * 255); 
    int r = round(color.r * color.a  * 255); 
    int g = round(color.g * color.a  * 255); 
    int b = round(color.b * color.a  * 255); 
    GPixel pixelColor = GPixel_PackARGB(a, r, g,  b);
    return pixelColor;
}

GPixel mult(GPixel pix, GColor color){
    int a = round(color.a * 255); 
    int r = round(color.r * color.a  * 255); 
    int g = round(color.g * color.a  * 255); 
    int b = round(color.b * color.a  * 255); 
    
    a = div255_2(a * GPixel_GetA(pix));
    r = div255_2(r * GPixel_GetR(pix));
    g = div255_2(g * GPixel_GetG(pix));
    b = div255_2(b * GPixel_GetB(pix));

    return GPixel_PackARGB(a,r,g,b);
}


class MaddieTexProxy : public GShader {
    GShader* fRealShader;
    GMatrix fExtraTransform;
    public:
        MaddieTexProxy(GShader* originalShader, GMatrix& extraTransform) : fRealShader(originalShader), fExtraTransform(extraTransform){}
        GMatrix fInv;
        bool isOpaque() override{
            return fRealShader->isOpaque();
        }

        bool setContext(const GMatrix& ctm) override{
            return fRealShader->setContext(ctm*fExtraTransform);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override{
            fRealShader->shadeRow(x,y,count,row);
        }
};

class MaddieColorProxy : public GShader {
    GPoint* pts;
    GColor* colors;
    GMatrix M;
    public:
        MaddieColorProxy(GPoint pts[], GColor colors[], GMatrix M) : pts(pts), colors(colors), M(M){}
        GMatrix fInv;
        bool isOpaque() override{
            for(int  i = 0; i<3; i++){
                if(colors[i].a != 1){return false;}
            }
            return true;
        }

        bool setContext(const GMatrix& ctm) override{
            return (ctm*M).invert(&fInv);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override{
            GColor DC[3];
            DC[1] = colors[1] - colors[0];
            DC[2] = colors[2] - colors[0];
            DC[0] = fInv[0] * DC[1] + fInv[3] * DC[2];

            GPoint p = {x+0.5, y+0.5};
            GPoint p_prime = fInv * p; 
            GColor C = p_prime.x() * DC[1] + p_prime.y() * DC[2] + colors[0];
            for(int i =0; i< count; i++){
                row[i] = pack_C(C);
                C+=DC[0];
            }
        }
};

class MaddieTexColorProxy : public GShader {
    GPoint* pts;
    GColor* colors;
    GMatrix extraTransform;
    GMatrix M;
    GShader* originalShader;
    GBitmap fDevice;
    public:
        MaddieTexColorProxy(GBitmap fDevice, GPoint pts[], GColor colors[], GMatrix& extraTransform, GMatrix M, GShader* originalShader) : pts(pts), colors(colors), extraTransform(extraTransform), M(M), originalShader(originalShader), fDevice(fDevice){}
        GMatrix fInv;
        bool isOpaque() override{
            for(int  i = 0; i<3; i++){
                if(colors[i].a != 1){return false;}
            }
            return originalShader->isOpaque();
        }

        bool setContext(const GMatrix& ctm) override{
            (ctm*M).invert(&fInv);
            return originalShader->setContext(ctm*extraTransform);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override{
            
            originalShader->shadeRow(x,y,count,row);

            GColor DC[3];
            DC[1] = colors[1] - colors[0];
            DC[2] = colors[2] - colors[0];
            DC[0] = fInv[0] * DC[1] + fInv[3] * DC[2];

            GPoint p = {x+0.5, y+0.5};
            GPoint p_primeM = fInv * p; 
            GColor C = p_primeM.x() * DC[1] + p_primeM.y() * DC[2] + colors[0];
            for(int i =0; i< count; i++){
                row[i] = mult(row[i], C);
                C+=DC[0];
            }
        }
};



GMatrix compute_basis(GPoint pts[]){
    float Ux = pts[1].x() - pts[0].x();
    float Uy = pts[1].y() - pts[0].y();
    float Vx = pts[2].x() - pts[0].x();
    float Vy = pts[2].y() - pts[0].y();
    return {Ux, Vx, pts[0].x(), Uy, Vy, pts[0].y()};
}


void drawMeshColor(GCanvas* canvas, GPoint pts[], GColor colors[], GShader* originalShader){
    GMatrix M, invM; 
    M = compute_basis(pts);
    MaddieColorProxy proxy(pts, colors, M);
    GPaint p(&proxy);
    canvas->drawConvexPolygon(pts,3, p);
}

void drawMeshTex(GCanvas* canvas, GPoint pts[], GPoint tex[], GShader* originalShader){
    GMatrix P, T, invT;
    P = compute_basis(pts);
    T = compute_basis(tex);
    T.invert(&invT);
    GMatrix transform = P*invT;
    MaddieTexProxy proxy(originalShader, transform);
    GPaint p(&proxy);
    canvas->drawConvexPolygon(pts,3, p);
}

void drawMeshTexColor(GBitmap fDevice, GCanvas* canvas, GPoint pts[], GColor colors[], GPoint tex[], GShader* originalShader){
    GMatrix P, T, invT;
    P = compute_basis(pts);
    T = compute_basis(tex);
    T.invert(&invT);
    GMatrix transform = P*invT;
    MaddieTexColorProxy proxy(fDevice, pts, colors, transform, P, originalShader);
    GPaint p(&proxy);
    canvas->drawConvexPolygon(pts,3, p);


}