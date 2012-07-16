#include "AKMSSDKPointCloudGenerator.h"
#ifdef AIRKINECT_TARGET_MSSDK

void AKMSSDKPointCloudGenerator::setSourceDepthSize(int width, int height)
{
	AKPointCloudGenerator::setSourceDepthSize(width, height);
	_sourceDepthResolution = getResolutionFrom(width, height);
}

void AKMSSDKPointCloudGenerator::setSourceRGBSize(int width, int height)
{
	AKPointCloudGenerator::setSourceRGBSize(width, height);
	_sourceRGBResolution = getResolutionFrom(width, height);
}

void AKMSSDKPointCloudGenerator::setNuiSensor(INuiSensor* nuiSensor)
{
	_nuiSensor = nuiSensor;
}

void AKMSSDKPointCloudGenerator::generateTargetBytes()
{
	short* pointCloudRun = _targetBytes;
    
	if(_pointCloudRegions != 0)
	{
		for(int i = 0; i < _numRegions; i++)
		{
			_pointCloudRegions[i].numPoints = 0;
		}
	}
	else
	{
		_numRegions = 0;
	}

	unsigned int rgbValue;
	int rgbIndex;
	long depthX, depthY, rgbX, rgbY;
	unsigned short packedDepth, realDepth;
	for(int y = 0; y < _targetHeight; y+=_targetDensity)
	{
		depthY = y * _scale;

		const unsigned short* pDepthBuffer = _sourceDepthBytes + ((y + _directionFactor) * (_sourceDepthWidth * _scale)) - _directionFactor;
			
		for(int x = 0; x < _targetWidth; x+=_targetDensity)
		{
			packedDepth = *pDepthBuffer;
			realDepth = NuiDepthPixelToDepth(packedDepth);

			//write to point cloud
			*pointCloudRun = x;
			pointCloudRun++;
			*pointCloudRun = y;
			pointCloudRun++;
			*pointCloudRun = realDepth;
			pointCloudRun++;

			if(_includeRGB)
			{
				//calculate mirrored x
				depthX = (_direction * x * _scale) + (_directionFactor * _sourceDepthWidth);

				_nuiSensor->NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
					_sourceRGBResolution,
					_sourceDepthResolution,
					0,
					depthX, 
					depthY, 
					packedDepth,
					&rgbX,
					&rgbY
				);
				
				rgbIndex = (rgbX + (rgbY * _sourceRGBWidth));

				if(rgbIndex > _sourceRGBPixelCount)
				{
					* pointCloudRun = 0;
					pointCloudRun++;
					* pointCloudRun = 0;
					pointCloudRun++;
					* pointCloudRun = 0;
					pointCloudRun++;
				}
				else
				{
					rgbValue = _sourceRGBBytes[rgbIndex];
					* pointCloudRun = (USHORT) (0xff & (rgbValue >> 16));
					pointCloudRun++;
					* pointCloudRun = (USHORT) (0xff & (rgbValue >> 8));
					pointCloudRun++;
					* pointCloudRun = (USHORT) (0xff & rgbValue);
					pointCloudRun++;
				}
			}
            
			//check regions
			for(int i = 0; i < _numRegions; i++)
			{
				if(
					x >= _pointCloudRegions[i].x && x <= _pointCloudRegions[i].maxX &&
					y >= _pointCloudRegions[i].y && y <= _pointCloudRegions[i].maxY &&
					realDepth >= _pointCloudRegions[i].z && realDepth <= _pointCloudRegions[i].maxZ
					)
				{
					_pointCloudRegions[i].numPoints++;
				}
			}

			pDepthBuffer += (_scale * _direction * _targetDensity);
		}
	}
}

NUI_IMAGE_RESOLUTION AKMSSDKPointCloudGenerator::getResolutionFrom(int width, int height)
{
	NUI_IMAGE_RESOLUTION rtnRes = NUI_IMAGE_RESOLUTION_320x240;
	if(width == 80 && height == 60) rtnRes = NUI_IMAGE_RESOLUTION_80x60;
	if(width == 640 && height == 480) rtnRes = NUI_IMAGE_RESOLUTION_640x480;
	if(width == 1280 && height == 960) rtnRes = NUI_IMAGE_RESOLUTION_1280x960;
	return rtnRes;
}

#endif