#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "CoordinateTransform.h"

#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include <tclap/CmdLine.h>


int main(int argc, char* argv[])
{
	TCLAP::CmdLine cmd("EnvMapTool. Stanislav Podgorskiy.", ' ', "0.1", false);

	TCLAP::ValueArg<std::string> inputFileArg("i", "input", "The input texture file. Can be of the following formats: *.tga, *.png, .*ddse", true, "", "Input file");
	cmd.add(inputFileArg);

	cmd.parse( argc, argv );

	double rad=0.002f;
	int iter = 10;

	texture* tex =  loadddstexture(inputFileArg.getValue().c_str());
	texture* otex = new texture;
	otex->width = tex->width * 4;
	otex->height = tex->height * 4;
	int size = otex->width*otex->height;
	otex->buff = new pixel[size];

	for (int i = 0;i<otex->height;i++)
	{
		for (int j = 0;j<otex->width;j++)
		{
			double2 uv = GetUVFromIndices(otex->width, otex->height, i, j);
			double3 v;
			bool valid = spheruv2v(uv, v);
			int face;
			double2 uv_ = cube2uv(v,&face);
			pixel p = FetchTexture( tex, uv_, face);
			p *= valid;
			WriteTexture(otex, uv, 0, p);
		}
	}

	/*
	texture* otex = new texture;
	otex->width = tex->width*4;
	otex->height = tex->height*2;
	int size = sizeof(pixel)*otex->width*otex->height;
	otex->buff = (pixel*)malloc(size);
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			otex->buff[(j) + (i+otex->height/2)*otex->width] = tex->buff[tex->width*tex->height*0 + j + i*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			otex->buff[(j+otex->width/2) + (i+otex->height/2)*otex->width] = tex->buff[tex->width*tex->height*1 + j + i*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			otex->buff[(j+otex->width/4) + (i+otex->height/2)*otex->width] = tex->buff[tex->width*tex->height*4 + j + i*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			otex->buff[(j+otex->width/4) + (i)*otex->width] = tex->buff[tex->width*tex->height*5 + j + i*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			otex->buff[(j+otex->width/2) + (i)*otex->width] = tex->buff[tex->width*tex->height*3 + j + i*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			otex->buff[(j) + (i)*otex->width] = tex->buff[tex->width*tex->height*2 + j + i*tex->width];
		}
	}

	*/
	/*
	double ax=0.5;

	texture* otex = new texture;
	otex->width = tex->width*4;
	otex->height = tex->height*2;
	int size = sizeof(pixel)*otex->width*otex->height;
	otex->buff = (pixel*)malloc(size);
	double2 uv;
	double2 uv_;
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			uv.x = j/(double)tex->width;
			uv.y = (tex->height - i)/(double)tex->height;
			double3 v = uv2cube(uv,0);
			v.rotatex(ax);
			int n;
			uv_ = cube2uv(v,&n);
			int j_ = uv_.x*tex->width;
			int i_ = (1.0-uv_.y)*tex->height;
			otex->buff[(j) + (i+otex->height/2)*otex->width] = tex->buff[tex->width*tex->height*n + j_ + i_*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			uv.x = j/(double)tex->width;
			uv.y = (tex->height - i)/(double)tex->height;
			double3 v = uv2cube(uv,1);
			v.rotatex(ax);
			int n;
			uv_ = cube2uv(v,&n);
			int j_ = uv_.x*tex->width;
			int i_ = (1.0-uv_.y)*tex->height;
			otex->buff[(j+otex->width/2) + (i+otex->height/2)*otex->width] = tex->buff[tex->width*tex->height*n + j_ + i_*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			uv.x = j/(double)tex->width;
			uv.y = (tex->height - i)/(double)tex->height;
			double3 v = uv2cube(uv,4);
			v.rotatex(ax);
			int n;
			uv_ = cube2uv(v,&n);
			int j_ = uv_.x*tex->width;
			int i_ = (1.0-uv_.y)*tex->height;
			otex->buff[(j+otex->width/4) + (i+otex->height/2)*otex->width] = tex->buff[tex->width*tex->height*n + j_ + i_*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			uv.x = j/(double)tex->width;
			uv.y = (tex->height - i)/(double)tex->height;
			double3 v = uv2cube(uv,5);
			v.rotatex(ax);
			int n;
			uv_ = cube2uv(v,&n);
			int j_ = uv_.x*tex->width;
			int i_ = (1.0-uv_.y)*tex->height;
			otex->buff[(j+otex->width/4) + (i)*otex->width] = tex->buff[tex->width*tex->height*n + j_ + i_*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			uv.x = j/(double)tex->width;
			uv.y = (tex->height - i)/(double)tex->height;
			double3 v = uv2cube(uv,3);
			v.rotatex(ax);
			int n;
			uv_ = cube2uv(v,&n);
			int j_ = uv_.x*tex->width;
			int i_ = (1.0-uv_.y)*tex->height;
			otex->buff[(j+otex->width/2) + (i)*otex->width] = tex->buff[tex->width*tex->height*n + j_ + i_*tex->width];
		}
	}
	for (int i = 0;i<tex->height;i++){
		for (int j = 0;j<tex->width;j++){
			uv.x = j/(double)tex->width;
			uv.y = (tex->height - i)/(double)tex->height;
			double3 v = uv2cube(uv,2);
			v.rotatex(ax);
			int n;
			uv_ = cube2uv(v,&n);
			int j_ = uv_.x*tex->width;
			int i_ = (1.0-uv_.y)*tex->height;
			otex->buff[(j) + (i)*otex->width] = tex->buff[tex->width*tex->height*n + j_ + i_*tex->width];
		}
	}

		*/
	//double ax=0.5;
	/*
	double rad=0.1f;
	int iter=100;

	texture* otex = new texture;
	otex->width = 2048;
	otex->height = 1024;
	int size = sizeof(pixel)*otex->width*otex->height;
	otex->buff = new pixel[size];
	double2 uv;
	double2 uv_;
	for (int s=0;s<6;s++){
		for (int i = 0;i<otex->height/2;i++){
			for (int j = 0;j<otex->width/4;j++){
				int r=0;
				int g=0;
				int b=0;
				for (int k=0;k<iter;k++){
					double2 uv = GetUVFormIndices(otex->width/4, otex->height/2, i, j);
					double3 v;
					switch(s){
						case 0: v = uv2cube(uv,0); break;
						case 1: v = uv2cube(uv,1); break;
						case 2: v = uv2cube(uv,4); break;
						case 3: v = uv2cube(uv,5); break;
						case 4: v = uv2cube(uv,3); break;
						case 5: v = uv2cube(uv,2); break;
					}
					v.x+=(50 - rand()%100)/50.0*rad;
					v.y+=(50 - rand()%100)/50.0*rad;
					v.z+=(50 - rand()%100)/50.0*rad;
					v.Normalize();
					int face;
					uv_ = cube2uv(v,&face);
					pixel p = FetchFormTexture( tex, uv_, face);
					r+=p.r;
					g+=p.g;
					b+=p.b;
				}
				pixel p(r/iter,g/iter,b/iter);
				switch(s){
					case 0: otex->buff[(j) + (i+otex->height/2)*otex->width] = p; break;
					case 1: otex->buff[(j+otex->width/2) + (i+otex->height/2)*otex->width]=p; break;
					case 2: otex->buff[(j+otex->width/4) + (i+otex->height/2)*otex->width]=p; break;
					case 3: otex->buff[(j+otex->width/4) + (i)*otex->width]=p; break;
					case 4: otex->buff[(j+otex->width/2) + (i)*otex->width]=p; break;
					case 5: otex->buff[(j) + (i)*otex->width]=p; break;
				}
			}
		}
	}
	*/
	savetgatexture(otex,"uffizi_cros.tga",0);

	return 0;
}
