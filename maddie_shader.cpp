#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include "GPaint.h"
#include "GPixel.h"


GPixel pack(GColor color){ 
    int a = round(color.a * 255); 
    int r = round(color.r * color.a  * 255); 
    int g = round(color.g * color.a  * 255); 
    int b = round(color.b * color.a  * 255); 
    GPixel pixelColor = GPixel_PackARGB(a, r, g,  b);
    return pixelColor;
}

float tile(float xy, GShader::TileMode mode){
    if (mode == GShader::kClamp){
        if (xy<0){return 0;}
        if (xy>1){return 1;}
    }
    if (mode == GShader::kRepeat){
        return xy - floor(xy);
    }
    if (mode == GShader::kMirror){
        xy*=0.5;
        xy = xy-floor(xy);
        xy*=2;
        if(xy<1){return xy-floor(xy);}
        else{
            xy = 1-xy;
            return xy-floor(xy);
        }
    }
    return xy;
}


class MaddieShader : public GShader {
    const GBitmap bm; 
    const GMatrix LM; 
    const GShader::TileMode tmode;
    public: 
        
        MaddieShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tmode) : bm(bitmap), LM(localMatrix), tmode(tmode) {}
        GMatrix fInv;
        bool isOpaque() override{
            return bm.isOpaque();
        }
        
        bool setContext(const GMatrix& ctm) override{
            GMatrix WH = {bm.width(), 0, 0, 0, bm.height(), 0};
            return (ctm*LM*WH).invert(&fInv);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override{
            
            for(int i =0; i<count; i++){
                GPoint p = {x+0.5+i, y+0.5};
                GPoint p_prime = fInv*p;
                
                p_prime.fX = tile(p_prime.x(), tmode);
                p_prime.fY = tile(p_prime.y(), tmode);

                p_prime.fX = GFloorToInt(p_prime.x() * bm.width());
                p_prime.fY = GFloorToInt(p_prime.y() * bm.height());

                GPixel* pixel_addr = bm.getAddr(p_prime.x(), p_prime.y());
                row[i] = *pixel_addr;
            }
        }
};

class MaddieGradient : public GShader {
    public:
        MaddieGradient(GPoint p0, GPoint p1, const GColor color[], int count, GShader::TileMode tmode) : counts(count),
        p0(p0), p1(p1), tmode(tmode) {
            for(int i =0; i<counts;i++){
                colors.push_back(color[i]);
            }
        }

        GMatrix transform(GPoint p0, GPoint p1){
            float dx = p1.x() - p0.x();
            float dy = p1.y() - p0.y();
            return {dx, -dy, p0.x(), dy, dx, p0.y()};
        }
        bool isOpaque() override{
            for(int  i = 0; i<counts; i++){
                if(colors[i].a != 1){return false;}
            }
            return true;
        }

        GMatrix fInv;
        bool setContext(const GMatrix& ctm) override{
            GMatrix M = transform(p0, p1);
            return (ctm*M).invert(&fInv);
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override{
            if(counts<1){return;}
            if(counts == 1){
                for(int i =0; i<count; i++){
                    row[i] = pack(colors[0]);
                }
            }
            GColor DC[counts];
            for(int i =0; i<counts; i++){
                DC[i] = colors[i+1] - colors[i];
                if (i == counts-1){
                    DC[i] = colors[i];
                }
            }
            
            GPoint p = {x+0.5, y+0.5};
            GPoint p_prime = fInv * p; 
            
            GPoint pt;

            if(counts == 2){
                for(int i = 0; i < count; i++){
                    pt = p_prime;

                    pt.fX = tile(pt.x(), tmode);

                    GColor c = this->colors[0] + pt.x()*DC[0];
                    row[i] = pack(c);
                    p_prime.fX += fInv[0];
                }
                return;
            }

            float d;
            for(int i = 0; i<count; i++){  
                pt = p_prime;
                pt.fX = tile(pt.x(), tmode);
                d = pt.x() * (counts-1);
                int index = GFloorToInt(d);
                float w = d - index;
                GColor c = this->colors[index] + w*DC[index];
                row[i] = pack(c);
                p_prime.fX += fInv[0];
            }

        }
    private:
        const int counts; 
        GPoint p0;
        GPoint p1;
        std::vector<GColor> colors;
        GShader::TileMode tmode;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& device, const GMatrix& localMatrix, GShader::TileMode TileMode){
    return std::unique_ptr<GShader>(new MaddieShader(device, localMatrix, TileMode));
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode TileMode){
    return std::unique_ptr<GShader> (new MaddieGradient(p0, p1, color, count, TileMode));
}


