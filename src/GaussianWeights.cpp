#include "GaussianWeights.h"
#include "EnvMapMath.h"

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

