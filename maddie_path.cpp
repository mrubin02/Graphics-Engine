#include "GPath.h"
#include "GRect.h"

GPath& GPath::addRect(const GRect& rect, GPath::Direction dir){
    GPoint lt = {rect.left(), rect.top()};
    GPoint lb = {rect.left(), rect.bottom()};
    GPoint rb = {rect.right(), rect.bottom()};
    GPoint rt = {rect.right(), rect.top()};
    moveTo(lt);
    if (dir == GPath::kCW_Direction){
       lineTo(rt);
       lineTo(rb);
       lineTo(lb);
    }
    if (dir == GPath::kCCW_Direction){
       lineTo(lb);
       lineTo(rb);
       lineTo(rt);
    }
    return *this;
}

GPath& GPath::addPolygon(const GPoint pts[], int count){
    moveTo(pts[0]);
    for(int i = 1; i<count; i++){
        lineTo(pts[i]);
    }
    return *this;
}

GPath& GPath::addCircle(GPoint center, float radius, GPath::Direction dir){
    GPoint start = {center.x(),center.y()-radius};
    float c = 0.55228474983079 * radius;
    moveTo(start);
    if (dir == GPath::kCW_Direction){
        GPoint p1 = {c + center.x(), center.y() - radius};
        GPoint p2 = {center.x() + radius, -c + center.y()};
        GPoint p3 = {center.x()+radius, center.y()};
        cubicTo(p1, p2,p3);
        p2.fY += 2*c;
        p1.fY += 2*radius;
        start.fY += 2*radius;
        cubicTo(p2,p1,start);
        p1.fX -= 2*c;
        p2.fX -= 2*radius;
        p3.fX -= 2*radius;
        cubicTo(p1,p2,p3);
        p2.fY -= 2*c;
        p1.fY -= 2*radius;
        start.fY -= 2*radius;
        cubicTo(p2,p1,start);
    }

    if (dir == GPath::kCCW_Direction){
        GPoint p1 = {-c + center.x(), center.y() - radius};
        GPoint p2 = {center.x() - radius, -c + center.y()};
        GPoint p3 = {center.x()-radius, center.y()};
        cubicTo(p1, p2,p3);
        p2.fY += 2*c;
        p1.fY += 2*radius;
        start.fY += 2*radius;
        cubicTo(p2,p1,start);
        p1.fX += 2*c;
        p2.fX += 2*radius;
        p3.fX += 2*radius;
        cubicTo(p1,p2,p3);
        p2.fY -= 2*c;
        p1.fY -= 2*radius;
        start.fY -= 2*radius;
        cubicTo(p2,p1,start);
    }
    return *this;
}   


GRect GPath::bounds() const{
    if (fPts.size() == 0){
        return GRect::LTRB(0,0,0,0);
    }
    GRect rect = GRect::LTRB(fPts[0].x(), fPts[0].y(), fPts[0].x(), fPts[0].y());

    for(int i =1; i< fPts.size(); i++){
        if (rect.top() > fPts[i].y()){
           rect.fTop = fPts[i].y();
        }
        if (rect.bottom() < fPts[i].y()){
           rect.fBottom = fPts[i].y();
        }
        if (rect.left() > fPts[i].x()){
           rect.fLeft = fPts[i].x();
        }
        if (rect.right() < fPts[i].x()){
           rect.fRight = fPts[i].x();
        }
    }
    return rect;
}


void GPath::transform(const GMatrix& mat){
    for(int i =0; i< fPts.size(); i++){
        fPts[i] = mat * fPts[i];
    }
}


void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
    GPoint ABt =  (1-t)*src[0] + t*src[1];
    GPoint BCt =  (1-t)*src[1] + t*src[2];
    GPoint Pt =   (1-t)*(1-t)*src[0] + 2*src[1]*t*(1-t) + t*t*src[2];

    dst[0] = src[0];
    dst[1] = ABt;
    dst[2] = Pt;
    dst[3] = BCt; 
    dst[4] = src[2];
}

void GPath::ChopCubicAt(const GPoint src[], GPoint dst[], float t){
        
    // A = -a+3b-3c+d 
    // B = 3a -6b + 3c
    // C = -3a + 3b 
    // D = a 

    
    GPoint ABt =  (1-t)*src[0] + t*src[1];
    GPoint BCt =  (1-t)*src[1] + t*src[2];
    GPoint CDt =  (1-t)*src[2] + t*src[3];

    GPoint ABCt =  (1-t)*ABt+ t*BCt;
    GPoint BCDt =  (1-t)*BCt+ t*CDt;
    GPoint Pt =    (1-t)*ABCt + t*BCDt;
    
    dst[0] = src[0];
    dst[1] = ABt;
    dst[2] = ABCt;
    dst[3] = Pt;
    dst[4] = BCDt;
    dst[5] = CDt;
    dst[6] = src[3];
}







