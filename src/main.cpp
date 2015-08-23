#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "Actions/Actions.h"

#include <string>
#include <tclap/CmdLine.h>

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
	else if (formatString == "DDS")
	{
		format = new DDSFile;
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
