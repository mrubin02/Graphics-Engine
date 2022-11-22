#include "GMatrix.h"
#include "GColor.h"
#include "GMath.h"
#include "GPoint.h"
#include "GRect.h"

GMatrix::GMatrix(){
    fMat[0] = 1;    fMat[1] = 0;    fMat[2] = 0;
    fMat[3] = 0;    fMat[4] = 1;    fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty){
    return {1,0,tx,0,1,ty};
}
GMatrix GMatrix::Scale(float sx, float sy){
    return {sx, 0, 0, 0, sy, 0};
}

GMatrix GMatrix::Rotate(float radians){
    return {cos(radians), -sin(radians), 0, sin(radians), cos(radians), 0};
}

GMatrix GMatrix::Concat(const GMatrix& secundo, const GMatrix& primo){
    float ret[6];
    for(int i = 0; i<6; i+=3){
        ret[i] = secundo[i] * primo[0] + secundo[i+1]*primo[3];
        ret[i+1] = secundo[i] * primo[1] + secundo[i+1]*primo[4];
        ret[i+2] = secundo[i] * primo[2] + secundo[i+1]*primo[5] + secundo[i+2];
    }
    return {ret[0], ret[1], ret[2], ret[3], ret[4], ret[5]};
}

bool GMatrix::invert(GMatrix* inverse) const {

    float a = this->fMat[0];
    float b = this->fMat[1];
    float c = this->fMat[2];
    float d = this->fMat[3];
    float e = this->fMat[4];
    float f = this->fMat[5];
    float g = 0;
    float h = 0;
    float i = 1;

    /*float det = a*(e*i - f*h) - b*(d*i-f*g) + c*(d*h - e*g);*/
    float det = a*(e*i - f*h) - b*(d*i-f*g) + c*(d*h-e*g);
    
    if (det == float(0)){
        return false;
    }
    else{
        float adj[9]; 
        adj[0] = (e*i - h*f);
        adj[1] = (d*i - g*f);
        if(adj[1]!=0){adj[1]*=-1;}
        adj[2] = (d*h-e*g);
        adj[3] = (b*i-c*h);
        if(adj[3]!=0){adj[3]*=-1;}
        adj[4] = (a*i-g*c);
        adj[5] = (a*h-g*b);
        if(adj[5]!=0){adj[5]*=-1;}
        adj[6] = (b*f-c*e);
        adj[7] = (a*f-d*c);
        if(adj[7]!=0){adj[7]*=-1;}
        adj[8] = a*e - d*b;

        *inverse = GMatrix(adj[0]/det, adj[3]/det, adj[6]/det, adj[1]/det, adj[4]/det, adj[7]/det);
        (*inverse)[0] = adj[0]/det;
        (*inverse)[1] = adj[3]/det;
        (*inverse)[2] = adj[6]/det;
        (*inverse)[3] = adj[1]/det;
        (*inverse)[4] = adj[4]/det;
        (*inverse)[5] = adj[7]/det;
        return true;
    }  
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for(int i =0; i<count; i++){
        float x = src[i].x();
        float y = src[i].y();
        dst[i].fX = this->fMat[0] * x + this->fMat[1]*y + this->fMat[2];
        dst[i].fY = this->fMat[3] * x + this->fMat[4]*y + this->fMat[5];
    }
}
