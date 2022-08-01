////////////////////////////////////////////////////////////////////////////////////////
// MIT License
// Copyright (c) 2022 unmanned
////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////
// Roessler ////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float x;
	float y;
	float z;
	float a;
	float b;
	float c;
	float t;
    
} roessler;

void roessler_init(roessler* o)
{
    o->x = 1.0f;
	o->y = 1.0f;
	o->z = 1.0f;
	o->a = 0.2f;
	o->b = 0.2f;
	o->c = 5.7f;
	o->t = 0.01f;
}

void roessler_process(roessler* o)
{
    o->x += (-o->y - o->z) * o->t;
    o->y += (o->x + o->a * o->y) * o->t;
    o->z += (o->b + o->z * (o->x - o->c)) * o->t;
}

////////////////////////////////////////////////////////////////////////////////////
// Hopf ////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float	x;
	float	y;
	float	p;
    float   t;

} hopf;

void hopf_init(hopf* o)
{
    o->x = 0.01f;
	o->y = 0.01f;
	o->p = 0.11f;
    o->t = 0.01f;
}

void hopf_process(hopf* o)
{
    o->x += o->t * ( -o->y + o->x * (o->p - (o->x*o->x + o->y*o->y)));
    o->y += o->t * (  o->x + o->y * (o->p - (o->x*o->x + o->y*o->y)));
}

////////////////////////////////////////////////////////////////////////////////////////
// Helmholz ////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float	x;
	float	y;
    float   z;

	float	gamma;
	float	delta;
    float   t;
   
} helmholz;

void helmholz_init(helmholz* o)
{
    o->x = 0.1f;
	o->y = 0.1f;
    o->z = 0.1f;

	o->gamma = 5.11f;
	o->delta = 0.55f;
    o->t     = 0.01f;
}

void helmholz_process(helmholz* o)
{
    o->x += o->t * o->y;
    o->y += o->t * o->gamma * o->z;
    o->z += o->t * ( -o->z - o->delta * o->y - o->x - o->x * o->x);
}


//////////////////////////////////////////////////////////////////////////////////////
// Sprott ////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float	x;
	float	y;
    float   z;
    float   t;

} sprott;

void sprott_init(sprott *o)
{
    o->x = 0.1f;
    o->y = 0.1f;
    o->z = 0.1f;
    o->t = 0.1f;
}

void sprott_process(sprott *o)
{   
    o->x += o->t * o->y;
    o->y += o->t * (o->y * o->z - o->x);
    o->z += o->t * (1.0f - o->y * o->y);
}

////////////////////////////////////////////////////////////////////////////////////////
// Sprott-Linz /////////////////////////////////////////////////////////////////////////
typedef struct
{
	float x;
	float y;
    float z;
    float a;
    float t;

} linz;

void linz_init(linz* o)
{
    o->x = 0.1f;
	o->y = 0.1f;
    o->z = 0.1f;
    o->a = 0.5f;
    o->t = 0.1f;
}

void linz_process(linz* o)
{
    o->x += o->t * (o->y + o->z);
    o->y += o->t * (o->y * o->a - o->x);
    o->z += o->t * (o->x * o->x - o->z);
}

// ////////////////////////////////////////////////////////////////////////////////////////
// // Sprott-Linz D ///////////////////////////////////////////////////////////////////////
// struct linz_d
// {
// 	float	x = 0.1f;
// 	float	y = 0.1f;
//     float   z = 0.1f;

//     float   a = 3.0f;
//     float   t = 0.01;
    
//     void iterate()
//     {
//         x += t * (-y);
//         y += t * (x + z);
//         z += t * (x * z + a*y*y);
//     }

// };



// ////////////////////////////////////////////////////////////////////////////////////////
// // Sprott 6-term ///////////////////////////////////////////////////////////////////////
// struct sprott_st
// {
// 	float	x = 0.1f;
// 	float	y = 0.1f;
//     float   z = 0.1f;

// 	float	a = 0.8f;
// 	float	b = 0.5f;
//     float   c = 0.1f;
//     float   d = 1.0f;
//     float   t = 0.01f;
    
//     void iterate()
//     {
//         x += t * y * a;
//         y += t * (- y*z - x);
//         z += t * (b * y * y - c * x - d);
//     }

// };




// ////////////////////////////////////////////////////////////////////////////////////////
// // Rayleigh-Benard /////////////////////////////////////////////////////////////////////
// struct rayleigh
// {
// 	double	x = 0.01f;
// 	double	y = 0.0f;
//     double  z = 0.0f;

// 	float	a = 9.00f;
// 	float	r = 12.0f;
//     float   b = 5.00f;

//     float   t = 0.19f;
    
