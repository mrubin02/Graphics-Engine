#include "GCanvas.h"
#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GPaint.h"
#include "maddie_blend.h"
#include "maddie_poly.h"
#include "stack"
#include "GShader.h"
#include "GMatrix.h"
#include "GPath.h"
#include "GRandom.h"
#include "maddie_mesh.h"

class MaddieCanvas: public GCanvas {
public:
    std::stack<GMatrix> st; 
    MaddieCanvas(const GBitmap& device) : fDevice(device) {
        GMatrix ID = GMatrix(1,0,0,0,1,0);
        st.push(ID);
    }
    void save() override{
        st.push(st.top());
    }

    void restore() override{
        st.pop();
    }

    void concat(const GMatrix& matrix) override{
        st.top() = st.top() * matrix;
    }

    int get_edges(const GPoint points[], int count, struct edge* edges){
        int index = 0; 
        /*CLIP*/
        for(int i=0; i<count; i++){
            GPoint p0 = points[i];
            GPoint p1 = points[i+1];

            if(i == count - 1){
                p1 = points[0];
            }
        
            struct edge* add= clip_poly(p0, p1, fDevice.width(), fDevice.height());
            for(int j =0; j<3; j++){
                if(add[j].init==true){
                    edges[index] = add[j];
                    index++; 
                }
            }
        }
        return index;
    }
    void drawQuad(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int level, const GPaint& paint) override{
        
        GMatrix invM;
        int count = (level+1) * (level+1) * 2;
        GPoint pts[] = {verts[0], verts[1], verts[2], verts[3]};
        int indices[count*3];
        GPoint ab = verts[1]-verts[0];
        GPoint ac = verts[3]-verts[0];
        GMatrix M1 = {ab.x(), ac.x(), verts[0].x(), ab.y(), ac.y(), verts[0].y()};
        M1.invert(&invM);
        float ds, dt, s, t;
        ds = 1.0/(level+1);
        dt = 1.0/(level+1);
        s = 0; 
        invM.mapPoints(pts, pts, 4);
        int* idx = indices;
        int track = 0;
        GPoint ret[count];
        GColor clrs[count];
        GPoint tex[count];
        for(int i = 0; i < level+2; i++){
            t = 0;
            for(int j = 0; j<level+2; j++){
                GPoint A = (1 - s) * pts[0] + s * pts[1];
                GPoint B = (1 - s) * pts[3] + s * pts[2];
                GPoint pt = (1-t) * A + B*t;
                if (colors!= nullptr) {
                    GColor cA = (1 - s) * colors[0] + s * colors[1];
                    GColor cB = (1 - s) * colors[3] + s * colors[2];
                    GColor cpt = (1-t) * cA + cB*t;
                    clrs[track] = cpt;
                }
                if (texs!= nullptr) {
                    GPoint tA = (1 - s) * texs[0] + s * texs[1];
                    GPoint tB = (1 - s) * texs[3] + s * texs[2];
                    GPoint tpt = (1-t) * tA + tB*t;
                    tex[track] = tpt;
                }

                ret[track] = pt;
                t += dt;
                track+=1;
            }
            s += ds;
        }

        for(int i =0; i< level+1; i++){
            for(int j = 0; j<level+1; j++){
                idx[0] = i+j*(level+2);
                idx[1] = i+j*(level+2) + 1;
                idx[2] = i+(j+1)*(level+2);
                idx[3] = i+j*(level+2) + 1;
                idx[4] = i+(j+1)*(level+2) + 1;
                idx[5] = i+(j+1)*(level+2);
                idx+=6;
            }
        }
        M1.mapPoints(ret, ret, count);
        if (colors==nullptr && texs==nullptr){
            drawMesh(ret, nullptr, nullptr, count, indices, paint);
            return;
        }
        else if (colors==nullptr){
            drawMesh(ret, nullptr, tex, count, indices, paint);
            return;
        }
        else if (texs==nullptr){
            drawMesh(ret, clrs, nullptr, count, indices, paint);
            return;
        }
        drawMesh(ret, clrs, tex, count, indices, paint);

    }
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint& paint) override{
        GPoint pts[3];
        GColor c[3];
        GPoint t[3];
        GShader* shader = paint.getShader();
        int n = 0;
        for (int i =0; i<count; i++){
            pts[0] = verts[indices[n]];
            pts[1] = verts[indices[n+1]];
            pts[2] = verts[indices[n+2]];
            if(colors!=nullptr){
                 c[0] = colors[indices[n]];
                 c[1] = colors[indices[n+1]];
                 c[2] = colors[indices[n+2]]; 
                 if (texs==nullptr){
                     drawMeshColor(this, pts, c, shader);
                     n+=3;
                     continue;
                 }
            }
            if(texs!=nullptr){
                t[0] = texs[indices[n]];
                t[1] = texs[indices[n+1]];
                t[2] = texs[indices[n+2]];
                if (colors == nullptr){
                    drawMeshTex(this, pts, t, shader);
                    n+=3;
                    continue;
                }
            }
            drawMeshTexColor(fDevice, this,pts,c,t,shader);
            n+=3;
        }
    }

    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override{
        struct edge edges[count*3];
        GShader* shader = paint.getShader();
        bool shade = false; 
        int index;
        if(shader != nullptr){
            shade = shader->setContext(st.top());
        }
        GPoint color[count];
        st.top().mapPoints(color,points, count);
        index = get_edges(color,count,edges);

        /*SORT*/
        std::sort(edges, edges+index, sort_edge);
    
        /*SCAN*/
        int L = 0; 
        int R = 1; 
        int next = 2;
        for(int i = edges[L].top; i<edges[L].bottom; i++){
            int XL = edges[L].getX(i);
            int XR = edges[R].getX(i);

            if(XL<0){XL=0;}
            if (L == index || R == index){
                break;
            }           
            blit(fDevice, i, XL, XR, paint, shade);
    
            if(i==edges[L].bottom-1){
                L = next;
                next++;
            }
            if(i==edges[R].bottom-1){
                R = next;
                next++;
            }
        }
    }

    void drawPath(const GPath& path, const GPaint& paint) override{
        
        GShader* shader = paint.getShader();
        bool shade = false; 
        if(shader != nullptr){
            shade = shader->setContext(st.top());
        }

        int count = path.countPoints();
        GPath::Edger e(path);
        GPath::Verb v;
        GPoint pts2[4];
        int index = 0;
        struct edge* add;
        std::vector<struct edge> edges;
        std::vector<GPoint> pts; 

        while((v= e.next(pts2))!= GPath::kDone){
            
            if(v==GPath::kQuad){
                st.top().mapPoints(pts2, pts2, 3);
                GPoint perror = 0.25*pts2[0] + 0.25*pts2[2] - 0.5*pts2[1];
                float error = sqrt(perror.x()*perror.x() + perror.y()*perror.y());
                float lines = sqrt(error/.25);
                int n = ceil(log2(lines));
                if(n<=0){continue;}
                else{pts = recurse_quad(0, lines, pts2, fDevice.height());}

                for (int i =0; i<pts.size()-1; i++){
                    GPoint p0 = pts[i];
                    GPoint p1 = pts[i+1];
                    add = clip_poly(p0, p1, fDevice.width(), fDevice.height());
                    for(int j =0; j<3; j++){
                        if(add[j].init==true){
                            edges.push_back(add[j]);
                            index++; 
                        }
                    }
                }
            }

            if (v == GPath::kLine){
                st.top().mapPoints(pts2, pts2, 2);
                add = clip_poly(pts2[0], pts2[1], fDevice.width(), fDevice.height());
                for(int j =0; j<3; j++){
                    if(add[j].init==true){
                        edges.push_back(add[j]);
                        index++; 
                    }
                }
            }

            if (v == GPath::kCubic){
                st.top().mapPoints(pts2, pts2, 4);
                float Ux = pts2[0].x() - 2*pts2[1].x() + pts2[2].x();
                float Uy = pts2[0].y() - 2*pts2[1].y() + pts2[2].y();
                float Vx = pts2[1].x() - 2*pts2[2].x() + pts2[3].x();
                float Vy = pts2[1].y() - 2*pts2[2].y() + pts2[3].y();
                float Wx = std::max(abs(Ux), abs(Vx));
                float Wy = std::max(abs(Uy), abs(Vy));
                float W = sqrt(Wx*Wx + Wy*Wy);
                int k = float(sqrt(.75 * abs(W)/0.25));

                pts = recurse_cubic(0, k, pts2, fDevice.height());

                for (int i =0; i<pts.size()-1; i++){
                    GPoint p0 = pts[i];
                    GPoint p1 = pts[i+1];
                    add = clip_poly(p0, p1, fDevice.width(), fDevice.height());
                    for(int j =0; j<3; j++){
                        if(add[j].init==true){
                            edges.push_back(add[j]);
                            index++; 
                        }
                    }
                }
            }
        }

        if(index ==0){return;}

        /*SORT*/
        std::sort(edges.begin(),edges.end(), sort_edge);

        int c = edges.size();

        for(int y = edges[0].top; count>0; y++){
            int track = 0;
            int w = 0; 
            int L; 
            int R; 
            while(track<c && edges[track].top <= y){
                int X = edges[track].getX(y);
                if (X>fDevice.width()){X=fDevice.width();}
                if (w == 0){
                    L = X;
                }
                w+=edges[track].w;
                if(w == 0){
                    R = X;
                    blit(fDevice, y, L, R, paint, shade);
                }
                if(y >= edges[track].bottom - 1){
                    edges.erase(edges.begin() + track);
                    c--;
                }
                else{
                    track++;
                }
            }
            if (c<=1){break;}
            while(track<c && y+1 == edges[track].top){
                track += 1;
            }
            sort(edges.begin(), edges.begin()+track, sort_X(y+1));
        } 
    }
    
    void drawPaint(const GPaint& paint) override {
        GShader* shader = paint.getShader();
        bool shade = false; 
        if (shader != nullptr){
            shade = shader->setContext(st.top());
        }
        if(!shade){
            const GColor& color = paint.getColor(); 
            GPixel pixelColor = pack_color(color);
            for(int i =0; i<fDevice.width(); i++){
                for(int j =0; j<fDevice.height(); j++){
                    GPixel* pixel_addr = fDevice.getAddr(i,j);
                    *pixel_addr = pixelColor;
                }
            }
        }
        else{
            for(int i =0; i<fDevice.height(); i++){
                blit(fDevice, i, 0, fDevice.width(), paint, shade);
            }
        }
    }
        
    void drawRect(const GRect& rect, const GPaint& paint) override {

        if(rect.left()>=fDevice.width() || round(rect.top())>=fDevice.height()){return;}
        
        GPoint points[] = {{rect.left(),rect.top()}, {rect.left(), rect.bottom()}, 
        {rect.right(),rect.bottom()}, {rect.right(), rect.top()}};
        this->drawConvexPolygon(points, 4, paint);
        return;
    }


