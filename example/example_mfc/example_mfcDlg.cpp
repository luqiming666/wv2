
// example_mfcDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "example_mfc.h"
#include "example_mfcDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <string>
using namespace std;

#include "json.hpp"
using json = nlohmann::json;

#define WM_NOTIFICATION_TO_WEBPAGE   (WM_USER+100)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CexamplemfcDlg 대화 상자


CexamplemfcDlg::CexamplemfcDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EXAMPLE_MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CexamplemfcDlg::~CexamplemfcDlg() {
}

void CexamplemfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEBVIEW2, m_webview2);
	DDX_Control(pDX, IDC_EDIT_URL, m_editUrl);
}

BEGIN_MESSAGE_MAP(CexamplemfcDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CexamplemfcDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CexamplemfcDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CexamplemfcDlg::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD, &CexamplemfcDlg::OnBnClickedButtonForward)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD, &CexamplemfcDlg::OnBnClickedButtonReload)
	ON_MESSAGE(WM_NOTIFICATION_TO_WEBPAGE, OnNotifyWebPage)
END_MESSAGE_MAP()


// CexamplemfcDlg 메시지 처리기

BOOL CexamplemfcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	//CRect rect;
	//GetClientRect(rect);
	//MoveWindow(rect);

	m_webview2.SetWebMsgListener(this);

	//m_webview2.Navigate(_T("https://baidu.com"));
	
	//CString destUrl =  _T("file:///") + GetHtmlPath(_T("index.html"));
	CString destUrl = _T("file:///") + GetHtmlPath(_T("testpage.html"));
	m_webview2.Navigate(destUrl);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CexamplemfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

void CexamplemfcDlg::OnSize(UINT nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	if (m_webview2.GetSafeHwnd()) {
		const int controlHeight = 50;
		m_webview2.MoveWindow(0, controlHeight, cx, cy-controlHeight);
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CexamplemfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CexamplemfcDlg::OnBnClickedOk()
{
	CString url;
	m_editUrl.GetWindowText(url);
	m_webview2.Navigate(url);
	//CDialog::OnOK();
}


void CexamplemfcDlg::OnBnClickedCancel()
{
	CDialog::OnCancel();
}


void CexamplemfcDlg::OnBnClickedButtonBack()
{
	m_webview2.GoBack();
}


void CexamplemfcDlg::OnBnClickedButtonForward()
{
	m_webview2.GoForward();
}


void CexamplemfcDlg::OnBnClickedButtonReload()
{
	m_webview2.Reload();
}

// 当网页加载完成时的通知
void CexamplemfcDlg::OnNavigationCompleted()
{
}

// 接收来自网页的数据
void CexamplemfcDlg::OnWebMessageReceived(LPCTSTR message)
{
	try {
		json msg = json::parse(wstring(message));
#ifdef _DEBUG
		string whatsthis = msg.dump(4);
		//std::cout << whatsthis << std::endl;
#endif

		//auto appid = msg["appid"].get<int>();
		//auto cmdType = msg["cmd"].get<string>();

		////////////////////////////////////////////////
		// TEST: Reply to the web page
		NotifyWebUI("Hello World from App :)");
		// Alternatively, post a windows message
		//PostMessage(WM_NOTIFICATION_TO_WEBPAGE);
		////////////////////////////////////////////////
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		MessageBox(_T("Failed to parse Web message."), _T("QM"), MB_OK | MB_ICONERROR);
	}
}

void CexamplemfcDlg::NotifyWebUI(const char* pMsg)
{
	json jsResponse;
	jsResponse["type"] = 666;
	jsResponse["content"] = pMsg;
	CString strMsg(jsResponse.dump(4).c_str());
	m_webview2.PostWebMessageAsJson(strMsg); // 通知网页
}

LRESULT CexamplemfcDlg::OnNotifyWebPage(WPARAM wParam, LPARAM lParam)
{
	NotifyWebUI("Hello World from App. Oh yeah~ in Main UI thread");
	return 0;
}

CString CexamplemfcDlg::GetHtmlPath(TCHAR* filename)
{
	TCHAR szPath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szPath, MAX_PATH);

	CString strProcessPath(szPath);

	// 删除文件名部分，得到相同目录的路径
	PathRemoveFileSpec(strProcessPath.GetBuffer());
	strProcessPath.ReleaseBuffer();

	// 拼接 index.html 文件的完整路径
	CString strIndexHtmlPath;
	PathCombine(strIndexHtmlPath.GetBuffer(MAX_PATH), strProcessPath, filename);
	strIndexHtmlPath.ReleaseBuffer();

	return strIndexHtmlPath;
}