//     void iterate()
//     {
//         x = t * (- a*x + a*y);
//         y = t * (r*x - y - x*z);
//         z = t * (x*y - b*z);

//     }
// };




// ////////////////////////////////////////////////////////////////////////////////////
// // Wang ////////////////////////////////////////////////////////////////////////////
// struct wang
// {
// 	float	x = 0.1f;
// 	float	y = 0.1f;
//     float   z = 0.1f;
//     float   w;

// 	float	a = 27.5f;
// 	float	b = 3.0f;
//     float   c = 19.3f;
//     float   d = 3.3f;
//     float   h = 2.9f;
//     float   t = 0.001;
    
//     void iterate()
//     {
//         x += t * a * (y - x);
//         y += t * (b * x + c * y - x * z + w);
//         z += t * (y * y - h * z);
//         w  = d * -y;
//     }
// };




// ///////////////////////////////////////////////////////////////////////////////////////
// // Yu-Wang ////////////////////////////////////////////////////////////////////////////
// struct yu_wang
// {
// 	float	x = 0.1f;
// 	float	y = 0.1f;
//     float   z = 0.1f;

// 	float	a = 10.0f;
// 	float	b = 40.0f;
//     float   c = 2.0f;
//     float   d = 2.5f;

//     float   t = 0.001;
    
//     void iterate()
//     {
//         x += t * a * (y - x);
//         y += t * (b * x - c * x * z);
//         z += t * (pow(M_E, x*y) - d * z);

//     }
// };




////////////////////////////////////////////////////////////////////////////////////////
// Three-Scroll Unified Chaotic System (TSUCS) /////////////////////////////////////////
typedef struct 
{
	float	x;
	float	y;
    float   z;
	float	a;
	float	b;
    float   c;
    float   d;
    float   e;
    float   t;

} tsucs;

tsucs __tsucs = 
{
    .x = 1.0f,
    .y = 1.0f,
    .z = 1.0f,
    .a = 40.00f,
	.b = 0.500f,
    .c = 20.00f,
    .d = 0.833f,
    .e = 0.650f,
    .t = 0.001f
};

void fTsucs(tsucs *o)
{
    o->x += o->t * (o->a*(o->y-o->x) + o->b*o->x*o->z);
    o->y += o->t * (o->c*o->y - o->x*o->z);
    o->z += o->t * (o->d*o->z + o->x*o->y - o->e*o->x*o->x);
}


// ////////////////////////////////////////////////////////////////////////////////////////
// // Lorenz //////////////////////////////////////////////////////////////////////////////
// struct lorenz
// {
//         float a = 10.0;
//         float b = 28.0;
//         float c = 8.0 / 3.0;
//         float t = 0.01; 

//         float x = 0.1; 
//         float y = 0;
//         float z = 0; 

//         void iterate()
//         {
//                 x += t * a * (y - x);
//                 y += t * (x * (b - z) - y);
//                 z += t * (x * y - c * z);
//         }
// };

// ////////////////////////////////////////////////////////////////////////////////////////
// // Aizawa //////////////////////////////////////////////////////////////////////////////
// struct aizawa
// {
//         float a = 0.95f;
//         float b = 0.7f;
//         float c = 0.6f;
//         float d = 3.5f;
//         float e = 0.25f;
//         float f = 0.1f;

//         float t = 0.01f; 

//         float x = 0.1; 
//         float y = 0;
//         float z = 0; 

//         void iterate()
//         {
//                 x += t * ((z - b) * x - d * y);
//                 y += t * ((z - b) * y + d * x);
//                 z += t * (c + a*z - z*z*z/3.0f - (x*x+y*y)*(1.0f+e*z) + f*z*x*x*x);
//         }
// };


// ////////////////////////////////////////////////////////////////////////////////////////
// // Halvorsen //////////////////////////////////////////////////////////////////////////////
// struct halvorsen
// {
//         float a = 1.4;
//         float t = 0.01; 

//         float x = 0.1; 
//         float y = 0;
//         float z = 0; 

//         void iterate()
//         {
//                 x += t * (-a * x - 4.0f * y - 4.0f * z - y * y);
//                 y += t * (-a * y - 4.0f * z - 4.0f * x - z * z);
//                 z += t * (-a * z - 4.0f * x - 4.0f * y - x * x);
//         }
// };


////////////////////////////////////////////////////////////////////////////////////////
// Ikeda ///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float u;
	float x;
	float y;
	float t;
} ikeda;

ikeda __ikeda =
{
    .u = 0.918,
	.x = 0.8,
	.y = 0.7
};

