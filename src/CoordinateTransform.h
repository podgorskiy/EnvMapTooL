#pragma once

enum ENVMAP_FACES
{
	ENVMAP_POSITIVEX,
	ENVMAP_NEGATIVEX,
	ENVMAP_POSITIVEY,
	ENVMAP_NEGATIVEY,
	ENVMAP_POSITIVEZ,
	ENVMAP_NEGATIVEZ
};

//Functions that transforms 3d direction to spherical 2d coordinates and vica versa
double2 v2spheruv(double3 IN);
bool spheruv2v(double2 IN, double3& v);

//Functions that transforms 3d direction to cubemap 2d coordinates and vica versa
double2 cube2uv(double3 IN, int* n);
double3 uv2cube(double2 uv, int n);
