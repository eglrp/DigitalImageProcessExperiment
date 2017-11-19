#pragma once
#pragma once
#define NOISE 0.2
//  Define M_PI in the case it's not defined in the math header file
#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif
//  Degrees-to-radians constant 
const double  DegreesToRadians = M_PI / 180.0;
//�Զ�ɫ�׾������ò���
struct ColorBalanceParam {
	int shadow;//����ڳ�
	int highlight;//����׳�
	double midtones;//����ҳ�
	int outHightlight;//����׳�
	int outShadow;//����ڳ�
};
struct BilateralFilterParam {
	//mask�ĳ���
	int filter_length;
	double sigma_d ;
	double sigma_r ;
};
struct ThreadParam
{
	CImage * src;
	CImage * origin_src;//�����Ķ���ͼ����ĳЩ�任�л��õ�
	int startIndex;
	int endIndex;
	int maxSpan;//Ϊģ�����ĵ���Ե�ľ���
	double angle;//��ת�ĽǶ�
	double zoom_factor;//��������
	double alpha;//�ϳɵ�ʱ���alphaֵ
	double R_sum;//Rͨ�������ܺ�
	double G_sum;
	double B_sum;
	ColorBalanceParam * colorBalanceParam;
	BilateralFilterParam * bilateralFilterParam;
};

static bool GetValue(int p[], int size, int &value);
int normaliseXY(int x, int y, int max_x, int max_y);
float BiCubicPoly(float x);
int checkColorSpace(double x);

class ImageProcess
{
public:
	static UINT medianFilter(LPVOID  param);
	static UINT bilateralFilter(LPVOID p);
	static UINT addNoise(LPVOID param);
	static UINT whiteBalance(LPVOID p);
	static UINT colorBalance(LPVOID p);
	static UINT imageFusion(LPVOID p);
	static UINT rotatePicture(LPVOID p);
	static UINT zoomPicture(LPVOID p);
};