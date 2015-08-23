#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "CoordinateTransform.h"
#include "Actions/Actions.h"
#include "GaussianWeights.h"
#include <stdio.h>

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
