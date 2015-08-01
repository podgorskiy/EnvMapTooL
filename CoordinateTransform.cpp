#include "math.h"
#include "CoordinateTransform.h"

//Функция преобразования вектора отражения в текстурные координаты для сферической текстуры
double2 v2spheruv(double3 IN){
    double2 uv;
	double m = 2 * sqrt(IN.x*IN.x + IN.y*IN.y + (IN.z+1)*(IN.z+1));
	uv.x =  IN.x/m + 0.5;
	uv.y = -IN.y/m + 0.5;
	return uv;
}

bool spheruv2v(double2 IN, double3& v){
	double3 n(IN.x * 2.0 - 1.0, IN.y * 2.0 - 1.0, 0.0);
	double l = 1.0 - n.x * n.x - n.y * n.y;
	n.z = sqrt(abs(l));
	double3 r = n;
	r *= -2.0 * n.z;
	r.z += 1.0;
	v = r;
	return l >= 0.0;
}

//Функция преобразования вектора отражения в текстурные координаты для кубической текстуры
double2 scube2uv(double3 IN){
    double2 uv;
    double3 inSQ=IN;
	if(inSQ.x<0){
		inSQ.x=-inSQ.x;
	}
    if(inSQ.y<0){
		inSQ.y=-inSQ.y;
	}
    if(inSQ.z<0){
		inSQ.z=-inSQ.z;
	}

	if(inSQ.x>=inSQ.y&&inSQ.x>=inSQ.z){
        if(IN.x>0){
			uv.x=0.125;
            uv.y=0.25;
            uv.x+=IN.z/IN.x*0.125;
            uv.y-=IN.y/IN.x*0.25;
        }else{
            uv.x=0.625;
            uv.y=0.25;
            uv.x+=IN.z/IN.x*0.125;
            uv.y+=IN.y/IN.x*0.25;
        }
    }
    if(inSQ.y>inSQ.x&&inSQ.y>=inSQ.z){
		if(IN.y>0){
			uv.x=0.125;
            uv.y=0.75;
            uv.x-=IN.x/IN.y*0.125;
            uv.y+=IN.z/IN.y*0.25;
        }else{
			uv.x=0.625;
            uv.y=0.75;
            uv.x+=IN.x/IN.y*0.125;
            uv.y+=IN.z/IN.y*0.25;
        }
    }
    if(inSQ.z>inSQ.x&&inSQ.z>inSQ.y){
		if(IN.z>0){
			uv.x=0.375;
            uv.y=0.25;
            uv.x-=IN.x/IN.z*0.125;
            uv.y-=IN.y/IN.z*0.25*0.95;
        }else{
            uv.x=0.375;
            uv.y=0.75;
            uv.x-=IN.x/IN.z*0.125;
            uv.y+=IN.y/IN.z*0.25;
		}
    }
	return uv;
}


//Функция преобразования вектора отражения в текстурные координаты для кубической текстуры
double2 cube2uv(double3 IN, int* n){
    double2 uv;
    double3 inSQ=IN;
	if(inSQ.x<0){
		inSQ.x=-inSQ.x;
	}
    if(inSQ.y<0){
		inSQ.y=-inSQ.y;
	}
    if(inSQ.z<0){
		inSQ.z=-inSQ.z;
	}
	uv.x=0.5;
    uv.y=0.5;
	if(inSQ.x>=inSQ.y&&inSQ.x>=inSQ.z){
        if(IN.x>0){
			*n = 0;
            uv.x+=IN.z/IN.x*0.5;
            uv.y-=IN.y/IN.x*0.5;
        }else{
			*n = 1;
            uv.x+=IN.z/IN.x*0.5;
            uv.y+=IN.y/IN.x*0.5;
        }
    }
    if(inSQ.y>inSQ.x&&inSQ.y>=inSQ.z){
		if(IN.y>0){
			*n = 2;
            uv.x-=IN.x/IN.y*0.5;
            uv.y+=IN.z/IN.y*0.5;
        }else{
			*n = 3;
            uv.x+=IN.x/IN.y*0.5;
            uv.y+=IN.z/IN.y*0.5;
        }
    }
    if(inSQ.z>inSQ.x&&inSQ.z>inSQ.y){
		if(IN.z>0){
			*n = 4;
            uv.x-=IN.x/IN.z*0.5;
            uv.y-=IN.y/IN.z*0.5;
        }else{
			*n = 5;
            uv.x-=IN.x/IN.z*0.5;
            uv.y+=IN.y/IN.z*0.5;
		}
    }
	return uv;
}

double3 uv2cube(double2 uv, int n){
    double3 v(0.0, 0.0, 0.0);
	switch(n)
	{
	case ENVMAP_POSITIVEX:
		{
			v.x = 1;
            v.z = v.x*(2*uv.x - 1.0);
            v.y = v.x*(1.0 - 2*uv.y);
		}
	break;
	case ENVMAP_NEGATIVEX:
		{
			v.x = -1;
            v.z = v.x*(2*uv.x - 1.0);
            v.y = v.x*(2*uv.y - 1.0);
		}
	break;
	case ENVMAP_POSITIVEY:
		{
			v.y = 1;
            v.x = v.y*(1.0 - 2*uv.x);
            v.z = v.y*(2*uv.y - 1.0);
		}
	break;
	case ENVMAP_NEGATIVEY:
		{
			v.y = -1;
            v.x = v.y*(2*uv.x - 1.0);
            v.z = v.y*(2*uv.y - 1.0);
		}
	break;
	case ENVMAP_POSITIVEZ:
		{
			v.z = 1;
            v.x = v.z*(1.0 - 2*uv.x);
            v.y = v.z*(1.0 - 2*uv.y);
 		}
	break;
	case ENVMAP_NEGATIVEZ:
		{
			v.z = -1;
            v.x = v.z*(1.0 - 2*uv.x);
            v.y = v.z*(2*uv.y - 1.0);
		}
	}
	return v;
}