void fIkeda(ikeda* o)
{ 
    o->t  = 0.4f - 6.0f / (1.0f + o->x * o->x + o->y * o->y);
    o->x  = 1.0f + o->u * (o->x * cos(o->t) - o->y * sin(o->t));
    o->y  = o->u * (o->x * sin(o->t) + o->y * cos(o->t));
}
// ////////////////////////////////////////////////////////////////////////////////////////
// // Duffing /////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float x;
	float y;
	float a;
	float b;

} duffing;

duffing __duffing =
{
    .x = 0.1f,
    .y = 0.1f,
    .a = 2.75f,
    .b = 0.2f
};

void fDuffing(duffing* o)
{
	o->x = o->y;
	o->y = (-o->b*o->x + o->a*o->y - o->y*o->y*o->y);
}
// ////////////////////////////////////////////////////////////////////////////////////////
// // Henon ///////////////////////////////////////////////////////////////////////////////
// struct henon
// {
// 	float x;
// 	float y;
//     float dy, dx;

// 	float a = 1.4f;
// 	float b = 0.3f;

//     float t = 1.0f;

// 	void iterate()
// 	{
// 		dx = t * (1.0f - a * x * x + y);
// 		dy = t * b * x;

//         x = dx;
//         y = dy;
// 	}
// };


////////////////////////////////////////////////////////////////////////////////////////
// Gingerbreadman //////////////////////////////////////////////////////////////////////
typedef struct
{
	float x;
	float y;

} gingerbreadman;

gingerbreadman __gingerbreadman = 
{
    .x = 1.0f,
    .y = 1.0f
};

void fGingerbreadman(gingerbreadman* o)
{
	o->x = 1.0f - o->y + abs(o->x);
	o->y = o->x;
}

////////////////////////////////////////////////////////////////////////////////////////
// Van Der Pol /////////////////////////////////////////////////////////////////////////
typedef struct 
{
	float x;
	float y;
	float f;
    float t;
	float m;

} vanderpol;

vanderpol __vanderpol =
{
    .x = 0.1f,
	.y = 0.1f,
	.f = 1.2f,
    .t = 0.1f,
	.m = 1.0f
};

void fVanderpol(vanderpol* o)
{
    o->x += o->t * o->y;
    o->y += o->t * (o->m * (o->f - o->x * o->x) * o->y - o->x);
}

// ////////////////////////////////////////////////////////////////////////////////////////
// // Kaplan-Yorke ////////////////////////////////////////////////////////////////////////
// struct kaplan_yorke
// {
// 	float   x;
// 	float   y;
//     long long a = 0xFFFFFFFF;
//     long long b = 2147483647;
//     float   t   = 0.1f;
// 	float   alpha;

//     inline void iterate()
//     {
//         long long aa = 2 * a % b;
//         x += t * (float(a) / float(b));
//         y += t * (alpha*y + cos(4.0f * M_PI * x));
//         a = aa;
//     }
// };



// ////////////////////////////////////////////////////////////////////////////////////////
// // Rabinovich-Fabrikant ////////////////////////////////////////////////////////////////
// struct rabinovich_fabrikant
// {
//     float gamma = 0.87f;
//     float alpha = 1.1f;
//     float x = 0.1f, y = 0.1f, z = 0.1f;
//     float t = 0.01f;

//     void iterate() 
//     {
//         x += t * (y*(x-1+x*x)+gamma*x);
//         y += t * (x*(3*z+1-x*x)+gamma*y);
//         z += t * (-2*z*(alpha+x*y));
//     }
// };



// ////////////////////////////////////////////////////////////////////////////////////////
// // Chen-Lee ////////////////////////////////////////////////////////////////////////////
// struct chen_lee
// {
//     float a = 45.0f, b = 3.0f, c = 28.0f;
//     float x = 1, y = 1, z = 1;
//     float t = 0.0035f;

//     void iterate()
//     {
//         x += t * a * (y - x);
//         y += t * ((c - a) * x - x*z + c*y);
//         z += t * (x*y - b*z);
//     }
// };



// ////////////////////////////////////////////////////////////////////////////////////////
// // Chua ////////////////////////////////////////////////////////////////////////////////
// struct chua
// {
//     float x = 1, y = 1, z = 1;  // Ins

//     float alpha  =  15.6;
//     float beta   =  28; 
//     float ma     = -1.143;
//     float mb     = -0.714;
//     float h;
//     float t = 0.1f;
    
//     float dx;
//     float dy;
//     float dz;
   
//     void iterate()
//     {
//         h  = ma * x + 0.5f * (ma - mb) * (abs(x + 1.0f) - abs(x - 1.0f));
//         dx = t * (alpha * (y - x - h));
//         dy = t * (x - y + z);
//         dz = t * (- beta * y);

//         x = dx;
//         y = dy;
//         z = dz;
//     }
// };



// ////////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////////