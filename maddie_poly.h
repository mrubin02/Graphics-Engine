#include "GPoint.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GBlendMode.h"
#include "GShader.h"
#include "GPath.h"

struct edge{
    int top; 
    int bottom;
    float m; 
    float b; 
    int getX(int y); 
    bool init = false;
    int w;
};

GPoint eval_quad(float t, const GPoint src[3]){
    GPoint Pt =   (1-t)*(1-t)*src[0] + 2*src[1]*t*(1-t) + t*t*src[2];
    return Pt;
}


GPoint eval_cubic(float t, const GPoint src[4]){
    GPoint ABt =  (1-t)*src[0] + t*src[1];
    GPoint BCt =  (1-t)*src[1] + t*src[2];
    GPoint CDt =  (1-t)*src[2] + t*src[3];

    GPoint ABCt =  (1-t)*ABt+ t*BCt;
    GPoint BCDt =  (1-t)*BCt+ t*CDt;
    GPoint Pt =    (1-t)*ABCt + t*BCDt;

    return Pt;
}

std::vector<GPoint> recurse_quad(int i, int n, const GPoint src[], int height){
    std::vector<GPoint> ret;
    if(true){
        ret.push_back(src[0]);
        float dt = 1.0/n;
        float t = dt;
        for(int i = 0; i<n-1; i++){
            GPoint pt = eval_quad(t, src);
            ret.push_back(pt);
            t += dt;
        }
        ret.push_back(src[2]);
        return ret;
    }
    GPoint pts[5];
    GPath::ChopQuadAt(src, pts, 0.5);
    GPoint left[] = {pts[0], pts[1], pts[2]};
    GPoint right[] = {pts[2], pts[3], pts[4]};
    i+=1;
    if (i == n){
        std::vector<GPoint> points(pts+0, pts+5);
        return points;
    }
    std::vector<GPoint> ret_l;
    std::vector<GPoint> ret_r;

    if(!(left[-1].y()<0 && left[0].y() <0) && !(left[-1].y()>height && left[0].y()>height)){
        ret_l= recurse_quad(i, n, left, height);
    }

    if(!(right[-1].y()<0 && right[0].y() <0) && !(right[-1].y()>height && right[0].y()>height)){
        ret_r = recurse_quad(i, n, right, height);
    }

    ret_l.insert(ret_l.end(), ret_r.begin() + 1, ret_r.end());
    
    return ret_l;
}

std::vector<GPoint> recurse_cubic(int i, int n, const GPoint src[4], int height){
    
    
    std::vector<GPoint> ret;
    if(true){
        ret.push_back(src[0]);
        float dt = 1.0/n;
        float t = dt;
        for(int i = 0; i<n-1; i++){
            GPoint pt = eval_cubic(t, src);
            ret.push_back(pt);
            t += dt;
        }
        ret.push_back(src[3]);
        return ret;
    }
    
    GPoint pts[7];
    GPath::ChopCubicAt(src, pts, 0.5);
    GPoint left[4] = {pts[0], pts[1], pts[2],pts[3]};
    GPoint right[4] = {pts[3], pts[4], pts[5],pts[6]};
    i+=1;
    if (i == n){
        std::vector<GPoint> points(pts+0, pts+7);
        return points;
    }

    std::vector<GPoint> ret_l;
    std::vector<GPoint> ret_r;

    
    if(!(left[-1].y()<0 && left[0].y() <0) && !(left[-1].y()>height && left[0].y()>height)){
        ret_l= recurse_cubic(i, n, left, height);
    }

    if(!(right[-1].y()<0 && right[0].y() <0) && !(right[-1].y()>height && right[0].y()>height)){
        ret_r = recurse_cubic(i, n, right, height);
    }

    ret_l.insert(ret_l.end(), ret_r.begin() + 1, ret_r.end());
    
    return ret_l;
}


int edge::getX(int y){return round(m*(y+0.5) + b);}

int find_intersect(edge add1, int clip, int y){
    float t1_h = abs(add1.bottom - add1.top);
    float t1_b = abs(add1.getX(add1.bottom) - add1.getX(add1.top));
    float t2_b = abs(clip - add1.getX(y));
    float t2_h = t1_h * t2_b/t1_b;
    return t2_h;
}

struct edge chop_vert(struct edge add1, int height){
    if(add1.top < 0){
        add1.top = 0; 
    }
    if(round(add1.bottom) >= height){
        add1.bottom = height;
    }
    return add1;
}

struct edge proj_horiz(int width, struct edge add1){
    if(add1.getX(add1.top)>=width && add1.getX(add1.bottom)>=width){
        add1.m = 0;
        add1.b = width;
    }
    if(round(add1.getX(add1.bottom)) <= 0 && round(add1.getX(add1.top))<=0) {
        add1.m = 0; 
        add1.b = 0;
    }
    return add1;
}

