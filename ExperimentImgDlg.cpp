
// ExperimentImgDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CExperimentImgDlg �Ի���



CExperimentImgDlg::CExperimentImgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENTIMG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//���ضԻ����ʱ���ʼ��
	m_pImgSrc = NULL;
	m_pImgCpy = NULL;
	m_nThreadNum = 1;
	m_pThreadParam = new ThreadParam[MAX_THREAD];
	srand(time(0));
}

void CExperimentImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT_INFO, mEditInfo);
	DDX_Control(pDX, IDC_PICTURE, mPictureControl);
	DDX_Control(pDX, IDC_CHECK_100, m_CheckCirculation);
	DDX_Control(pDX, IDC_ORIGIN_PICTURE, originPictureControl);
	DDX_Control(pDX, IDC_OUTPUT, mOutputInfo);
	DDX_Control(pDX, IDC_ANGLE, mEditAngle);
	DDX_Control(pDX, IDC_SHADOW, m_shadow);
	DDX_Control(pDX, IDC_HIGHLIGHT, m_highlight);
}

BEGIN_MESSAGE_MAP(CExperimentImgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CExperimentImgDlg::OnBnClickedButtonOpen)
//	ON_EN_CHANGE(IDC_EDIT1, &CExperimentImgDlg::OnEnChangeEdit1)
//	ON_EN_CHANGE(IDC_EDIT_INFO, &CExperimentImgDlg::OnEnChangeEditInfo)
ON_CBN_SELCHANGE(IDC_COMBO_FUNCTION, &CExperimentImgDlg::OnCbnSelchangeComboFunction)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_THREADNUM, &CExperimentImgDlg::OnNMCustomdrawSliderThreadnum)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_ALPHA, &CExperimentImgDlg::OnNMCustomdrawSliderAlpha)
ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CExperimentImgDlg::OnBnClickedButtonProcess)
ON_MESSAGE(WM_NOISE, &CExperimentImgDlg::OnNoiseThreadMsgReceived)
ON_MESSAGE(WM_MEDIAN_FILTER, &CExperimentImgDlg::OnMedianFilterThreadMsgReceived)
ON_BN_CLICKED(IDC_BUTTON_RESET, &CExperimentImgDlg::OnBnClickedButtonReset)
END_MESSAGE_MAP()


// CExperimentImgDlg ��Ϣ�������

BOOL CExperimentImgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
//	mEditInfo.SetWindowTextW(CString("File Path"));
	CComboBox * cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	cmb_function->InsertString(0, _T("�Զ�ɫ�׾���"));
	cmb_function->InsertString(1, _T("�Զ���ƽ��"));
	cmb_function->InsertString(2, _T("����"));
	cmb_function->InsertString(3, _T("˳ʱ����ת")); 
	cmb_function->InsertString(4, _T("��ֵ�˲�"));
	cmb_function->InsertString(5, _T("��������"));
	cmb_function->InsertString(6, _T("ͼ���ں�"));
	cmb_function->InsertString(7, _T("˫���˲�"));
	//cmb_function->AddString(_T("��������"));
	//cmb_function->AddString(_T("��ֵ�˲�"));
	//cmb_function->AddString(_T("˳ʱ����ת"));
	cmb_function->SetCurSel(0);

	CComboBox * cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	cmb_thread->InsertString(0, _T("WIN���߳�"));
	cmb_thread->InsertString(1, _T("OpenMP"));
	cmb_thread->InsertString(2, _T("CUDA"));
	cmb_thread->InsertString(3, _T("OpenCL"));
	cmb_thread->SetCurSel(0);

	CSliderCtrl * slider = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM));
	slider->SetRange(1, MAX_THREAD, TRUE);
	slider->SetPos(MAX_THREAD);

	CSliderCtrl * alpha_slider = ((CSliderCtrl*)GetDlgItem(IDC_ALPHA));
	alpha_slider->SetRange(0, 100, TRUE);
	alpha_slider->SetPos(50);

	AfxBeginThread((AFX_THREADPROC)&CExperimentImgDlg::Update, this);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CExperimentImgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

//���ݾ��εĴ�С�Զ�������С
void CExperimentImgDlg::AutoSizePaint(CStatic &picture_Control, CImage * img) {
	if (img != NULL){
		int height = img->GetHeight();
		int width = img->GetWidth();
		CRect rect;//����picture control��rect���󣬰������С��Ϣ
		CRect rect1;//mPictureControl�������ź��rect1����

		picture_Control.GetClientRect(&rect);
		CDC *pDC = picture_Control.GetDC();
		SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

		if (width <= rect.Width() && height <= rect.Width()){//��ͼ��Ĵ�С��picture controlҪС�����������
			rect1 = CRect(rect.TopLeft(), CSize(width, height));
			img->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
		}
		else{
			float xScale = (float)rect.Width() / (float)width;
			float yScale = (float)rect.Height() / (float)height;
			float ScaleIndex = (xScale <= yScale ? xScale : yScale);
			rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
			//��picture control������ͼ�����ź�Ĵ�С
			//				CWnd *pWnd;
			//				pWnd = GetDlgItem(IDC_PICTURE);
			//				pWnd->MoveWindow(CRect((int)rect.top, (int)rect.left, (int)width*ScaleIndex, (int)height*ScaleIndex));
			img->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
		}
		ReleaseDC(pDC);
	}
}
void CExperimentImgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		AutoSizePaint(mPictureControl, m_pImgSrc);
		AutoSizePaint(originPictureControl, m_pImgCpy);
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CExperimentImgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CExperimentImgDlg::Update(void* p)
{
	while (1)
	{
		Sleep(200);
		CExperimentImgDlg* dlg = (CExperimentImgDlg*)p;
		dlg->AutoSizePaint(dlg->mPictureControl, dlg->m_pImgSrc);
		dlg->AutoSizePaint(dlg->originPictureControl, dlg->m_pImgCpy);
	}
	return 0;
}

