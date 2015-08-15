#include "TextureUtils.h"

typedef unsigned long DWORD;
#include "dds.h"
#include "math.h"

#include <iostream>
#include <fstream>

texture* loadddstexture(const char* path)
{
	texture* t = new texture;
	std::ifstream  ddsfile(path,std::ios::binary);
	DWORD magic;
	ddsfile.read((char*)&magic,sizeof(DWORD));
	DDS_HEADER header;
	ddsfile.read((char*)&header,sizeof(DDS_HEADER));
	if ((header.dwSurfaceFlags & DDS_SURFACE_FLAGS_CUBEMAP) != DDS_SURFACE_FLAGS_CUBEMAP)
		return NULL;
	if ((header.dwCubemapFlags & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
		return NULL;
	std::cout<<header.ddspf.dwFlags;
	if ((header.ddspf.dwFlags & DDS_RGB) != DDS_RGB)
		return NULL;
	bool alpha = ((header.ddspf.dwFlags & DDS_RGBA) == DDS_RGBA);
	if ((header.ddspf.dwSize==32)|(header.ddspf.dwSize==24)){
		t->width = header.dwWidth;
		t->height = header.dwHeight;
		int size = t->width*t->height*6;
		t->buff = new pixel[size];
		for (int k=0;k<6;k++){
			for (int j=t->height-1;j>=0;j--){
				for (int i=t->width-1;i>=0;i--){
					if (alpha){
						apixel b;
						ddsfile.read((char*)&b,sizeof(apixel));
						t->buff[i+j*t->width+t->width*t->height*k] = b;
					}else{
						pixel b;
						ddsfile.read((char*)&b,sizeof(pixel));
						t->buff[i+j*t->width+t->width*t->height*k] = b;
					}
				}
			}
			int mip=1;
			for (unsigned int j=0;j<header.dwMipMapCount-1;j++){
				mip*=2;
				int pos = static_cast<int>(ddsfile.tellg()); 
				if (alpha)
					ddsfile.seekg(pos + t->width*t->height*sizeof(apixel)/mip/mip);
				else
					ddsfile.seekg(pos + t->width*t->height*sizeof(pixel)/mip/mip);
			}
		}
	}else
		return NULL;
	/*header.
	t->width  = *((short*)&buff[0xc]);
	t->height = *((short*)&buff[0xe]);
	int size = sizeof(pixel)*t->width*t->height;
	t->buff = (pixel*)malloc(size);
	tgafile.read((char*)t->buff,size);*/
	return t;
}

texture* loadtgatexture(const char* path){	
	texture* t = new texture;	
	std::ifstream  tgafile(path,std::ios::binary);
	char buff[18];
	tgafile.read(buff,18);
	t->width  = *((short*)&buff[0xc]);
	t->height = *((short*)&buff[0xe]);
	int size = t->width*t->height;
	t->buff = new pixel[size];
	tgafile.read((char*)t->buff,size);
	return t;
}

int savetgatexture(texture* tex, const char* path,int image){	
	std::ofstream  renderoutfile(path, std::ios::binary);
	if ( ! renderoutfile) {
		std::cout << "ошибка: не могу открыть выходной файл: " << std::endl;
		return -2;
    }
	char buff[18];
	for (int i=0;i<18;buff[i++]=0);
	buff[2]=2;
	buff[0xc]=(char)tex->width;
	buff[0xd]=*((char*)&tex->width+1);
	buff[0xe]=(char)tex->height;
	buff[0xf]=*((char*)&tex->height+1);
	buff[0x10]=24;
	renderoutfile.write(buff,18);
	renderoutfile.write((char*)(tex->buff+tex->width*tex->height*image),tex->width*tex->height*sizeof(pixel));
	renderoutfile.close();
	return 0;
}

void GetIndicesFromUV(const double2& uv, int width, int height, int& i, int& j)
{
	double2 uv_ = clamp(uv, 0.0f, 1.0f);
	j = Round(uv_.x * (width - 1));
	i = height - 1 - Round(uv_.y * (height - 1));
}

double2 GetUVFromIndices(int width, int height, int i, int j)
{
	return double2(
		j/static_cast<double>(width-1), 
		(height-1 - i)/static_cast<double>(height-1)
	);
}

pixel FetchTexture(const texture* tex, double2 uv, int face)
{
	int i = 0, j = 0;
	GetIndicesFromUV(uv, tex->width, tex->height, i, j);
	return tex->buff[tex->width * tex->height * face + j + i*tex->width];
}

void WriteTexture(texture* tex, double2 uv, int face, const pixel& p)
{
	int i = 0, j = 0;
	GetIndicesFromUV(uv, tex->width, tex->height, i, j);
	tex->buff[tex->width * tex->height * face + j + i*tex->width] = p;
}
