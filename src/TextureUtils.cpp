#include "TextureUtils.h"

typedef unsigned int DWORD;
#include "dds.h"
#include "EnvMapMath.h"

#include <iostream>
#include <fstream>

void Texture::LoadFromFile(const char* path, int face)
{
	std::ifstream fileStream(path,std::ios::binary);

	//Test DDS
    int magic;
    fileStream.read((char*)&magic, 4);
    fileStream.seekg(0);
	if (magic == 0x20534444)
	{
        LoadDDStexture(fileStream, face);
        return;
	}

    //Test Targa
    struct TgaTestHeader
    {
        char unused;
        char color_map_type;
        char image_type;
        char unused2[5];
        char unused3[8];
        char pixel_depth;
    };
    TgaTestHeader tgaTest;
    fileStream.read((char*)&tgaTest, sizeof(TgaTestHeader));
    fileStream.seekg(0);
    if (
        ( (tgaTest.color_map_type & 0xFE) == 0) &&
        ( (tgaTest.image_type & 0xF0) == 0) &&
        ( (tgaTest.pixel_depth == 1) || (tgaTest.pixel_depth == 8) || (tgaTest.pixel_depth == 24) || (tgaTest.pixel_depth == 32) )
    )
    {
        LoadTARGAtexture(fileStream, face);
        return;
    }
}

void Texture::SaveToFile(const char* path, IFileFormat* formatOptions, int face)
{
	std::ofstream  outputStream(path, std::ios::binary);
	if ( ! outputStream) {
		std::cout << "Error: Can not create output file" << std::endl;
		return;
    }
    formatOptions->SaveToFile(*this, outputStream, face);
}

