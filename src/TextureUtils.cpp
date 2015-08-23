#include "TextureUtils.h"

typedef unsigned int DWORD;
#include "dds.h"
#include "EnvMapMath.h"

#include <png.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

void Texture::LoadFromFile(const char* path, int face)
{
	std::ifstream fileStream(path,std::ios::binary);

	int magic;
	fileStream.read((char*)&magic, 4);
	fileStream.seekg(0);

	//Test DDS
	if (magic == 0x20534444)
	{
		LoadDDStexture(fileStream, face);
		return;
	}

	//TEST PNG
	if (magic == 0x474E5089)
	{
		LoadPNGtexture(fileStream, face);
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

	if ((header.ddspf.dwRGBBitCount==32)|(header.ddspf.dwRGBBitCount==24))
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
					f.Pow(m_gamma);
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
			f.Pow(m_gamma);
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
			f.Pow(1.0f/tex.m_gamma);
			b = pixel(f.r * 255, f.g * 255, f.b * 255);
			outputStream.write((char*)(&b),sizeof(pixel));
		}
	}
	return;
}

void DDSFile::SaveToFile(const Texture& tex, std::ostream& outputStream, int face)
{
	DWORD magic = 0x20534444;
	outputStream.write((char*)&magic,sizeof(DWORD));
	DDS_HEADER header;
	header.dwSize = sizeof(DDS_HEADER);
	header.dwHeaderFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
	header.dwSurfaceFlags = DDSCAPS_TEXTURE;
	if(tex.m_cubemap)
	{
		header.dwSurfaceFlags |= DDS_SURFACE_FLAGS_CUBEMAP;
		header.dwCubemapFlags = DDS_CUBEMAP_ALLFACES;
	}
	header.ddspf.dwFlags = DDS_RGB;

	header.dwWidth = tex.m_width;
	header.dwHeight = tex.m_height;
	header.ddspf.dwSize=32;
	header.ddspf.dwRGBBitCount = 24;
	header.ddspf.dwRBitMask = 0x00ff0000;
	header.ddspf.dwGBitMask = 0x0000ff00;
	header.ddspf.dwBBitMask = 0x000000ff;
	header.ddspf.dwABitMask = 0xff000000;
	header.dwMipMapCount = 0;
	header.dwPitchOrLinearSize = (tex.m_width * 24 + 7)/8;

	outputStream.write((char*)&header,sizeof(DDS_HEADER));

	int countOfFaces = tex.m_cubemap ? 6 : 1;
	int size = tex.m_width * tex.m_height;

	for (int k=0; k < countOfFaces; k++)
	{
		for (int j=tex.m_height-1;j>=0;j--){
			for (int i=tex.m_width-1;i>=0;i--){
				pixel b;
				fpixel f;
				f = tex.m_faces[k].m_buff[i+j*tex.m_width];
				f.Pow(1.0f/tex.m_gamma);
				b = pixel(f.r * 255, f.g * 255, f.b * 255);
				outputStream.write((char*)(&b),sizeof(pixel));
			}
		}
	}
}

struct mem_encode
{
    char *buffer;
    size_t size;
};

void my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct mem_encode* p=(struct mem_encode*)png_get_io_ptr(png_ptr);
    size_t nsize = p->size + length;

    /* allocate or grow buffer */
    if(p->buffer)
        p->buffer = (char*)realloc(p->buffer, nsize);
    else
        p->buffer = (char*)malloc(nsize);

    if(!p->buffer)
        png_error(png_ptr, "Write Error");

    /* copy new bytes to end of buffer */
    memcpy(p->buffer + p->size, data, length);
    p->size += length;
}

void my_png_read_data(png_structp pngPtr, png_bytep data, png_size_t length) {
    png_voidp a = png_get_io_ptr(pngPtr);
    ((std::istream*)a)->read((char*)data, length);
}

void Texture::LoadPNGtexture(std::istream& inputStream, int face)
{
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
    	return;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
    	png_destroy_read_struct(&png_ptr, NULL, NULL);
    	return;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
    	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    	return;
    }
    png_set_read_fn(png_ptr,(png_voidp)&inputStream, my_png_read_data);
    png_read_info(png_ptr, info_ptr);
    m_width =  png_get_image_width(png_ptr, info_ptr);
    m_height = png_get_image_height(png_ptr, info_ptr);
    png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
    png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
    png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);
    switch (color_type)
    {
    case PNG_COLOR_TYPE_PALETTE:
        png_set_palette_to_rgb(png_ptr);
        channels = 3;
    break;
    case PNG_COLOR_TYPE_GRAY:
        if (bitdepth < 8)
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        bitdepth = 8;
    break;
    }
    if (bitdepth == 16)
        png_set_strip_16(png_ptr);

    png_bytep* rowPtrs = new png_bytep[m_height];
    char* data = new char[m_width * m_height * bitdepth * channels / 8];
    const unsigned int stride = m_width * bitdepth * channels / 8;

    for (size_t i = 0; i < m_height; i++) {
        png_uint_32 q = (m_height- i - 1) * stride;
        rowPtrs[i] = (png_bytep)data + q;
    }
    png_read_image(png_ptr, rowPtrs);
    delete[] (png_bytep)rowPtrs;
    png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)0);

	int size = m_width*m_height;
	m_faces[face].m_buff.resize(size);
	for (int j=0;j<m_height;j++)
	{
		for (int i=0;i<m_width;i++)
		{
			pixel b;
			fpixel f;
			int d = *reinterpret_cast<int*>(&data[(i+j*m_width) * bitdepth * channels / 8]);
			b.r = (0xFF0000 & d) >> 16;
			b.g = (0xFF00 & d) >> 8;
			b.b = (0xFF & d);
			f = b;
			f.Pow(m_gamma);
			m_faces[face].m_buff[i+j*m_width] = f;
		}
	}
	delete[] data;
}

void PNGFile::SaveToFile(const Texture& tex, std::ostream& outputStream, int face)
{
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_uint_32 bytes_per_row;
    png_byte **row_pointers = NULL;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
    	return;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
    	png_destroy_write_struct(&png_ptr, NULL);
    	return;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
    	png_destroy_write_struct(&png_ptr, &info_ptr);
    	return;
    }

    png_set_IHDR(png_ptr,
                 info_ptr,
                 tex.m_width,
                 tex.m_height,
                 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    bytes_per_row = tex.m_width * 3;
    row_pointers = (png_byte **)png_malloc(png_ptr, tex.m_height * sizeof(png_byte *));
    for (y = 0; y < tex.m_height; ++y) {
    	unsigned char *row = (unsigned char *)png_malloc(png_ptr, tex.m_width * sizeof(unsigned char) * 3);
    	row_pointers[y] = (png_byte *)row;
    	for (x = 0; x < tex.m_width; ++x) {
            pixel b;
			fpixel f;
			f = tex.m_faces[face].m_buff[x+(tex.m_height - 1 - y)*tex.m_width];
			f.Pow(1.0f/tex.m_gamma);
			b = pixel(f.r * 255, f.g * 255, f.b * 255);
    		*row++ = b.r;
    		*row++ = b.g;
    		*row++ = b.b;
    	}
    }

    mem_encode state;
    state.buffer = NULL;
    state.size = 0;

    png_set_write_fn(png_ptr, &state, my_png_write_data, NULL);

    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    if(state.buffer)
    {
      outputStream.write((char*)(state.buffer),state.size);
      free(state.buffer);
    }
    for (y = 0; y < tex.m_height; y++) {
    	png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);

    png_destroy_write_struct(&png_ptr, &info_ptr);
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
