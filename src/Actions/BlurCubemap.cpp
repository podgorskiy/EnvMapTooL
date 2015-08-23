#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "CoordinateTransform.h"
#include "Actions/Actions.h"
#include <stdio.h>

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
