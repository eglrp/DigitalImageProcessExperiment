#include "stdafx.h"
#include "ImageProcess.h"
#include <vector>
#include <algorithm>

static bool GetValue(int p[], int size, int &value)
{
	//�����м��ֵ
	int zxy = p[(size - 1) / 2];
	//���ڼ�¼ԭ������±�
	int *a = new int[size];
	int index = 0;
	for (int i = 0; i < size; ++i)
		a[index++] = i;

	for (int i = 0; i < size - 1; i++)
		for (int j = i + 1; j < size; j++)
			if (p[i] > p[j]) {
				int tempA = a[i];
				a[i] = a[j];
				a[j] = tempA;
				int temp = p[i];
				p[i] = p[j];
				p[j] = temp;

			}
	int zmax = p[size - 1];
	int zmin = p[0];
	int zmed = p[(size - 1) / 2];

	if (zmax > zmed&&zmin < zmed) {
		if (zxy > zmin&&zxy < zmax)
			value = (size - 1) / 2;
		else
			value = a[(size - 1) / 2];
		delete[]a;
		return true;
	}
	else {
		delete[]a;
		return false;
	}

}

//ʹ������ֵ������ͼ������ֵ
int normaliseXY(int x, int y, int max_x, int max_y) {
	if (x >= max_x || x <= 0 || y >= max_y || y <= 0) //�����߽�Ĳ����ú�ɫ���
		return 0;
	return x;
}

UINT ImageProcess::medianFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int maxSpan = param->maxSpan;
	int maxLength = (maxSpan * 2 + 1) * (maxSpan * 2 + 1);

	byte* pRealData = (byte*)param->src->GetBits();
	int pit = param->src->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	int *pixel = new int[maxLength];//�洢ÿ�����ص�ĻҶ�
	int *pixelR = new int[maxLength];
	int *pixelB = new int[maxLength];
	int *pixelG = new int[maxLength];
	int index = 0;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int Sxy = 1;
		int med = 0;
		int state = 0;
		int x = i % maxWidth;
		int y = i / maxWidth;
		while (Sxy <= maxSpan)
		{
			index = 0;
			for (int tmpY = y - Sxy; tmpY <= y + Sxy && tmpY < maxHeight; tmpY++)
			{
				if (tmpY < 0) continue;
				for (int tmpX = x - Sxy; tmpX <= x + Sxy && tmpX < maxWidth; tmpX++)
				{
					if (tmpX < 0) continue;
					if (bitCount == 1)
					{
						pixel[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixelR[index++] = pixel[index];

					}
					else
					{
						pixelR[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 2);
						pixelG[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 1);
						pixelB[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);

					}
				}

			}
			if (index <= 0)
				break;
			if ((state = GetValue(pixel, index, med)) == 1)
				break;

			Sxy++;
		};

		if (state)
		{
			if (bitCount == 1)
			{
				*(pRealData + pit*y + x*bitCount) = pixelR[med];

			}
			else
			{
				*(pRealData + pit*y + x*bitCount + 2) = pixelR[med];
				*(pRealData + pit*y + x*bitCount + 1) = pixelG[med];
				*(pRealData + pit*y + x*bitCount) = pixelB[med];

			}
		}

	}



	delete[]pixel;
	delete[]pixelR;
	delete[]pixelG;
	delete[]pixelB;

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_MEDIAN_FILTER, 1, NULL);
	return 0;
}

