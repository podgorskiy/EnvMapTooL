#pragma once

struct pixel;
struct double2;

struct texture{
	pixel* buff;
	int width;
	int height;
};

texture* loadddstexture(const char* path);
texture* loadtgatexture(const char* path);
int savetgatexture(texture* tex, const char* path,int image);


void GetIndicesFromUV(const double2& uv, int width, int height, int& i, int& j);

double2 GetUVFromIndices(int width, int height, int i, int j);

pixel FetchTexture(const texture* tex, double2 uv, int face);

void WriteTexture(texture* tex, double2 uv, int face, const pixel& p);
