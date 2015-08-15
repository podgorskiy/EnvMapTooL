#pragma once
#include <cmath>

struct pixel 
{
	unsigned char r,g,b;
	pixel():r(0),g(0),b(0){};
	pixel(char r, char g, char b):r(r),g(g),b(b){};
	void operator += (const pixel& a){	r += a.r; g += a.g; b += a.b;}		
	void operator *= (const double& a){	
		r = static_cast<unsigned char>(r * a); 
		g = static_cast<unsigned char>(g * a); 
		b = static_cast<unsigned char>(b * a); 
	}	
};

struct apixel {
	unsigned char r,g,b,a;
	apixel():a(0),r(0),g(0),b(0){};
	apixel(char a, char r, char g, char b):a(a),r(r),g(g),b(b){};
	operator pixel() { return pixel(r,g,b);};
};

struct fpixel {
	double r,g,b;
	fpixel():r(0),g(0),b(0){};
	fpixel(double r, double g, double b):r(r),g(g),b(b){};
	fpixel(pixel p) : r(p.r/255.0f), g(p.g/255.0f), b(p.b/255.0f) {}		
	void operator += (const fpixel& v){	r += v.r; g += v.g; b += v.b;}	
	void operator += (const double& v){	r += v; g += v; b += v;}	
	void operator *= (const double& a){	r *= a; g *= a; b *=a;}	
	void operator *= (const fpixel& p){	r *= p.r; g *= p.g; b *=p.b;}
	void Set (const double& x){r=x;g=x;b=x;}
	void Set (const double& xr, const double& xg, const double& xb){r=xr;g=xg;b=xb;}
};

struct double2 {
	double x,y;
	double2():x(0),y(0){};
	double2(double x, double y):x(x),y(y){};
	void Set(const double& x_, const double& y_) { x = x_; y = y_; }
};

struct double3
{
	/// Default constructor does nothing (for performance).
	double3() {}
	/// Construct using coordinates.
	double3(double x, double y, double z) : x(x), y(y), z(z) {}		
	/// Set this vector to some specified coordinates.
	void Set(const double& x_, const double& y_, const double& z_) { x = x_; y = y_; z = z_;}
	/// Negate this vector.
	double3 operator -() const { double3 v; v.Set(-x, -y, -z); return v; }	
	/// Add a vector to this vector.
	void operator += (const double3& v){	x += v.x; y += v.y; z += v.z;}	
	/// Subtract a vector from this vector.
	void operator -= (const double3& v){	x -= v.x; y -= v.y; z -= v.z;}
	/// Multiply this vector by a scalar.
	void operator *= (const double& a){	x *= a; y *= a; z *=a;}
	/// Get the length of this vector (the norm).
	double Length() const{ return sqrt(x * x + y * y + z * z);}
	/// Get the length squared. 
	double Length2() const{ return x * x + y * y + z * z;}
	/// Convert this vector into a unit vector. Returns the length.
	void Normalize()
	{
		double length = Length();
		double invLength = 1.0f / length;
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}
	void rotatex(const double& a)
	{
		double tmp = cos(a) * y - sin(a) * z;
		z = sin(a) * y + cos(a) * z;
		y = tmp;
	}
	void rotatey(const double& a)
	{
		double tmp = cos(a) * x + sin(a) * z;
		z = - sin(a) * x + cos(a) * z;
		x = tmp;
	}
	double x, y, z;
};

inline double3 operator + (const double3& a, const double3& b){	return double3(a.x + b.x, a.y + b.y, a.z + b.z);};	
inline double3 operator - (const double3& a, const double3& b){	return double3(a.x - b.x, a.y - b.y, a.z - b.z);};	
inline double3 operator * (const double a, const double3& b){	return double3(a * b.x, a * b.y, a * b.z);};	
inline double3 operator * (const double3& b, double a){	return double3(a * b.x, a * b.y, a * b.z);};
inline double3 operator / (const double3& a, double b){	return double3(a.x / b, a.y / b, a.z / b);};
inline double dot (const double3& a, const double3& b){	return a.x * b.x + a.y * b.y + a.z * b.z;};		

template<typename T>
inline T clamp(const T&, double a, double b);

template<>
inline double clamp(const double& v, double a, double b)
{
	double x = v;
	if (v < a) {x = a;}
	if (v > b) {x = b;}
	return x;
}

template<>
inline double3 clamp(const double3& v, double a, double b)
{
	double3 x = v;
	x.x = clamp(x.x, a, b);
	x.y = clamp(x.y, a, b);
	x.z = clamp(x.z, a, b);
	return x;
}

template<>
inline double2 clamp(const double2& v, double a, double b)
{
	double2 x = v;
	x.x = clamp(x.x, a, b);
	x.y = clamp(x.y, a, b);
	return x;
}

inline int Round(double number)
{
    return static_cast<int>(number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5));
}

inline unsigned long xorshf96(void) {          //period 2^96-1
	static unsigned long x=123456789, y=362436069, z=521288629;
	unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return z;
}

inline float fastNormal(float sigma)
{
	int x = 
		(xorshf96() % 256) +
		(xorshf96() % 256) +
		(xorshf96() % 256) +
		(xorshf96() % 256) +
		(xorshf96() % 256);

	return (x - 5 * 127) / 127.0f * sigma / 3.0f;
}
