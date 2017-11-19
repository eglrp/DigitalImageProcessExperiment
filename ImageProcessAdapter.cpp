#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "afxdialogex.h"
#include <boost/thread/thread.hpp>
#include <CL/cl.h>

//�����������й��̺߳�ͼ��Ԥ����Ĳ���

//extern "C" void add_host(int *host_a, int *host_b, int *host_c); //interface for kernel function
extern "C" void rotate_and_scale_host(int *pixel, int Width, int Height, double angle, double factor);
extern "C" void white_balance_host(int *pixel, int Width, int Height, double color_sum, double RGB_sum);


void CExperimentImgDlg::AddNoise()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();
	switch (thread)
	{
	case 0://win���߳�
	{
		AddNoise_WIN();
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].src = m_pImgSrc;
			ImageProcess::addNoise(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
		break;
	}
}
void CExperimentImgDlg::MedianFilter()
{
	//	AfxMessageBox(_T("��ֵ�˲�"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;

	startTime = CTime::GetTickCount();
	m_nThreadNum;
	switch (thread)
	{
	case 0://win���߳�
	{
		MedianFilter_WIN();
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = m_pImgSrc;
			ImageProcess::medianFilter(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
		break;
	}
}
void CExperimentImgDlg::WhiteBalance()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();
	switch (thread)
	{
	case 0://win���߳�
	{
		WhiteBalance_WIN();
	}
	break;
	case 1://openmp
	{
		//�����һ��Ԥ����
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int pit = m_pImgSrc->GetPitch();
		int maxWidth = m_pImgSrc->GetWidth();
		int maxHeight = m_pImgSrc->GetHeight();
		//������ͼ�����Ԥ��������ͳ����R��G��B�����ܺͣ�
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		double R = 0, G = 0, B = 0;//����ͼ���RGB�����ܺ�
		int endIndex = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		for (int i = 0; i <= endIndex; ++i)
		{
			int x = i % maxWidth;
			int y = i / maxWidth;
			B += *(pRealData + pit * y + x * bitCount + 0);
			G += *(pRealData + pit * y + x * bitCount + 1);
			R += *(pRealData + pit * y + x * bitCount + 2);
		}

		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].origin_src = m_pImgCpy;
			m_pThreadParam[i].R_sum = R;
			m_pThreadParam[i].G_sum = G;
			m_pThreadParam[i].B_sum = B;
			ImageProcess::whiteBalance(&m_pThreadParam[i]);
		}
	}
	break;
	case 2://cuda


		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;

		//�����һ��Ԥ����
		double  R = 0, G = 0, B = 0;//����ͼ���RGB�����ܺ�
		int endIndex = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		for (int i = 0; i <= endIndex; ++i)
		{
			int x = i % width;
			int y = i / width;
			B += *(pRealData + pit * y + x * bitCount + 0);
			G += *(pRealData + pit * y + x * bitCount + 1);
			R += *(pRealData + pit * y + x * bitCount + 2);
		}

		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			white_balance_host(pixel, width, height, R, R + G + B);//�Ҷ�ͼ��ô��ƽ��
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			white_balance_host(pixelR, width, height, R, R + G + B);
			white_balance_host(pixelG, width, height, G, R + G + B);
			white_balance_host(pixelB, width, height, B, R + G + B);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}
		CTime endTime = CTime::GetTickCount();
		CString timeStr;
		timeStr.Format(_T("����%d��,��ʱ:%dms"), circulation, endTime - startTime);
		mOutputInfo.SetWindowTextW(timeStr);
		AfxMessageBox(_T("finish!"));
		break;
	}
}
void CExperimentImgDlg::BilateralFilter()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();
	//���˫�����˲��Ĳ���
	CEdit * cEdit_mask = ((CEdit*)GetDlgItem(IDC_MASK));
	CEdit * cEdit_sigma_d = ((CEdit*)GetDlgItem(IDC_SIGMAD));
	CEdit * cEdit_sigma_r = ((CEdit*)GetDlgItem(IDC_SIGMAR));
	CString mask_cStr;
	CString sigma_d_cStr;
	CString sigma_r_cStr;
	cEdit_mask->GetWindowTextW(mask_cStr);
	cEdit_sigma_d->GetWindowTextW(sigma_d_cStr);
	cEdit_sigma_r->GetWindowTextW(sigma_r_cStr);
	//
	BilateralFilterParam * bilateralFilterParam = new BilateralFilterParam();
	bilateralFilterParam->filter_length = _ttoi(mask_cStr);
	bilateralFilterParam->sigma_d = _ttof(sigma_d_cStr);
	bilateralFilterParam->sigma_r = _ttof(sigma_r_cStr);

	bilateralFilter(bilateralFilterParam);
}
void CExperimentImgDlg::ImageFusion()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files ��*.*��|*.*||");
	CString filePath("");

	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);

		if (m_pImgCpy != NULL)
		{
			m_pImgCpy->Destroy();
			delete m_pImgCpy;
		}
		m_pImgCpy = new CImage();
		m_pImgCpy->Load(strFilePath);
		CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
		int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
		startTime = CTime::GetTickCount();
		//ʵ����boost����̺߳�win���߳�
		CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_ALPHA);
		switch (thread) {
		case 0: //win���߳�
			ImageFusion_WIN(slider->GetPos() / 100.0);
			break;
		default:
			ImageFusion_BOOST(slider->GetPos() / 100.0);
		}
		this->Invalidate();
	}
}
void CExperimentImgDlg::ColorBalance()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();

	//���MFC�ĽǶ�ֵ
	CEdit * cEdit_shadow = ((CEdit*)GetDlgItem(IDC_SHADOW));
	CEdit * cEdit_hightlight = ((CEdit*)GetDlgItem(IDC_HIGHLIGHT));
	CString shadow_cStr;
	CString highlight_cStr;
	cEdit_shadow->GetWindowTextW(shadow_cStr);
	cEdit_hightlight->GetWindowTextW(highlight_cStr);

	ColorBalanceParam * colorBalanceParam = new ColorBalanceParam();
	colorBalanceParam->shadow = _ttoi(shadow_cStr);
	colorBalanceParam->highlight = _ttoi(highlight_cStr);
	colorBalanceParam->midtones = 0.5;
	colorBalanceParam->outHightlight = 255;
	colorBalanceParam->outShadow = 0;

	switch (thread)
	{
	case 0://win���߳�
	{
		ColorBalance_WIN(colorBalanceParam);
	}
	break;
	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].origin_src = m_pImgCpy;
			m_pThreadParam[i].colorBalanceParam = colorBalanceParam;
			ImageProcess::colorBalance(&m_pThreadParam[i]);
		}
	}
	break;
	case 2://cuda
		break;
	}
}
void CExperimentImgDlg::RotateImage()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();
	timeStart = clock();
	//���MFC�ĽǶ�ֵ
	CEdit * cEdit_angle = ((CEdit*)GetDlgItem(IDC_ANGLE));
	CString cStr;
	cEdit_angle->GetWindowTextW(cStr);
	double angle = _ttof(cStr);
	switch (thread)
	{
	case 0://win���߳�
	{
		Rotate_WIN(angle);
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
		int h = m_pImgSrc->GetHeight() / m_nThreadNum;
		int w = m_pImgSrc->GetWidth() / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].origin_src = m_pImgCpy;
			m_pThreadParam[i].angle = angle;
			ImageProcess::rotatePicture(&m_pThreadParam[i]);
		}
	}
	break;
	case 2://cuda �����ͼ������һ������
		//���������ֵ
	{


		CEdit * cEdit = ((CEdit*)GetDlgItem(IDC_ZOOM));
		CString cStr;
		cEdit->GetWindowTextW(cStr);
		double factor = _ttof(cStr);

		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			rotate_and_scale_host(pixel, width, height, angle, factor);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			rotate_and_scale_host(pixelR, width, height, angle, factor);
			rotate_and_scale_host(pixelG, width, height, angle, factor);
			rotate_and_scale_host(pixelB, width, height, angle, factor);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}

		timeEnd = clock();
		CString timeStr;
		timeStr.Format(_T("����%d��,��ʱ:%gms"), circulation, timeEnd - timeStart);
		mOutputInfo.SetWindowTextW(timeStr);
		AfxMessageBox(_T("finish!"));
	}
	break;
	case 3://opencl
	{
		CEdit * cEdit = ((CEdit*)GetDlgItem(IDC_ZOOM));
		CString cStr;
		cEdit->GetWindowTextW(cStr);
		double factor = _ttof(cStr);

		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			Rotate_And_Scale_CL(pixel, pixelIndex,width, height, angle, factor);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			Rotate_And_Scale_CL(pixelR, pixelIndex,width, height, angle, factor);
			Rotate_And_Scale_CL(pixelG, pixelIndex,width, height, angle, factor);
			Rotate_And_Scale_CL(pixelB, pixelIndex,width, height, angle, factor);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}

		timeEnd = clock();
		CString timeStr;
		timeStr.Format(_T("����%d��,��ʱ:%gms"), circulation, timeEnd - timeStart);
		mOutputInfo.SetWindowTextW(timeStr);
		AfxMessageBox(_T("finish!"));
	}
	}

}
void CExperimentImgDlg::ZoomImage()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();
	//���������ֵ
	CEdit * cEdit = ((CEdit*)GetDlgItem(IDC_ZOOM));
	CString cStr;
	cEdit->GetWindowTextW(cStr);
	double factor = _ttof(cStr);
	//�������Ҫ��ͼ�����ݽ��и�����
	//���¹����³ߴ��ͼ
	int origin_BPP = m_pImgSrc->GetBPP();
	int new_Width = ceil((m_pImgSrc->GetWidth()* factor));
	int new_Height = ceil((m_pImgSrc->GetHeight()* factor));
	//�����µ�ͼƬ��С��BPP����һ��
	if (!m_pImgSrc->IsNull())
	{
		m_pImgSrc->Destroy();
		//delete m_pImgSrc;
	}
	//m_pImgSrc = new CImage();
	m_pImgSrc->Create(new_Width, new_Height, origin_BPP, 0);
	this->Invalidate();
	switch (thread)
	{
	case 0://win���߳�
	{
		Zoom_WIN(factor);
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
		int h = m_pImgSrc->GetHeight() / m_nThreadNum;
		int w = m_pImgSrc->GetWidth() / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].origin_src = m_pImgCpy;
			m_pThreadParam[i].zoom_factor = factor;
			ImageProcess::zoomPicture(&m_pThreadParam[i]);
		}
	}
	break;
	case 2://cuda
		break;
	}
}
//windows���߳�
void CExperimentImgDlg::AddNoise_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		//�����þŹ�����������ȥ�ֿ飬���Ǵ��ķ�ʽ��
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::MedianFilter_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::medianFilter, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::Rotate_WIN(double angle)
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].angle = angle;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::rotatePicture, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::WhiteBalance_WIN()
{
	//�����һ��Ԥ����
	int bitCount = m_pImgSrc->GetBPP() / 8;
	int pit = m_pImgSrc->GetPitch();
	int maxWidth = m_pImgSrc->GetWidth();
	int maxHeight = m_pImgSrc->GetHeight();
	//������ͼ�����Ԥ��������ͳ����R��G��B�����ܺͣ�
	byte* pRealData = (byte*)m_pImgSrc->GetBits();
	double R = 0, G = 0, B = 0;//����ͼ���RGB�����ܺ�
	int endIndex = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
	for (int i = 0; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		B += *(pRealData + pit * y + x * bitCount + 0);
		G += *(pRealData + pit * y + x * bitCount + 1);
		R += *(pRealData + pit * y + x * bitCount + 2);
	}

	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].R_sum = R;
		m_pThreadParam[i].G_sum = G;
		m_pThreadParam[i].B_sum = B;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::whiteBalance, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::ImageFusion_WIN(double alpha)
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].alpha = alpha;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::imageFusion, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::ColorBalance_WIN(ColorBalanceParam * p)
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].colorBalanceParam = p;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::colorBalance, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::Zoom_WIN(double zoom_factor)
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].zoom_factor = zoom_factor;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::zoomPicture, &m_pThreadParam[i]);
	}
}
//boost���߳�
void CExperimentImgDlg::ImageFusion_BOOST(double alpha)
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].alpha = alpha;
		boost::thread thrd(&ImageProcess::imageFusion, &m_pThreadParam[i]);
		//AfxBeginThread((AFX_THREADPROC)&ImageProcess::imageFusion, &m_pThreadParam[i]);
	}
}
void CExperimentImgDlg::bilateralFilter(BilateralFilterParam *p)
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].origin_src = m_pImgCpy;
		m_pThreadParam[i].bilateralFilterParam = p;
		//ImageProcess::bilateralFilter(&m_pThreadParam[i]);
		boost::thread thrd(&ImageProcess::bilateralFilter, &m_pThreadParam[i]);
		//AfxBeginThread((AFX_THREADPROC)&ImageProcess::bilateralFilter, &m_pThreadParam[i]);
	}
}
LRESULT CExperimentImgDlg::OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// �������̶߳�������ֵ1����ȫ������~��ʾʱ��
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("��ʱ:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				MedianFilter_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("����%d��,��ʱ:%dms"), circulation, endTime - startTime);
				//����Ϣ�е���ʽչʾ���Ƚ����Ϊ����ڳ����·��ĵ�����Ϣ��Ŀ
				//AfxMessageBox(timeStr);
				mOutputInfo.SetWindowTextW(timeStr);
			}
			// ��ʾ��Ϣ����
			//			AfxMessageBox(timeStr);
		}
	}
	return 0;
}
LRESULT CExperimentImgDlg::OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("��ʱ:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
			AddNoise_WIN();
		else
		{
			tempProcessCount = 0;
			CTime endTime = CTime::GetTickCount();
			timeEnd = clock();
			CString timeStr;
			timeStr.Format(_T("����%d��,��ʱ:%gms"), circulation, timeEnd - timeStart);
			//timeStr.Format(_T("����%d��,��ʱ:%dms"), circulation, endTime - startTime);
			mOutputInfo.SetWindowTextW(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

void CExperimentImgDlg::Rotate_And_Scale_CL(int *pixel, int *pixelIndex, int width, int height,double angle,double factor)
{
	//step 1:get platform;
	cl_int ret;														//errcode;
	cl_uint num_platforms;											//���ڱ����ȡ����platforms����;
	ret = clGetPlatformIDs(0, NULL, &num_platforms);
	if ((CL_SUCCESS != ret) || (num_platforms < 1))
	{
		CString errMsg;
		errMsg.Format(_T("Error getting platform number : %d"), ret);
		AfxMessageBox(errMsg);
		return;
	}
	cl_platform_id platform_id = NULL;
	ret = clGetPlatformIDs(1, &platform_id, NULL);					//��ȡ��һ��platform��id;
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error getting platform id:%d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 2:get device ;
	cl_uint num_devices;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
	if ((CL_SUCCESS != ret) || (num_devices < 1))
	{
		CString errMsg;
		errMsg.Format(_T("Error getting GPU device number:%d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	cl_device_id device_id = NULL;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error getting GPU device id: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 3:create context;
	cl_context_properties props[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0
	};
	cl_context context = NULL;
	context = clCreateContext(props, 1, &device_id, NULL, NULL, &ret);
	if ((CL_SUCCESS != ret) || (NULL == context))
	{
		CString errMsg;
		errMsg.Format(_T("Error createing context: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 4:create command queue;						//һ��device�ж��queue��queue֮�䲢��ִ��
	cl_command_queue command_queue = NULL;
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	if ((CL_SUCCESS != ret) || (NULL == command_queue))
	{
		CString errMsg;
		errMsg.Format(_T("Error createing command queue: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 5:create memory object;						//�������ͣ�buffer����ͼ�����ͣ�iamge��

	cl_mem mem_obj = NULL;
	cl_mem mem_objout = NULL;

	//create opencl memory object using host ptr
	//	mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buffer, &ret);
	mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, width * height * sizeof(int), pixel, &ret);
	mem_objout = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, width * height * sizeof(int), pixelIndex, &ret);
	if ((CL_SUCCESS != ret) || (NULL == mem_obj))
	{
		CString errMsg;
		errMsg.Format(_T("Error creating memory object: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 6:create program;
	size_t szKernelLength = 0;
	//	const char* oclSourceFile = "add_vector.cl";
	const char* oclSourceFile = "rotate_and_scale.cl";
	const char* kernelSource = LoadProgSource(oclSourceFile, "", &szKernelLength);
	if (kernelSource == NULL)
	{
		CString errMsg;
		errMsg.Format(_T("Error loading source file: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//create program
	cl_program program = NULL;
	program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, &ret);
	if ((CL_SUCCESS != ret) || (NULL == program))
	{
		CString errMsg;
		errMsg.Format(_T("Error creating program: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//build program 
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (CL_SUCCESS != ret)
	{
		size_t len;
		char buffer[8 * 1024];
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		CString errMsg = (CString)buffer;
		AfxMessageBox(errMsg);
		return;
	}

	//step 7:create kernel;
	cl_kernel kernel = NULL;
	//	kernel = clCreateKernel(program, "test", &ret);
	kernel = clCreateKernel(program, "rotate_and_scale", &ret);
	if ((CL_SUCCESS != ret) || (NULL == kernel))
	{
		CString errMsg;
		errMsg.Format(_T("Error creating kernel: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 8:set kernel arguement;
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)& mem_obj);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 0: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)& mem_objout);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 1: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 2, sizeof(int), (void*)& width);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 2: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 3, sizeof(int), (void*)& height);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 3: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 4, sizeof(double), (void*)& angle);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 4: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 5, sizeof(double), (void*)& factor);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 5: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	//step 9:set work group size;  							//<---->dimBlock\dimGrid
	cl_uint work_dim = 2;
	size_t local_work_size[2] = { 32, 32 };
	size_t global_work_size[2] = { RoundUp(local_work_size[0], width),
		RoundUp(local_work_size[1], height) };		//let opencl device determine how to break work items into work groups;

													//step 10:run kernel;				//put kernel and work-item arugement into queue and excute;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, work_dim, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error enqueue NDRange: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 11:get result;
	int *device_buffer = (int *)clEnqueueMapBuffer(command_queue, mem_objout, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, width * height * sizeof(int), 0, NULL, NULL, &ret);
	if ((CL_SUCCESS != ret) || (NULL == device_buffer))
	{
		CString errMsg;
		errMsg.Format(_T("Error map buffer: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	memcpy(pixel, device_buffer, width * height * sizeof(int));

	//step 12:release all resource;
	if (NULL != kernel)
		clReleaseKernel(kernel);
	if (NULL != program)
		clReleaseProgram(program);
	if (NULL != mem_obj)
		clReleaseMemObject(mem_obj);
	if (NULL != command_queue)
		clReleaseCommandQueue(command_queue);
	if (NULL != context)
		clReleaseContext(context);
	//	if (NULL != host_buffer)
	//		free(host_buffer);
}

char* CExperimentImgDlg::LoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength)
{
	FILE* pFileStream = NULL;
	size_t szSourceLength;

	// open the OpenCL source code file  
	pFileStream = fopen(cFilename, "rb");
	if (pFileStream == NULL)
	{
		return NULL;
	}

	size_t szPreambleLength = strlen(cPreamble);

	// get the length of the source code  
	fseek(pFileStream, 0, SEEK_END);
	szSourceLength = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET);

	// allocate a buffer for the source code string and read it in  
	char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1);
	memcpy(cSourceString, cPreamble, szPreambleLength);
	if (fread((cSourceString)+szPreambleLength, szSourceLength, 1, pFileStream) != 1)
	{
		fclose(pFileStream);
		free(cSourceString);
		return 0;
	}

	// close the file and return the total length of the combined (preamble + source) string  
	fclose(pFileStream);
	if (szFinalLength != 0)
	{
		*szFinalLength = szSourceLength + szPreambleLength;
	}
	cSourceString[szSourceLength + szPreambleLength] = '\0';

	return cSourceString;
}

size_t CExperimentImgDlg::RoundUp(int groupSize, int globalSize)
{
	int r = globalSize % groupSize;
	if (r == 0)
	{
		return globalSize;
	}
	else
	{
		return globalSize + groupSize - r;
	}
}