void CExperimentImgDlg::ThreadDraw(DrawPara *p)
{
	CRect rect;
	GetClientRect(&rect);
	CDC     memDC;             // ���ڻ����ͼ���ڴ滭��  
	CBitmap memBitmap;         // ���ڻ����ͼ���ڴ滭��
	memDC.CreateCompatibleDC(p->pDC);  // ������ԭ���ʼ��ݵĻ���
	memBitmap.CreateCompatibleBitmap(p->pDC, p->width, p->height);  // ������ԭλͼ���ݵ��ڴ滭��
	memDC.SelectObject(&memBitmap);      // ���������뻭���Ĺ���
	memDC.FillSolidRect(rect, p->pDC->GetBkColor());
	p->pDC->SetStretchBltMode(HALFTONE);
	// ��pImgSrc���������Ż����ڴ滭����
	p->pImgSrc->StretchBlt(memDC.m_hDC, 0, 0, p->width, p->height);

	// ���ѻ��õĻ������Ƶ������Ļ�������
	p->pDC->BitBlt(p->oriX, p->oriY, p->width, p->height, &memDC, 0, 0, SRCCOPY);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
}
void CExperimentImgDlg::OnCbnSelchangeComboFunction()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void CExperimentImgDlg::OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM);
	CString text("");
	m_nThreadNum = slider->GetPos();
	text.Format(_T("%d"), m_nThreadNum);
	GetDlgItem(IDC_STATIC_THREADNUM)->SetWindowText(text);
	*pResult = 0;
}

void CExperimentImgDlg::OnNMCustomdrawSliderAlpha(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_ALPHA);
	CString text("");
	
	text.Format(_T("%.2f "), (slider->GetPos()/100.0));
	GetDlgItem(IDC_STATIC_ALPHA)->SetWindowText(text);
	*pResult = 0;
}

void CExperimentImgDlg::OnBnClickedButtonOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files ��*.*��|*.*||");

	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
		//		mEditInfo.SetWindowTextW(strFilePath);	//���ı�������ʾͼ��·��

		if (m_pImgSrc != NULL)
		{
			m_pImgSrc->Destroy();
			delete m_pImgSrc;
		}
		m_pImgSrc = new CImage();
		m_pImgSrc->Load(strFilePath);

		if (m_pImgCpy != NULL)
		{
			m_pImgCpy->Destroy();
			delete m_pImgCpy;
		}
		m_pImgCpy = new CImage();//��Ҫnew
		//����һ��ԭͼ��
		ImageCopy(m_pImgSrc, m_pImgCpy);
		//refresh the application 
		this->Invalidate();
	}
}
void CExperimentImgDlg::OnBnClickedButtonProcess()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	int func = cmb_function->GetCurSel();
	//����һ��ԭͼ��������Դﵽ����ͼ�������Ч��
	//m_pImgCpy = new CImage();//��Ҫnew
	ImageCopy(m_pImgSrc, m_pImgCpy);
	switch (func)
	{
	case 0: //��������
		ColorBalance();
		break;
	case 1: //�Զ���ƽ��
		WhiteBalance();
		break;
	case 2:  //˳ʱ����ת
		ZoomImage();
		break;
	case 3: //����Ӧ��ֵ�˲�
		RotateImage();
		break;
	case 4: //��������
		MedianFilter();
		break;
	case 5: //��������
		AddNoise();
		break;
	case 6: //ͼ���ں�
		ImageFusion();
		break;
	case 7: //˫���˲�
		BilateralFilter();
		break;
	default:
		break;
	}
}


void CExperimentImgDlg::OnBnClickedButtonReset()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//���˵���һ�εĴ���
	ImageCopy(m_pImgCpy, m_pImgSrc);
	if (m_pImgCpy != NULL)
	{
		m_pImgCpy->Destroy();
		delete m_pImgCpy;
	}
	m_pImgCpy = new CImage();
	m_pImgCpy->Load(filePath);
	//�����Ϣ
	CString Str;
	Str.Format(_T("���õ���һ�δ���"));
	mOutputInfo.SetWindowTextW(Str);
	//refresh the application 
	this->Invalidate();
}

void CExperimentImgDlg::ImageCopy(CImage* pImgSrc, CImage* pImgDrt)
{
	int MaxColors = pImgSrc->GetMaxColorTableEntries();
	RGBQUAD* ColorTab;
	ColorTab = new RGBQUAD[MaxColors];

	CDC *pDCsrc, *pDCdrc;
	if (!pImgDrt->IsNull())
	{
		pImgDrt->Destroy();
	}
	pImgDrt->Create(pImgSrc->GetWidth(), pImgSrc->GetHeight(), pImgSrc->GetBPP(), 0);

	if (pImgSrc->IsIndexed())
	{
		pImgSrc->GetColorTable(0, MaxColors, ColorTab);
		pImgDrt->SetColorTable(0, MaxColors, ColorTab);
	}

	pDCsrc = CDC::FromHandle(pImgSrc->GetDC());
	pDCdrc = CDC::FromHandle(pImgDrt->GetDC());
	pDCdrc->BitBlt(0, 0, pImgSrc->GetWidth(), pImgSrc->GetHeight(), pDCsrc, 0, 0, SRCCOPY);
	pImgSrc->ReleaseDC();
	pImgDrt->ReleaseDC();
	delete ColorTab;

}
