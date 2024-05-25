#pragma once

#include <afxwin.h>
#include <string>

class IWebMsgListener {
public:
	virtual void OnWebMessageReceived(LPCTSTR message) = 0;
	virtual void OnNavigationCompleted() = 0;
};

struct wv2;

class CMFCWebView2 : public CWnd
{
	DECLARE_DYNAMIC(CMFCWebView2)

public:
	CMFCWebView2();
	virtual ~CMFCWebView2();

	static bool Initialize(LPCTSTR browserExecutableFolder);

	virtual bool Navigate(LPCTSTR uri);
	virtual bool GoBack();
	virtual bool GoForward();
	virtual bool Reload();
	virtual void PreSubclassWindow();

	virtual void OnWebMessageReceived(LPCTSTR message);
	virtual void OnNavigationCompleted();

	virtual bool PostWebMessageAsString(LPCTSTR message);
	virtual bool PostWebMessageAsJson(LPCTSTR message);

	void SetWebMsgListener(IWebMsgListener* pListener) { webMsgListener = pListener; }

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSize(UINT nType, int cx, int cy);	

private:
	wv2*	webview2_;
	IWebMsgListener*	webMsgListener;
};