//�ڶ��̳߳����в�����ȫ�ֱ���������ʱ������
//***
UINT ImageProcess::rotatePicture(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	//�������
	double theta = 15 * DegreesToRadians;// ��ת�ĽǶ�
	short type = 0;	//0 ������ڲ�ֵ ;1 ���ײ�ֵ

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)param->origin_src->GetBits();//param->origin_srcָ����ԭͼ�����ﲻ��������в���
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();
	for (int i = startIndex; i <= endIndex; ++i)//ɨ��������ص�λ�ã�����ÿһ��λ�ã�x,y��ʹ��(v,w) = T^(-1)(x,y)��������ͼ���е���Ӧλ��
	{
		//x,y�����ص�����?
		int x = i % maxWidth;
		int y = i / maxWidth;

		//v,w��ԭͼ�������ص�����
		//int v = x*cos(theta) + y*sin(theta);
		//int w = (-1)*x*sin(theta) + y*cos(theta);
		int v = x*cos(theta) + y*sin(theta) + cos(theta)*(-1)*(maxWidth / 2) + sin(theta)*(-1)*(maxHeight / 2) - (-1)*maxWidth / 2;
		int w = (-1)*x*sin(theta) + y*cos(theta) + cos(theta)*(-1)*(maxHeight / 2) - sin(theta)*(-1)*(maxWidth / 2) - (-1)*maxHeight / 2;
		//��ÿһ��λ�ã�x,y��ʹ��(v,w) = T^(-1)(x,y)��������ͼ���е���Ӧλ�� 
		int value;
		for (int off = 0; off < bitCount; off++) {//RGBͼ���������ͨ����ֵ
			if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0) {//��û�����߽�Ļ�
				if (type == 0)//������ڲ�ֵ
					value = *(pOriginData + pit * w + v * bitCount + off);
				else {//���ײ�ֵ
					double temp = 0;
					if (v >= 1 && v < maxWidth - 1 && w >= 1 && w < maxHeight - 1)//��ֹԽ��
						for (int i = -1; i < 2; i++)//MaskӦ��
							for (int j = -1; j < 2; j++)
								temp += *(pOriginData + pit * (w + i) + (v + j) * bitCount + off);
					value = temp / 9;
				}
			}
			*(pRealData + pit * y + x * bitCount + off) = value;
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}

UINT ImageProcess::zoomPicture(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	//�������
	double factor = 0.5;// ��ת�ĽǶ�
	short type = 0;//0 ������ڲ�ֵ ;1 ���ײ�ֵ

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)param->origin_src->GetBits();//param->origin_srcָ����ԭͼ�����ﲻ��������в���
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();
	for (int i = startIndex; i <= endIndex; ++i)//ɨ��������ص�λ�ã�����ÿһ��λ�ã�x,y��ʹ��(v,w) = T^(-1)(x,y)��������ͼ���е���Ӧλ��
	{
		//x,y�����ص�����?
		int x = i % maxWidth;
		int y = i / maxWidth;
		//v,w��ԭͼ�������ص�����
		int v = x*factor;
		int w = y*factor;
		//��ÿһ��λ�ã�x,y��ʹ��(v,w) = T^(-1)(x,y)��������ͼ���е���Ӧλ�� 
		int value;
		for (int off = 0; off < bitCount; off++) {//RGBͼ���������ͨ����ֵ
			if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0) {//��û�����߽�Ļ�
				if (type == 0)//������ڲ�ֵ
					value = *(pOriginData + pit * w + v * bitCount + off);
				else {
					double temp = 0;
					if (v >= 1 && v < maxWidth - 1 && w >= 1 && w < maxHeight - 1)//��ֹԽ��
						for (int i = -1; i < 2; i++)//MaskӦ��
							for (int j = -1; j < 2; j++)
								temp += *(pOriginData + pit * (w + i) + (v + j) * bitCount + off);
					value = temp / 9;
				}
			}
			*(pRealData + pit * y + x * bitCount + off) = value;
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}

UINT ImageProcess::addNoise(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//ͼ���������Ⱥ͸߶�
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	//GetBits() �����������ָ��
	byte* pRealData = (byte*)param->src->GetBits();
	//GetBPP() ����ÿ��������ռ��bit����������/8��ζ��bitCount��ÿ��������ռ��byte��
	int bitCount = param->src->GetBPP() / 8;
	//���ͼ������ÿһ�е��ֽ���
	//The pitch of the image.If the return value is negative, the bitmap is a bottom - up DIB and its origin is the lower left corner.
	//	If the return value is positive, the bitmap is a top - down DIB and its origin is the upper left corner.
	int pit = param->src->GetPitch();

	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		if ((rand() % 1000) * 0.001 < NOISE)//����¼����������
		{
			int value = 0;
			if (rand() % 1000 < 500)
			{
				value = 0;//�ڵ�
			}
			else
			{
				value = 255;//�׵�
			}
			if (bitCount == 1)//����255λ��ɫͼ��
			{
				*(pRealData + pit * y + x * bitCount) = value;
			}
			else//����ָ�ľ���RGB 255*255*255���ɫ
			{
				*(pRealData + pit * y + x * bitCount) = value;
				*(pRealData + pit * y + x * bitCount + 1) = value;
				*(pRealData + pit * y + x * bitCount + 2) = value;
			}
		}
	}
	//???
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}