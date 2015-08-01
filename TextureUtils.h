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


void GetIndicesFormUV(const double2& uv, int width, int height, int& i, int& j);

double2 GetUVFormIndices(int width, int height, int i, int j);

pixel FetchFormTexture(const texture* tex, double2 uv, int face);

void WriteToTexture(texture* tex, double2 uv, int face, const pixel& p);