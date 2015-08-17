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

class Sphere2CubeMap: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex);
};

class DummyAction: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex)
    {
        outputTex = inputTex;
    }
};


int main(int argc, char* argv[])
{
	TCLAP::CmdLine cmd("EnvMapTool. Stanislav Podgorskiy.", ' ', "0.1", true);

	TCLAP::ValueArg<std::string> inputFileArg("i", "input", "The input texture file. Can be of the following formats: *.tga, *.png, *.dds", true, "", "Input file");
	TCLAP::MultiArg<std::string> inputMultiFileArg("I", "inputSequence", "The input texture files for cube map. You need specify six files: xp, xn yp, yn, zp, zn. WARNING! All the files MUST be the same format and size!", true, "Input files");
	TCLAP::ValueArg<std::string> outputFileArg("o", "output", "The output texture file.", true, "", "Output file");
	TCLAP::MultiArg<std::string> outputMultiFileArg("O", "outputSequence", "The output texture files for cube map. You need specify six files: xp, xn yp, yn, zp, zn", true, "Output files");
	TCLAP::ValueArg<std::string> outputFormatFileArg("f", "format", "Output texture file format. Can be one of the following \"TGA\", \"DDS\", \"PNG\". Default TGA.", false, "TGA", "Output format");
	TCLAP::ValueArg<int> faceToWriteArg("F", "faceToWrite", "If cubemap texture is written to format that does not support faces, this face will be written", false, 0, "Face to write");

    std::vector<std::string> allowed;
		allowed.push_back("cube2sphere");
		allowed.push_back("sphere2cube");
		allowed.push_back("convert");
		TCLAP::ValuesConstraint<std::string> allowedVals( allowed );

    TCLAP::UnlabeledValueArg<std::string>  actionLable( "action",
        "Action. Can be:\n"
        "\tcube2sphere - Converts cube map texture to spherical map\n"
        "\tsphere2cube - Converts spherical map texture to cube map\n"
        "\tconvert - Do nothing. Just to convert txture from one format to other\n", true, "", &allowedVals );

    cmd.add(actionLable);
    cmd.add(faceToWriteArg);
    cmd.add(outputFormatFileArg);
    cmd.xorAdd(outputFileArg, outputMultiFileArg);
    cmd.xorAdd(inputFileArg, inputMultiFileArg);
	cmd.parse( argc, argv );

    Texture* inputTex = new Texture;

    if ( inputFileArg.isSet() )
    {
        inputTex->LoadFromFile(inputFileArg.getValue().c_str());
    }
    else if ( inputMultiFileArg.isSet() )
    {
        std::vector<std::string> files = inputMultiFileArg.getValue();
        if(files.size() != 6)
        {
            printf("Error: You should specify exactly six input files.\n");
            return 0;
        }
        int face = 0;
        for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            inputTex->LoadFromFile(it->c_str(), face);
            face++;
        }
        inputTex->m_cubemap = true;
    }

    Texture* outputTex = new Texture;
    IFileFormat* format = NULL;
    std::string formatString = outputFormatFileArg.getValue();
    if (formatString == "TGA")
    {
        format = new TGAFile;
    }
    else
    {
        printf("Error: Wrong output format!\n");
        return 0;
    }

    IAction* action = NULL;
    std::string actionString = actionLable.getValue();
    if (actionString == "cube2sphere")
    {
        action = new CubeMap2Sphere;
    }
    else if (actionString == "sphere2cube")
    {
        action = new Sphere2CubeMap;
    }
    else if (actionString == "convert")
    {
        action = new DummyAction;
    }
    else
    {
        printf("Error: Wrong action!\n");
        return 0;
    }

    action->DoTask(*inputTex, *outputTex);

    if ( outputFileArg.isSet() )
    {
        int face = 0;
        if(outputTex->m_cubemap)
        {
            face = faceToWriteArg.getValue();
            face = face > 5 ? 5 : face;
            face = face < 0 ? 0 : face;
        }
        outputTex->SaveToFile(outputFileArg.getValue().c_str(), format, face);
    }
    else if ( outputMultiFileArg.isSet() )
    {
        if(!outputTex->m_cubemap)
        {
            printf("Error: Can't output not a cube map to a sequence of files.\n");
            return 0;
        }
        std::vector<std::string> files = outputMultiFileArg.getValue();
        if(files.size() != 6)
        {
            printf("Error: You should specify exactly six output files.\n");
            return 0;
        }
        int face = 0;
        for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            outputTex->SaveToFile(it->c_str(), format, face);
            face++;
        }
    }
	return 0;
}

void CubeMap2Sphere::DoTask(const Texture& inputTex, Texture& outputTex)
{
    if (!inputTex.m_cubemap)
    {
        printf("Error: For this task required cubmap.\n");
        return;
    }
	outputTex.m_width = inputTex.m_width * 4;
	outputTex.m_height = inputTex.m_height * 4;
	int size = outputTex.m_width*outputTex.m_height;
	outputTex.m_faces[0].m_buff.resize(size);

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

void Sphere2CubeMap::DoTask(const Texture& inputTex, Texture& outputTex)
{
    if (inputTex.m_cubemap)
    {
        printf("Error: For this task required not a cubmap.\n");
        return;
    }
	outputTex.m_width = inputTex.m_width / 4;
	outputTex.m_height = inputTex.m_height / 4;
	int size = outputTex.m_width*outputTex.m_height;
	for(int k = 0; k <6; ++k)
	{
        outputTex.m_faces[k].m_buff.resize(size);
        for (int i = 0;i<outputTex.m_height;i++)
        {
            for (int j = 0;j<outputTex.m_width;j++)
            {
                double2 uv = GetUVFromIndices(outputTex.m_width, outputTex.m_height, i, j);
                double3 v = uv2cube(uv, k);
                double2 sphereUv = v2spheruv(v);
                fpixel p = FetchTexture(inputTex, sphereUv, 0);
                WriteTexture(outputTex, uv, k, p);
            }
        }
    }
    outputTex.m_cubemap = true;
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


