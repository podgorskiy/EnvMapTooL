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

//Функция преобразования вектора отражения в текстурные координаты для сферической текстуры
double2 v2spheruv(double3 IN);
bool spheruv2v(double2 IN, double3& v);

//Функция преобразования вектора отражения в текстурные координаты для кубической текстуры
double2 scube2uv(double3 IN);

//Функция преобразования вектора отражения в текстурные координаты для кубической текстуры
double2 cube2uv(double3 IN, int* n);
double3 uv2cube(double2 uv, int n);