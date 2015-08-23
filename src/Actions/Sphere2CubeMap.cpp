#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "CoordinateTransform.h"
#include "Actions/Actions.h"
#include <stdio.h>

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