void Texture::LoadDDStexture(std::istream& inputStream, int face)
{
	DWORD magic;
	inputStream.read((char*)&magic,sizeof(DWORD));
	DDS_HEADER header;
	inputStream.read((char*)&header,sizeof(DDS_HEADER));
	m_cubemap = ((header.dwSurfaceFlags & DDS_SURFACE_FLAGS_CUBEMAP) == DDS_SURFACE_FLAGS_CUBEMAP);

    if (m_cubemap)
    {
        if ((header.dwCubemapFlags & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
        {
            printf("Error: Missing faces in cubemap!");
            return;
        }
    }

	if ((header.ddspf.dwFlags & DDS_RGB) != DDS_RGB)
	{
        printf("Error: Wrong DDS format!");
        return;
    }

	bool alpha = ((header.ddspf.dwFlags & DDS_RGBA) == DDS_RGBA);

	if ((header.ddspf.dwSize==32)|(header.ddspf.dwSize==24))
	{
		m_width = header.dwWidth;
		m_height = header.dwHeight;
		int countOfFaces = m_cubemap ? 6 : 1;
		int size = m_width * m_height;

        m_faces.resize(countOfFaces);

		for (int k=0; k < countOfFaces; k++)
		{
            if (!m_cubemap)
            {
                k = face;
            }

			m_faces[k].m_buff.resize(size);
			for (int j=m_height-1;j>=0;j--){
				for (int i=m_width-1;i>=0;i--){
                    fpixel f;
					if (alpha){
						apixel b;
						inputStream.read((char*)&b,sizeof(apixel));
						f = b;
					}else{
						pixel b;
						inputStream.read((char*)&b,sizeof(pixel));
						f = b;
					}
					f.Pow(2.2);
					m_faces[k].m_buff[i+j*m_width] = f;
				}
			}

            //skeeping mipmaps
			int mip=1;
			for (unsigned int j=0;j<header.dwMipMapCount-1;j++){
				mip*=2;
				int pos = static_cast<int>(inputStream.tellg());
				if (alpha)
					inputStream.seekg(pos + m_width*m_height*sizeof(apixel)/mip/mip);
				else
					inputStream.seekg(pos + m_width*m_height*sizeof(pixel)/mip/mip);
			}
		}
	}
	else
	{
        printf("Error: Wrong DDS format!");
        return;
    }
}

void Texture::LoadTARGAtexture(std::istream& inputStream, int face)
{
	char buff[18];
	inputStream.read(buff,18);
	if (face == 0)
	{
        m_width  = *((short*)&buff[0xc]);
        m_height = *((short*)&buff[0xe]);
	}
	int size = m_width*m_height;
	m_faces[face].m_buff.resize(size);
    for (int j=0;j<m_height;j++)
    {
		for (int i=0;i<m_width;i++)
		{
            pixel b;
            fpixel f;
			inputStream.read((char*)&b,sizeof(pixel));
			f = b;
            f.Pow(2.2);
			m_faces[face].m_buff[i+j*m_width] = f;
		}
    }
}

void TGAFile::SaveToFile(const Texture& tex, std::ostream& outputStream, int face)
{
	char buff[18];
	for (int i=0;i<18;buff[i++]=0);
	buff[2]=2;
	buff[0xc]=(char)tex.m_width;
	buff[0xd]=*((char*)&tex.m_width+1);
	buff[0xe]=(char)tex.m_height;
	buff[0xf]=*((char*)&tex.m_height+1);
	buff[0x10]=24;
	outputStream.write(buff,18);
    for (int j=0;j<tex.m_height;j++)
    {
		for (int i=0;i<tex.m_width;i++)
		{
            pixel b;
            fpixel f;
            f = tex.m_faces[face].m_buff[i+j*tex.m_width];
            f.Pow(1.0/2.2);
            b = pixel(f.r * 255, f.g * 255, f.b * 255);
            outputStream.write((char*)(&b),sizeof(pixel));
        }
    }
	return;
}

void GetIndicesFromUV(const double2& uv, int width, int height, int& i, int& j)
{
	double2 uv_ = clamp(uv, 0.0f, 1.0f);
	j = Round(uv_.x * (width - 1));
	i = height - 1 - Round(uv_.y * (height - 1));
}

void GetIndicesFromUV(const double2& uv, int width, int height, int& il, int& jl, int& ih, int& jh, double& wi, double& wj)
{
	double2 uv_ = clamp(uv, 0.0f, 1.0f);
	double j = uv_.x * (width - 1);
	double i = height - 1 - uv_.y * (height - 1);
    il = static_cast<int>(floor(i));
    ih = static_cast<int>(ceil(i));
    jl = static_cast<int>(floor(j));
    jh = static_cast<int>(ceil(j));
    wi = i - il;
    wj = j - jl;
}

double2 GetUVFromIndices(int width, int height, int i, int j)
{
	return double2(
		j/static_cast<double>(width-1),
		(height-1 - i)/static_cast<double>(height-1)
	);
}

fpixel FetchTexture(const Texture& tex, double2 uv, int face)
{
	int il = 0, ih = 0, jl = 0, jh = 0;
	double wi = 0.0, wj = 0.0;
	GetIndicesFromUV(uv, tex.m_width, tex.m_height, il, jl, ih, jh, wi, wj);
	fpixel ll = tex.m_faces[face].m_buff[jl + il*tex.m_width];
	fpixel lh = tex.m_faces[face].m_buff[jh + il*tex.m_width];
	fpixel hl = tex.m_faces[face].m_buff[jl + ih*tex.m_width];
	fpixel hh = tex.m_faces[face].m_buff[jh + ih*tex.m_width];
    fpixel result =
        ll * (1.0 - wi) * (1.0 - wj) +
        lh * (1.0 - wi) * wj +
        hl * wi * (1.0 - wj) +
        hh * wi * wj;
	return result;
}

void WriteTexture(Texture& tex, double2 uv, int face, const fpixel& p)
{
	int i = 0, j = 0;
	GetIndicesFromUV(uv, tex.m_width, tex.m_height, i, j);
	tex.m_faces[face].m_buff[j + i*tex.m_width] = p;
}
