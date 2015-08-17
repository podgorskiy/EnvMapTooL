#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "CoordinateTransform.h"

#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include <tclap/CmdLine.h>

class IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex) = 0;
};

class CubeMap2Sphere: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex);
};


int main(int argc, char* argv[])
{
	TCLAP::CmdLine cmd("EnvMapTool. Stanislav Podgorskiy.", ' ', "0.1", true);

	TCLAP::ValueArg<std::string> inputFileArg("i", "input", "The input texture file. Can be of the following formats: *.tga, *.png, *.dds", true, "", "Input file");
	TCLAP::ValueArg<std::string> outputFileArg("o", "output", "The output texture file.", true, "", "Output file");
	TCLAP::ValueArg<std::string> outputFormatFileArg("f", "format", "Output texture file format. Can be one of the following \"TGA\", \"DDS\", \"PNG\". Default TGA.", false, "TGA", "Output format");
    TCLAP::UnlabeledValueArg<std::string>  actionLable( "action",
        "Action. Can be:\n"
        "\tcube2sphere - Converts cube map texture to spherical map\n"
        "\tsphere2cube - Converts spherical map texture to cube map\n"
        "\tconvert - Do nothing. Just to convert txture from one format to other\n", true, "", "Action" );

    cmd.add(actionLable);
    cmd.add(outputFormatFileArg);
    cmd.add(outputFileArg);
    cmd.add(inputFileArg);
	cmd.parse( argc, argv );

    Texture* inputTex = new Texture;
    inputTex->LoadFromFile(inputFileArg.getValue().c_str());


    Texture* outputTex = new Texture;
    IFileFormat* format = NULL;
    std::string formatString = outputFormatFileArg.getValue();
    if (formatString == "TGA")
    {
        format = new TGAFile;
    }
    else
    {
        printf("Error: Wrong output format!");
        return 0;
    }

    IAction* action = NULL;
    std::string actionString = actionLable.getValue();
    if (actionString == "cube2sphere")
    {
        action = new CubeMap2Sphere;
    }
    else
    {
        printf("Error: Wrong action!");
        return 0;
    }

    action->DoTask(*inputTex, *outputTex);

	outputTex->SaveToFile(outputFileArg.getValue().c_str(), format);

	return 0;
}

void CubeMap2Sphere::DoTask(const Texture& inputTex, Texture& outputTex)
{
    if (!inputTex.m_cubemap)
    {
        printf("For this task required cubmap.");
    }
	outputTex.m_width = inputTex.m_width * 4;
	outputTex.m_height = inputTex.m_height * 4;
	int size = outputTex.m_width*outputTex.m_height;
	outputTex.m_buff.resize(size);

	for (int i = 0;i<outputTex.m_height;i++)
	{
		for (int j = 0;j<outputTex.m_width;j++)
		{
			double2 uv = GetUVFromIndices(outputTex.m_width, outputTex.m_height, i, j);
			double3 v;
			bool valid = spheruv2v(uv, v);
			int face;
			double2 uv_ = cube2uv(v,&face);
			fpixel p = FetchTexture(inputTex, uv_, face);
			p *= valid;
			WriteTexture(outputTex, uv, 0, p);
		}
	}
}

	/*
	double rad=0.002f;
	int iter = 10;


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