struct edge* chop_left(struct edge add1, struct edge add2){
    add2.init = true;
    add2.m = 0; 
    add2.b = 0; 
    add2.w = add1.w;
    if (add1.m<0){
        int y = round(find_intersect(add1, 1, add1.bottom)); 
        add2.top = add1.bottom - y; 
        add2.bottom = add1.bottom;
        add1.bottom = add1.bottom - y;
    }else{
        int y = round(find_intersect(add1, 1, add1.top)); 
        add2.top = add1.top;
        add2.bottom = add1.top + y; 
        add1.top = add1.top + y;
    }
    struct edge* ret; 
    ret = new edge[2]; 
    ret[0] = add1;
    ret[1] = add2;
    return ret;
}

struct edge* chop_right(struct edge add1, struct edge add3, int width){
    add3.init = true;
    add3.m = 0; 
    add3.b = width; 
    add3.w = add1.w;
    if (add1.m<0){
        int y = round(find_intersect(add1, width, add1.top)); 
        add3.top = add1.top; 
        add3.bottom = add1.top + y;
        add1.top = add1.top+y;
    }else{
        int y = round(find_intersect(add1, width, add1.bottom)); 
        add3.top = add1.bottom-y;
        add3.bottom = add1.bottom; 
        add1.bottom = add1.bottom-y;
    }
    struct edge* ret;
    ret = new edge[2]; 
    ret[0] = add1; 
    ret[1] = add3;
    return ret;
}


struct edge* clip_poly(GPoint& p0, GPoint& p1, int width, int height){
    struct edge add1;
    struct edge add2; 
    struct edge add3; 
    struct edge* edges;
    
    add1.w = 1;

    if(p0.y() > p1.y()){
        add1.w = -1;
        std::swap(p0, p1);
    }

    if((round(p1.y()) <= 0) || (round(p0.y()) >= height) 
    || (round(p0.y()) == round(p1.y())) 
    || (p1.y() == 0 && p0.y() == 0 && p0.x() == 0 && p1.x() ==0)){
        add1.init = false;
    }
    else{
        add1.init = true;
    }

    if(add1.init){
        add1.top = round(p0.y()); 
        add1.bottom = round(p1.y()); 
        add1.m = (p1.x()-p0.x())/(p1.y() - p0.y());
        add1.b =  p0.x()-(p0.y()*add1.m); 
        add1 = chop_vert(add1, height);
        add1 = proj_horiz(width, add1);


        if(add1.getX(add1.top)<0 && add1.getX(add1.bottom)>0
        || (add1.getX(add1.bottom)<0 && add1.getX(add1.top)>0)){
            struct edge* adds = chop_left(add1, add2);
            add1 = adds[0];
            add2 = adds[1];
        }

        if((add1.getX(add1.top)>width && add1.getX(add1.bottom) <= width)
        || (add1.getX(add1.bottom)>width && add1.getX(add1.top) <= width)){
            struct edge* adds = chop_right(add1,add3,width);
            add1 = adds[0];
            add3 = adds[1];
        }
    }
    edges = new struct edge[3];
    edges[0] = add1;
    edges[1] = add2; 
    edges[2] = add3;
    return edges;
}

bool sort_edge(struct edge e1, struct edge e2){
    if(e1.top!=e2.top){
        return e1.top<e2.top;
    }
    if(e1.getX(e1.top)!= e2.getX(e2.top)){
        return e1.getX(e1.top)<e2.getX(e2.top);
    }
    if(e1.m != e2.m){
        return e1.m < e2.m;
    }
    return false;
}

class sort_X{
    int y; 
public:
    sort_X(int p) : y(p) {}
    bool operator()(struct edge e1, struct edge e2){
        return e1.getX(y)<e2.getX(y);
    }
}; 


void blit(const GBitmap& fDevice, int y, int XL, int XR, const GPaint& paint, bool shade){
    assert (XL<=XR);
    if(shade == true){
        GShader* shader = paint.getShader();
        bool opaque = shader->isOpaque();
        const GBlendMode mode_t = paint.getBlendMode();
        GBlendMode mode = paint.getBlendMode();
        if(opaque){
            mode = simple_blend(mode_t, 1);
        }
        else{mode = mode_t;}
        GPixel src[XR-XL];
        shader->shadeRow(XL,y, XR-XL,src);
        for(int i =0; i<XR-XL; i++){
            *fDevice.getAddr(XL+i, y) = call_blend(mode, src[i], *fDevice.getAddr(XL+i, y));
        }
    }
    else{
        const GColor& color = paint.getColor(); 
        const float alpha = paint.getAlpha();
        if (alpha!=1){
            //printf("%f\n", alpha);
        }
        const GBlendMode mode_t = paint.getBlendMode();
        GBlendMode mode = simple_blend(mode_t, alpha);
        /* call function to simplify the blend mode*/
        GPixel pixelColor = pack_color(color);
        for(int i = XL; i < XR; i++){
            GPixel* pixel_addr = fDevice.getAddr(i,y);
            *pixel_addr = call_blend(mode, pixelColor, *(pixel_addr));
        }
    }
}      

