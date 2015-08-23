#include "EnvMapMath.h"
#include "TextureUtils.h"
#include "CoordinateTransform.h"
#include "Actions/Actions.h"
#include <stdio.h>

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
