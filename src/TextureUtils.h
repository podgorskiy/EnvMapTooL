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
	struct Face
	{
		std::vector<fpixel> m_buff;
	};
	std::vector<Face> m_faces;
	int m_width;
	int m_height;
	bool m_cubemap;
	void LoadFromFile(const char* path, int face = 0);
	void SaveToFile(const char* path, IFileFormat* formatOptions, int face = 0);

	float m_gamma;
	Texture():m_cubemap(false), m_gamma(2.2)
	{
		m_faces.resize(6);
	};
private:
	void LoadDDStexture(std::istream& inputStream, int face);
	void LoadTARGAtexture(std::istream& inputStream, int face);
};

class IFileFormat
{
public:
	virtual void SaveToFile(const Texture& tex, std::ostream& outputStream, int face) = 0;
};

class TGAFile: public IFileFormat
{
public:
	virtual void SaveToFile(const Texture& tex, std::ostream& outputStream, int face);
};

class DDSFile: public IFileFormat
{
public:
	virtual void SaveToFile(const Texture& tex, std::ostream& outputStream, int face);
};

void GetIndicesFromUV(const double2& uv, int width, int height, int& il, int& jl, int& ih, int& jh, double& wi, double& wj);
void GetIndicesFromUV(const double2& uv, int width, int height, int& i, int& j);

double2 GetUVFromIndices(int width, int height, int i, int j);

fpixel FetchTexture(const Texture& tex, double2 uv, int face);

void WriteTexture(Texture& tex, double2 uv, int face, const fpixel& p);
