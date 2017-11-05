#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "afxdialogex.h"
#include <boost/thread/thread.hpp>

//�����������й��̺߳�ͼ��Ԥ����Ĳ���

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
	case 2://cuda
		break;
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
	//�������Ҫ��ͼ�����ݽ��и�����
	//���¹����³ߴ��ͼ
	int origin_BPP = m_pImgSrc->GetBPP();
	double factor = _ttof(cStr);
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
			CString timeStr;
			timeStr.Format(_T("����%d��,��ʱ:%dms"), circulation, endTime - startTime);
			//����Ϣ�е���ʽչʾ���Ƚ����Ϊ����ڳ����·��ĵ�����Ϣ��Ŀ
			//AfxMessageBox(timeStr);
			mOutputInfo.SetWindowTextW(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}
