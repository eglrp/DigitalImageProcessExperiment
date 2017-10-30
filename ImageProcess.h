#pragma once
#pragma once
#define NOISE 0.2
//  Define M_PI in the case it's not defined in the math header file
#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif
//  Degrees-to-radians constant 
const double  DegreesToRadians = M_PI / 180.0;
struct ThreadParam
{
	CImage * src;
	CImage * origin_src;//�����Ķ���ͼ����ĳЩ�任�л��õ�
	int startIndex;
	int endIndex;
	int maxSpan;//Ϊģ�����ĵ���Ե�ľ���
};

static bool GetValue(int p[], int size, int &value);

class ImageProcess
{
public:
	static UINT medianFilter(LPVOID  param);
	static UINT addNoise(LPVOID param);
	static UINT rotatePicture(LPVOID p);
	static UINT zoomPicture(LPVOID p);
};