#pragma once

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