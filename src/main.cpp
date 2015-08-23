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

    bool m_doNotRemoveOuterAreas;
};

class Sphere2CubeMap: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex);
};

class BlurCubemap: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex);

    float m_blurRadius;
    int m_blurQuality;
};

class FastBlurCubemap: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex);

    float m_blurRadius;
};

class DummyAction: public IAction
{
public:
    virtual void DoTask(const Texture& inputTex, Texture& outputTex)
    {
        outputTex = inputTex;
    }
};

std::vector<double> GenerateKernel(double sigma, int kernelSize, int sampleCount);

int main(int argc, char* argv[])
{
	TCLAP::CmdLine cmd("EnvMapTool. Stanislav Podgorskiy.", ' ', "0.1", true);

	TCLAP::ValueArg<std::string> inputFileArg("i", "input", "The input texture file. Can be of the following formats: *.tga, *.png, *.dds", true, "", "Input file");
	TCLAP::MultiArg<std::string> inputMultiFileArg("I", "inputSequence", "The input texture files for cube map. You need specify six files: xp, xn yp, yn, zp, zn. WARNING! All the files MUST be the same format and size!", true, "Input files");
	TCLAP::ValueArg<std::string> outputFileArg("o", "output", "The output texture file.", true, "", "Output file");
	TCLAP::MultiArg<std::string> outputMultiFileArg("O", "outputSequence", "The output texture files for cube map. You need specify six files: xp, xn yp, yn, zp, zn", true, "Output files");
	TCLAP::ValueArg<std::string> outputFormatFileArg("f", "format", "Output texture file format. Can be one of the following \"TGA\", \"DDS\", \"PNG\". Default TGA.", false, "TGA", "Output format");
	TCLAP::ValueArg<int> faceToWriteArg("F", "faceToWrite", "If cubemap texture is written to format that does not support faces, this face will be written", false, 0, "Face to write");

	TCLAP::ValueArg<int> blurQualityArg("q", "blurQuality", "Effects the number of samples in Monte Carlo integration. Reasonable values are between 4 - 8. Large values will increase calculation time dramatically. Default is 4", false, 4, "Blur quality");
    TCLAP::ValueArg<float> blurRadiusArg("b", "blurRadius", "Gaussian blur radius. Default is 10.0", false, 10.0f, "Blur radius");
    TCLAP::SwitchArg doNotRemoveOuterAreaFlag("l", "leaveOuter", "If flag is set, than while cubemap -> sphere transform area around the sphere circule are not filled black, but represent mathematical extrapolation.", false);

    TCLAP::ValueArg<float> inputGammaArg("g", "inputGamma", "Gamma of input texture. Default is 2.2", false, 2.2f, "Input gamma");
    TCLAP::ValueArg<float> outputGammaArg("G", "outputGamma", "Gamma of output texture. Default is 2.2", false, 2.2f, "Output gamma");

    TCLAP::ValueArg<int> outputWidthArg("W", "outputWidth", "Width of output texture. Default is the same as input, or 4 times upscaled in case of cube2sphere transform, or 4 times downscaled in case of sphere2cube transform", false, -1, "Output texture width");
    TCLAP::ValueArg<int> outputHeightArg("H", "outputHeight", "Height of output texture. Default is the same as input, or 4 times upscaled in case of cube2sphere transform, or 4 times downscaled in case of sphere2cube transform", false, -1, "Output texture height");

    std::vector<std::string> allowed;
		allowed.push_back("cube2sphere");
		allowed.push_back("sphere2cube");
		allowed.push_back("blurCubemap");
		allowed.push_back("fastBlurCubemap");
		allowed.push_back("convert");
		TCLAP::ValuesConstraint<std::string> allowedVals( allowed );

    TCLAP::UnlabeledValueArg<std::string>  actionLable( "action",
        "Action. Can be:\n"
        "\tcube2sphere - Converts cube map texture to spherical map\n"
        "\tsphere2cube - Converts spherical map texture to cube map\n"
        "\tblurCubemap - Gaussian blur of cubemap\n"
        "\tconvert - Do nothing. Just to convert txture from one format to other\n", true, "", &allowedVals );

    cmd.add(actionLable);
    cmd.add(outputWidthArg);
    cmd.add(outputHeightArg);
    cmd.add(outputGammaArg);
    cmd.add(inputGammaArg);
    cmd.add(doNotRemoveOuterAreaFlag);
    cmd.add(blurRadiusArg);
    cmd.add(blurQualityArg);
    cmd.add(faceToWriteArg);
    cmd.add(outputFormatFileArg);
    cmd.xorAdd(outputFileArg, outputMultiFileArg);
    cmd.xorAdd(inputFileArg, inputMultiFileArg);
	cmd.parse( argc, argv );

    Texture* inputTex = new Texture;
    inputTex->m_gamma = inputGammaArg.getValue();

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
    outputTex->m_gamma = outputGammaArg.getValue();

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

    int outputWidth = inputTex->m_width;
    int outputHeight = inputTex->m_height;

    IAction* action = NULL;
    std::string actionString = actionLable.getValue();
    if (actionString == "cube2sphere")
    {
        outputWidth *= 4;
        outputHeight *= 4;
        CubeMap2Sphere* cube2s = new CubeMap2Sphere;
        cube2s->m_doNotRemoveOuterAreas = doNotRemoveOuterAreaFlag.getValue();
        action = cube2s;
    }
    else if (actionString == "sphere2cube")
    {
        outputWidth /= 4;
        outputHeight /= 4;
        action = new Sphere2CubeMap;
    }
    else if (actionString == "blurCubemap")
    {
        BlurCubemap* blur = new BlurCubemap;
        blur->m_blurQuality = blurQualityArg.getValue();
        blur->m_blurRadius = blurRadiusArg.getValue();
        action = blur;
    }
    else if (actionString == "fastBlurCubemap")
    {
        FastBlurCubemap* blur = new FastBlurCubemap;
        blur->m_blurRadius = blurRadiusArg.getValue();
        action = blur;
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

	outputTex->m_width = outputWidthArg.isSet() ? outputWidthArg.getValue() : outputWidth;
	outputTex->m_height = outputHeightArg.isSet() ? outputHeightArg.getValue() : outputHeight;

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
	int size = outputTex.m_width*outputTex.m_height;
	outputTex.m_faces[0].m_buff.resize(size);

	for (int i = 0;i<outputTex.m_height;i++)
	{
		for (int j = 0;j<outputTex.m_width;j++)
		{
			double2 uv = GetUVFromIndices(outputTex.m_width, outputTex.m_height, i, j);
			double3 v;
			bool valid = spheruv2v(uv, v) + m_doNotRemoveOuterAreas;
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

void BlurCubemap::DoTask(const Texture& inputTex, Texture& outputTex)
{
    if (!inputTex.m_cubemap)
    {
        printf("Error: For this task required cubmap.\n");
        return;
    }
    float s = m_blurRadius / sqrt(inputTex.m_width * inputTex.m_height);

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
                fpixel p(0.0,0.0,0.0);
                int iterationCount = m_blurQuality;
                for (int itx = 0; itx < iterationCount; ++itx)
                {
                    for (int ity = 0; ity < iterationCount; ++ity)
                    {
                        for (int itz = 0; itz < iterationCount; ++itz)
                        {
                            float x = fastNormal(itx * (1.0/iterationCount), (itx+1) * (1.0/iterationCount));
                            float y = fastNormal(ity * (1.0/iterationCount), (ity+1) * (1.0/iterationCount));
                            float z = fastNormal(itz * (1.0/iterationCount), (itz+1) * (1.0/iterationCount));
                            double3 noise(x, y, z);
                            double3 v_ = v;
                            v_ += noise * s;
                            v_.Normalize();
                            int face = 0;
                            double2 uv_ = cube2uv(v_, &face);
                            p += FetchTexture(inputTex, uv_, face);
                        }
                    }
                }
                p /= iterationCount;
                p /= iterationCount;
                p /= iterationCount;
                //p = p * (k!=5) + fpixel(((float)i)/outputTex.m_width,((float)j)/outputTex.m_width,1.0) * (k==5);
                WriteTexture(outputTex, uv, k, p);
            }
        }
    }
    outputTex.m_cubemap = true;
}


void FastBlurCubemap::DoTask(const Texture& inputTex, Texture& outputTex)
{
    if (!inputTex.m_cubemap)
    {
        printf("Error: For this task required cubmap.\n");
        return;
    }

    int radius = m_blurRadius;
    int kernelSize = 2 * radius + 1;
    std::vector<double> kernel = GenerateKernel(1.0 / 3.0 * radius, kernelSize, 1000.0);
    Texture tmpTex = outputTex;

    Texture tmpTex2 = outputTex;
    Texture resTex = outputTex;

    for(int k = 0; k <6; ++k)
	{
        tmpTex = outputTex;
        tmpTex2 = outputTex;

        for(int ki = 0; ki <6; ++ki)
        {
            for (int i = 0;i<outputTex.m_height;i++)
            {
                for (int j = 0;j<outputTex.m_width;j++)
                {
                    fpixel s(0.0, 0.0, 0.0);
                    for(int n = -radius; n < radius+1; ++n)
                    {
                        int j_ = j + n;
                        int k_ = ki;
                        if (ki != 2 && ki != 3)
                        {
                            if (j_ < 0)
                            {
                                j_ += outputTex.m_width;
                                switch(ki)
                                {
                                case 4:k_ = 0; break;
                                case 5:k_ = 1; break;
                                case 0:k_ = 5; break;
                                case 1:k_ = 4; break;
                                }
                            }
                            if (j_ >= outputTex.m_width)
                            {
                                j_ -= outputTex.m_width;
                                switch(ki)
                                {
                                case 0:k_ = 4; break;
                                case 1:k_ = 5; break;
                                case 5:k_ = 0; break;
                                case 4:k_ = 1; break;

                                }
                            }
                            s += outputTex.m_faces[k_].m_buff[j_ + i*outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            if(k!= 0 && k!= 1)
                            {
                                if (ki==2)
                                {
                                    if (j_ < 0)
                                    {
                                        j_ += outputTex.m_width;
                                        k_ = 0;
                                        s += outputTex.m_faces[k_].m_buff[(outputTex.m_width - i - 1) + j_*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else if (j_ >= outputTex.m_width)
                                    {
                                        j_ -= outputTex.m_width;
                                        j_ = outputTex.m_height - j_ - 1;
                                        k_ = 1;
                                        s += outputTex.m_faces[k_].m_buff[i + j_*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else
                                    {
                                        s += outputTex.m_faces[k_].m_buff[j_ + i*outputTex.m_width] * kernel[n + radius];
                                    }
                                }
                                else//ki = 3
                                {
                                    if (j_ < 0)
                                    {
                                        j_ += outputTex.m_width;
                                        k_ = 0;
                                        s += outputTex.m_faces[k_].m_buff[i + (outputTex.m_height - j_ - 1)*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else if (j_ >= outputTex.m_width)
                                    {
                                        j_ -= outputTex.m_width;
                                        j_ = outputTex.m_height - j_ - 1;
                                        k_ = 1;
                                        s += outputTex.m_faces[k_].m_buff[(outputTex.m_width - i - 1) + (outputTex.m_height - j_ - 1)*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else
                                    {
                                        s += outputTex.m_faces[k_].m_buff[j_ + i*outputTex.m_width] * kernel[n + radius];
                                    }
                                }
                            }
                            else
                            {
                                int i_ = i + n;
                                int k_ = ki;
                                if (ki == 2)
                                {
                                    if (i_ < 0)
                                    {
                                        i_ += outputTex.m_height;
                                        k_ = 4;
                                        s += outputTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else if (i_ >= outputTex.m_width)
                                    {
                                        i_ -= outputTex.m_width;
                                        i_ = outputTex.m_height - i_ - 1;
                                        k_ = 5;
                                        s += outputTex.m_faces[k_].m_buff[(outputTex.m_width - j - 1) + i_*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else
                                    {
                                        s += outputTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                                    }
                                }
                                if (ki == 3)
                                {
                                    if (i_ < 0)
                                    {
                                        i_ += outputTex.m_height;
                                        i_ = outputTex.m_height - i_ - 1;
                                        k_ = 5;
                                        s += outputTex.m_faces[k_].m_buff[(outputTex.m_width - j - 1) + i_*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else if (i_ >= outputTex.m_width)
                                    {
                                        i_ -= outputTex.m_width;
                                        k_ = 4;
                                        s += outputTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                                    }
                                    else
                                    {
                                        s += outputTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                                    }
                                }
                            }
                        }
                    }
                    tmpTex.m_faces[ki].m_buff[j + i*outputTex.m_width]  = s;
                }
            }
        }


        for (int j = 0 ;j<outputTex.m_width;j++)
        {
            for (int i = 0;i<outputTex.m_height;i++)
            {
                fpixel s(0.0, 0.0, 0.0);
                for(int n = -radius; n < radius+1; ++n)
                {
                    int i_ = i + n;
                    int k_ = k;
                    if (k == 2)
                    {
                        if (i_ < 0)
                        {
                            i_ += outputTex.m_height;
                            k_ = 4;
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else if (i_ >= outputTex.m_width)
                        {
                            i_ -= outputTex.m_width;
                            i_ = outputTex.m_height - i_ - 1;
                            k_ = 5;
                            s += tmpTex.m_faces[k_].m_buff[(outputTex.m_width - j - 1) + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                    }
                    if (k == 3)
                    {
                        if (i_ < 0)
                        {
                            i_ += outputTex.m_height;
                            i_ = outputTex.m_height - i_ - 1;
                            k_ = 5;
                            s += tmpTex.m_faces[k_].m_buff[(outputTex.m_width - j - 1) + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else if (i_ >= outputTex.m_width)
                        {
                            i_ -= outputTex.m_width;
                            k_ = 4;
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                    }
                    else if(k == 5)
                    {
                        if (i_ < 0)
                        {
                            i_ += outputTex.m_height;
                            i_ = outputTex.m_height - i_ - 1;
                            k_ = 3;
                            s += tmpTex.m_faces[k_].m_buff[(outputTex.m_width - j - 1) + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else if (i_ >= outputTex.m_width)
                        {
                            i_ -= outputTex.m_width;
                            i_ = outputTex.m_height - i_ - 1;
                            k_ = 2;
                            s += tmpTex.m_faces[k_].m_buff[(outputTex.m_width - j - 1) + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                    }
                    else if(k == 4)
                    {
                        if (i_ < 0)
                        {
                            i_ += outputTex.m_height;
                            k_ = 3;
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else if (i_ >= outputTex.m_width)
                        {
                            i_ -= outputTex.m_width;
                            k_ = 2;
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                    }
                    else if(k == 0)
                    {
                        if (i_ < 0)
                        {
                            i_ += outputTex.m_height;
                            i_ = outputTex.m_width - 1 - i_;
                            k_ = 3;
                            s += tmpTex.m_faces[k_].m_buff[i_ + j * outputTex.m_width] * kernel[n + radius];
                        }
                        else if (i_ >= outputTex.m_width)
                        {
                            i_ -= outputTex.m_width;
                            k_ = 2;
                            s += tmpTex.m_faces[k_].m_buff[i_ + (outputTex.m_height - j - 1)*outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                    }
                    else if(k == 1)
                    {
                        if (i_ < 0)
                        {
                            i_ += outputTex.m_height;
                            k_ = 3;
                            s += tmpTex.m_faces[k_].m_buff[i_ + (outputTex.m_height - j - 1) * outputTex.m_width] * kernel[n + radius];
                        }
                        else if (i_ >= outputTex.m_width)
                        {
                            i_ -= outputTex.m_width;
                            i_ = outputTex.m_width - 1 - i_;
                            k_ = 2;
                            s += tmpTex.m_faces[k_].m_buff[i_ + j * outputTex.m_width] * kernel[n + radius];
                        }
                        else
                        {
                            s += tmpTex.m_faces[k_].m_buff[j + i_*outputTex.m_width] * kernel[n + radius];
                        }
                    }
                }
                tmpTex2.m_faces[k].m_buff[j + i*outputTex.m_width]  = s;
            }
        }
        resTex.m_faces[k] = tmpTex2.m_faces[k];
	}

	outputTex = resTex;
    outputTex.m_cubemap = true;
}

double gaussianDistribution(double x, double mu, double sigma)
{
    double d = x - mu;
    double n = 1.0 / (sqrt(2 * 3.141592654) * sigma);
    return exp(-d*d/(2 * sigma * sigma)) * n;
};

std::vector<std::pair<double, double> > sampleInterval(double sigma, double minInclusive, double maxInclusive, int sampleCount)
{
    std::vector<std::pair<double, double> > result;
    double stepSize = (maxInclusive - minInclusive) / (sampleCount-1);

    for(int s=0; s<sampleCount; ++s)
    {
        double x = minInclusive + s * stepSize;
        double y = gaussianDistribution(x, 0, sigma);
        result.push_back(std::make_pair(x, y));
    }
    return result;
};

double integrateSimphson(const std::vector<std::pair<double, double> >& samples)
{
    double result = samples[0].second + samples[samples.size()-1].second;
    for(int s = 1; s < samples.size()-1; ++s)
    {
        double sampleWeight = (s % 2 == 0) ? 2.0 : 4.0;
        result += sampleWeight * samples[s].second;
    }
    double h = (samples[samples.size()-1].first - samples[0].first) / (samples.size()-1);
    return result * h / 3.0;
};

double roundTo(double num, int decimals)
{
    double shift = pow(10, decimals);
    return Round(num * shift) / shift;
};

std::vector<std::pair<double, double> > calcSamplesForRange(double minInclusive, double maxInclusive, double sigma, int samplesPerBin)
{
    return sampleInterval(
        sigma,
        minInclusive,
        maxInclusive,
        samplesPerBin
    );
}

std::vector<double> GenerateKernel(double sigma, int kernelSize, int sampleCount)
{
    int samplesPerBin = ceil(sampleCount / kernelSize);
    if(samplesPerBin % 2 == 0)
        ++samplesPerBin;
    double weightSum = 0;
    int kernelLeft = -floor(kernelSize/2);

    std::vector<std::pair<double, double> > outsideSamplesLeft  = calcSamplesForRange(-5 * sigma, kernelLeft - 0.5, sigma, samplesPerBin);
    std::vector<std::pair<double, double> > outsideSamplesRight = calcSamplesForRange(-kernelLeft+0.5, 5 * sigma, sigma, samplesPerBin);

    std::vector<std::pair<std::vector<std::pair<double, double> >, double> > allSamples;
    allSamples.push_back(std::make_pair(outsideSamplesLeft, 0.0));

    for(int tap=0; tap<kernelSize; ++tap)
    {
        double left = kernelLeft - 0.5 + tap;

        std::vector<std::pair<double, double> > tapSamples = calcSamplesForRange(left, left+1, sigma, samplesPerBin);
        double tapWeight = integrateSimphson(tapSamples);
        allSamples.push_back(std::make_pair(tapSamples, tapWeight));
        weightSum += tapWeight;
    }

    allSamples.push_back(std::make_pair(outsideSamplesRight, 0.0));

    for(int i=0; i<allSamples.size(); ++i)
    {
        allSamples[i].second = roundTo(allSamples[i].second / weightSum, 6);
    }

    std::vector<double> weights;
    for(int i=1; i<allSamples.size()-1; ++i)
    {
        weights.push_back(allSamples[i].second);
    }
    return weights;
}

