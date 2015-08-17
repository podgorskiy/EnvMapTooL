#pragma once

#include <vector>
#include <iostream>

struct pixel;
struct fpixel;
struct double2;
class IFileFormat;

class Texture
{
public:
	std::vector<fpixel> m_buff;
	int m_width;
	int m_height;
	bool m_cubemap;
    void LoadFromFile(const char* path);
    void SaveToFile(const char* path, IFileFormat* formatOptions);

private:
    void LoadDDStexture(std::istream& inputStream);
    void LoadTARGAtexture(std::istream& inputStream);
};

class IFileFormat
{
public:
    virtual void SaveToFile(const Texture& tex, std::ostream& outputStream) = 0;
};

class TGAFile: public IFileFormat
{
public:
    virtual void SaveToFile(const Texture& tex, std::ostream& outputStream);
};


void GetIndicesFromUV(const double2& uv, int width, int height, int& i, int& j);

double2 GetUVFromIndices(int width, int height, int i, int j);

fpixel FetchTexture(const Texture& tex, double2 uv, int face);

void WriteTexture(Texture& tex, double2 uv, int face, const fpixel& p);
