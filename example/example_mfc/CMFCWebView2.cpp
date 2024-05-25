#include "pch.h"
#include "CMFCWebView2.h"
#include <iostream>
#include <fstream>

#include "wv2.h"
#pragma comment(lib,"wv2")

static CStringW gBrowserFolder;

static void _webMessageReceived(wv2_t sender, LPCWSTR message);
static void _navigationCompleted(wv2_t sender);
///////////////////////////////////////////////////////////////////////////////

// CMFCWebView2
IMPLEMENT_DYNAMIC(CMFCWebView2, CWnd)

CMFCWebView2::CMFCWebView2() : webview2_(nullptr), webMsgListener(nullptr)
{
}

CMFCWebView2::~CMFCWebView2()
{
	if (webview2_) {
		webview2_->destroy();
		webview2_ = nullptr;
	}
}


BEGIN_MESSAGE_MAP(CMFCWebView2, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

bool CMFCWebView2::Initialize(LPCTSTR browserExecutableFolder) {

	if (browserExecutableFolder) {
#if _UNICODE
		gBrowserFolder = browserExecutableFolder;
#else
		gBrowserFolder = CStringW(browserExecutableFolder);
#endif // _UNICODE
	}

	LPWSTR verStr = wv2getAvailableBrowserVersionString(gBrowserFolder);
	if (verStr) {
		wv2freeMemory((void*)verStr);
		return true;
	}
	return false;
}

void CMFCWebView2::PreSubclassWindow() {
	CWnd::PreSubclassWindow();

	wv2envOpts_t options = nullptr;
#ifdef DEBUG
	options = wv2envOptsCreate();
	wv2envOptsSetString(options, "AdditionalBrowserArguments", L"--auto-open-devtools-for-tabs");
#endif // DEBUG

	webview2_ = (wv2*)wv2createSync2(nullptr, nullptr, options, GetSafeHwnd());

	if (webview2_) {
		webview2_->setUserData(this);
		webview2_->setWebMessageReceivedHandler(_webMessageReceived);
		webview2_->setNavigationCompletedHandler(_navigationCompleted);

		wv2settings* pSettings = webview2_->getSettings();
		if (pSettings) {
			pSettings->areDefaultContextMenusEnabled = false; // 禁用网页内的右键菜单
			pSettings->isZoomControlEnabled = false; // 禁止页面缩放
			webview2_->setSettings(pSettings);
		}
	}
}

void CMFCWebView2::OnWebMessageReceived(LPCTSTR message) {
	if (webMsgListener) {
		webMsgListener->OnWebMessageReceived(message);
	}
}

void CMFCWebView2::OnNavigationCompleted() {
	if (webMsgListener) {
		webMsgListener->OnNavigationCompleted();
	}
}

bool CMFCWebView2::Navigate(LPCTSTR uri) {
	if (!uri || !webview2_) return false;
		
#if _UNICODE
	LPCWSTR wUri = uri;
#else
	LPCWSTR wUri = CStringW(uri);
#endif

	return webview2_->navigate(wUri);
}

int CMFCWebView2::OnCreate(LPCREATESTRUCT lpcs) {
	if (CWnd::OnCreate(lpcs) == -1) {
		return -1;
	}
	return 0;
}

void CMFCWebView2::OnSize(UINT nType, int cx, int cy) {
	CWnd::OnSize(nType, cx, cy);
	if (GetSafeHwnd()) {
		if (webview2_)	webview2_->resize(cx, cy);
	}
}

bool CMFCWebView2::GoBack() {
	return webview2_ ? webview2_->goBack() : false;
}

bool CMFCWebView2::GoForward() {
	return webview2_ ? webview2_->goForward() : false;
}

bool CMFCWebView2::Reload() {
	return webview2_ ? webview2_->reload() : false;
}

bool CMFCWebView2::PostWebMessageAsString(LPCTSTR message) {
	if (!message) return false;
	if (webview2_) {
		CStringW wMsg(message);
		return webview2_->postWebMessageAsString(wMsg);
	}
	return false;
}

// 在网页中：监听从C++发送的消息
/*
window.chrome.webview.addEventListener('message', function(event) {
	// 解析接收到的消息
	// var message = JSON.parse(event.data);
	// event.data is a JSON object, use it directly like event.data['id']

	// 处理接收到的消息
	// ...
});
*/
bool CMFCWebView2::PostWebMessageAsJson(LPCTSTR message) {
	if (!message) return false;
	if (webview2_) {
		CStringW sMsg(message);
		return webview2_->postWebMessageAsJson(sMsg);
	}
	return false;
}

// 在网页中：往C++发送消息
// window.chrome.webview.postMessage(JSON.stringify(params))
void _webMessageReceived(wv2_t sender, LPCWSTR message) {
	if(CMFCWebView2* p = (CMFCWebView2*)wv2getUserData(sender)) {
		CString s(message);
		p->OnWebMessageReceived(s);
	}
}

void _navigationCompleted(wv2_t sender) {
	if (CMFCWebView2* p = (CMFCWebView2*)wv2getUserData(sender)) {
		p->OnNavigationCompleted();
	}
}