private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas> (new MaddieCanvas(device));
}


static std::unique_ptr<GShader> make_bm_shader(GBitmap bm, GMatrix lm) {
    auto sh = GCreateBitmapShader(bm, lm);
    // just to be sure
    bm.reset();
    lm = GMatrix(0, 0, 0, 0, 0, 0);
    if (false) {
        printf("%p%p", &bm, &lm);
    }
    return sh;
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    
    GPaint white; 
    GColor w = {1,1,1,1};
    white.setColor(w);
    canvas->drawPaint(white);

    GBitmap bm;
    bm.readFromFile("pics/will4.png");
    auto shader = make_bm_shader(bm, GMatrix());
    GPaint p; 
    p.setShader(shader.get());

    GBitmap bm2;
    bm2.readFromFile("pics/wil2.png");
    auto shader2 = make_bm_shader(bm2, GMatrix());
    GPaint pt2; 
    pt2.setShader(shader2.get());

    GColor c1 = {1,1,0,.5};
    GColor c2 = {1,0,1,.5};
    GColor c3 = {0,0,0,.5};
    GColor c4 = {0,0,.5,.5};
    GColor clrs[] = {c1,c2,c3,c4};

    canvas->save();
    float sx = float(256)/(bm.height());
    float sy = float(256)/(bm.height());
    canvas->scale(sx,sy);
    canvas->translate(195, 0);
    GPoint p1 = {0, 0};
    GPoint p2 = {bm.width()-80, 0};
    GPoint p3 = {bm.width()-80, bm.height()};
    GPoint p4 = {0, bm.height()};
    GPoint pts[] = {p1, p2, p3, p4};

    
    GPoint tex[] = {p1, p2, p3, p4};
    canvas->drawQuad(pts, nullptr, tex, 2, p);
    //canvas->drawQuad(pts, clrs, nullptr, 4, p);
    canvas->restore();

    canvas->save();
    sx = float(256)/(bm2.height());
    sy = float(256)/(bm2.height());
    canvas->scale(sx,sy);
    canvas->translate(-100, 20);
    GPoint p12 = {0, 0};
    GPoint p22 = {bm2.width(), 0};
    GPoint p32 = {bm2.width(), bm2.height()};
    GPoint p42 = {0,bm2.height()};
    GPoint pts2[] = {p12, p22, p32, p42};
    GPoint tex2[] = {p12, p22, p32, p42};
    canvas->drawQuad(pts2, nullptr, tex2, 2, pt2);
    //canvas->drawQuad(pts2, clrs2, nullptr, 4, pt2);
    canvas->translate(100, -20);
    GPoint p14 = {160, 0};
    GPoint p24 = {bm2.width(), 0};
    GPoint p34 = {bm2.width()-160, bm2.height()};
    GPoint p44 = {0,bm2.height()};
    GColor c13 = {.1,.1,.3,.6};
    GColor c23 = {.1,.1,.3,.6};
    GColor c33 = {.1,.1,.3,.6};
    GColor c43 = {.1,.1,.3,.6};
    GColor clrs3[] = {c13,c23,c33,c43};
    GPoint pts4[] = {p14, p24, p34, p44};

    GPoint p15 = {0, 0};
    GPoint p25 = {bm2.width()-160, 0};
    GPoint p35 = {bm2.width(), bm2.height()};
    GPoint p45 = {160,bm2.height()};
    GPoint pts5[] = {p15, p25, p35, p45};

    canvas->drawQuad(pts4, clrs3, nullptr, 4, p);
    canvas->drawQuad(pts5, clrs, nullptr, 4, p);
    canvas->restore();

    return "get my face out of your 2D graphics project";
};
