
#ifndef __New3d_h
#define __New3d_h
#include <math.h>
#include "d3ddemo.h"
#include "typedefs.h"

/*===================================================================
	Defines
===================================================================*/
/* bjd - 3 defines below changed as they cause compilation errors with D3DVIEWPORT9 - only used in collision.c */
#define	ourX					0
#define	ourY					1
#define	ourZ					2
#define	EPS					1e-7
#define PI					3.14159265358979323846F
#define M_PI				3.14159265358979323846F
#define	D2R(x)				( ( x ) * ( 1.0F / 57.2957F) )		/* Nick Pelling changed */
#define	R2D(x)				( ( x ) * 57.2957F )
#define	SINR(x)				sin( x )
#define	COSR(x)				cos( x )
#define	SIND(x)				( sin( D2R( x ) ) )
#define	COSD(x)				( cos( D2R( x ) ) )
#define ATAND(y, x)			R2D( atan2(y, x) )
#define FMOD( NUM, DIV )	( (NUM) - (DIV) * ( (float) floor( (NUM) / (DIV) ) ) )
#define FDIV( NUM, DIV )	( (DIV) * ( (float) floor( (NUM) / (DIV) ) ) )

/*===================================================================
	Structures
===================================================================*/

// bjd - taken from d3dtypes.h
#define RGBA_MAKE(r, g, b, a)   ((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define	RGB_MAKE(r, g, b)    ((D3DCOLOR) (((r) << 16) | ((g) << 8) | (b)))
#define RGBA_GETALPHA(rgb)    ((rgb) >> 24) 

#define RGBA_GETRED(rgb)    (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)    (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)    ((rgb) & 0xff)

#define D3DFVF_LVERTEX    D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX1

#define D3DVAL(val)    ((float)val) 

typedef struct _D3DLVERTEX {
    union {
       float x;
       float dvX;
    };
    union {
       float y;
        float dvY;
    };
    union {
        float z;
        float dvZ;
    };
    union {
        D3DCOLOR color;
        D3DCOLOR dcColor;
    };
    union {
        D3DCOLOR specular;
		D3DCOLOR dcSpecular;
    };
    union {
        float tu;
        float dvTU;
    };
    union {
        float tv;
        float dvTV;
	};
/*
    _D3DLVERTEX() { }
    _D3DLVERTEX(const D3DVECTOR& v,D3DCOLOR col,D3DCOLOR spec,float _tu, float _tv)
        { x = v.x; y = v.y; z = v.z; 
          color = col; specular = spec;
          tu = _tu; tv = _tv;
    }
*/
} D3DLVERTEX, *LPD3DLVERTEX;

typedef struct _D3DTLVERTEX {
    union {
        float sx;
        float dvSX;
    };
    union {
        float sy;
        float dvSY;
    };
    union {
        float sz;
        float dvSZ;
    };
    union {
        float rhw;
        float dvRHW;
    };
    union {
        D3DCOLOR color;
        D3DCOLOR dcColor;
    };
    union {
        D3DCOLOR specular;
        D3DCOLOR dcSpecular;
    };
    union {
        float tu;
        float dvTU;
    };
    union {
        float tv;
        float dvTV;
    };

} D3DTLVERTEX, *LPD3DTLVERTEX;

#define D3DFVF_TLVERTEX	(D3DFVF_XYZRHW|/*D3DFVF_RESERVED0|*/D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX1)

typedef struct _D3DTRIANGLE { 
    union { 
        WORD v1; 
        WORD wV1; 
    }; 
    union { 
        WORD v2; 
        WORD wV2; 
    }; 
    union { 
        WORD v3; 
        WORD wV3; 
    }; 
    WORD     wFlags; 
} D3DTRIANGLE, *LPD3DTRIANGLE; 


/*===================================================================
	2D Vertices
===================================================================*/
typedef struct VERT2D {
	float	x;
	float	y;
} VERT2D;
/*===================================================================
	3D Vertices
===================================================================*/
typedef struct VERT {
	float	x;
	float	y;
	float	z;
} VERT;

/*===================================================================
	3D Normal
===================================================================*/
typedef struct NORMAL {
	float	nx;
	float	ny;
	float	nz;
} NORMAL;

/*===================================================================
	4 X 4 Matrix
===================================================================*/
typedef struct MATRIX {
	float	_11, _12, _13, _14;
	float	_21, _22, _23, _24;
	float	_31, _32, _33, _34;
	float	_41, _42, _43, _44;
} MATRIX;

/*===================================================================
	3 X 3 Matrix
===================================================================*/
typedef struct MATRIX3X3 {
	float	_11, _12, _13;
	float	_21, _22, _23;
	float	_31, _32, _33;
} MATRIX3X3;


/*===================================================================
	Vector
===================================================================*/
typedef struct VECTOR {
	float	x;
	float	y;
	float	z;
} VECTOR;

/*===================================================================
	Short Vector
===================================================================*/
typedef struct SHORTVECTOR {
	int16	x;
	int16	y;
	int16	z;
} SHORTVECTOR;


/*===================================================================
	Plane
===================================================================*/
typedef struct PLANE {
	VECTOR Normal;
	float Offset;
} PLANE;

/*===================================================================
	Prototypes
===================================================================*/
float DotProduct( VECTOR * a , VECTOR * b ); 

void CrossProduct( VECTOR * a, VECTOR * b, VECTOR * ab );

void NormaliseVector( VECTOR *  v );


void BuildRotMatrix( float xa, float ya, float za, MATRIX * m );
void MatrixMultiply( MATRIX * m0, MATRIX * m1, MATRIX * m0m1 );
void ApplyMatrix( MATRIX * m, VECTOR * v0, VECTOR * v1 );
void AddMatrixTrans( float xt, float yt, float zt, MATRIX * m );

void ReflectVector( VECTOR * old, NORMAL * normal, VECTOR * new1 );

float VectorLength( VECTOR * v ) ;

float DistanceVert2Vector( VERT *  a , VECTOR * b ) ;
float DistanceVector2Vector( VECTOR *  a , VECTOR * b );


void VisPolyApplyMatrix( MATRIX * m, VECTOR * v0, VECTOR * v1 );

void MatrixTranspose( MATRIX * m1, MATRIX * m2 );

uint16	Random_Range( uint16 Max );
float Random_Range_Float( float Max );


float	QuickDistance( VECTOR * V );
float	QuickDistance2d( float x , float y );
void	MakeViewMatrix(VECTOR *viewpos, VECTOR *lookpos, VECTOR *up, MATRIX *view);
void CalcViewAxes(VECTOR *viewpos, VECTOR *lookpos, VECTOR *up, VECTOR *vx, VECTOR *vy, VECTOR *vz);
void ScaleMatrix( MATRIX * m, VECTOR * v );
void MatrixFromAxisAndAngle( float angle, VECTOR * axis, MATRIX * rot );
void spline(VECTOR * p, float t, VECTOR * p1, VECTOR * p2, VECTOR * p3, VECTOR * p4);

#endif
