#ifndef _WEBVIEW2_IMPLEMENT_H_
#define _WEBVIEW2_IMPLEMENT_H_

#ifndef OVERRIDE
#define OVERRIDE override
#endif // OVERRIDE

#include <string>
#include <atlcomcli.h>

#include "WebView2.h"
#include "wv2.h"

#include "WindowCloseRequested.h"

class cwv2 :
	public wv2,
	public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
	public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
	public ICoreWebView2ExecuteScriptCompletedHandler,
	public ICoreWebView2NavigationStartingEventHandler,
	public ICoreWebView2NavigationCompletedEventHandler,
	public ICoreWebView2DOMContentLoadedEventHandler,
	public ICoreWebView2HistoryChangedEventHandler,
	public ICoreWebView2PermissionRequestedEventHandler,
	public ICoreWebView2WebMessageReceivedEventHandler
{
public:
	friend class WindowCloseRequested;

	enum CreateStatus{
		none,		// 생성전
		created,	// 생성
		completed,	// 준비완료
		failed,		// 실패	
	};

	struct request {
		bool isHtmlContent = false;
		std::wstring uriOrHtmlContent;
		
		void clear() {
			isHtmlContent = false;
			uriOrHtmlContent.clear();
		}
	};

	cwv2(HWND parentWindow, createCompleted createCompletedHandler =nullptr,
		void* userData =nullptr);
	virtual ~cwv2();

	// ICoreWebView2 interfaces	///////////////////////////////////////////////
	STDMETHODIMP Invoke(HRESULT errorCode, ICoreWebView2Environment *env) OVERRIDE;
	STDMETHODIMP Invoke(HRESULT errorCode, ICoreWebView2Controller* controller) OVERRIDE;
	STDMETHODIMP Invoke(HRESULT errorCode, LPCWSTR resultObjectAsJson) OVERRIDE;
	STDMETHODIMP Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationStartingEventArgs *args) OVERRIDE;
	STDMETHODIMP Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args) OVERRIDE;
	STDMETHODIMP Invoke(ICoreWebView2 *sender, ICoreWebView2DOMContentLoadedEventArgs *args) OVERRIDE;
	STDMETHODIMP Invoke(ICoreWebView2 *sender, ICoreWebView2PermissionRequestedEventArgs *args) OVERRIDE;
	STDMETHODIMP Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) OVERRIDE;
	STDMETHODIMP Invoke(ICoreWebView2 *sender, IUnknown *args) OVERRIDE;
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv) OVERRIDE;
	ULONG STDMETHODCALLTYPE AddRef() OVERRIDE;
	ULONG STDMETHODCALLTYPE Release() OVERRIDE;
	// ICoreWebView2 interfaces	///////////////////////////////////////////////

	// wv2 interface	///////////////////////////////////////////////////////
	wv2settings* getSettings() OVERRIDE;
	bool setSettings(const wv2settings* settings) OVERRIDE;

	bool executeScript(LPCWSTR script, executeScriptCompleted handler) OVERRIDE;
	LPCWSTR executeScriptSync(LPCWSTR script) OVERRIDE;
	LPCWSTR getSource() OVERRIDE;
	bool goBack() OVERRIDE;
	bool goForward() OVERRIDE;
	bool navigate(const wchar_t* url) OVERRIDE;
	bool navigateToString(const wchar_t* html) OVERRIDE;
	bool reload() OVERRIDE;
	bool resize(int width, int height) OVERRIDE;

	bool setHistoryChangedHandler(historyChanged handler) OVERRIDE;
	bool setNavigationStartingHandler(navigationStarting handler) OVERRIDE;
	bool setNavigationCompletedHandler(navigationCompleted handler) OVERRIDE;
	bool setDomContentLoadedHandler(domContentLoaded handler) OVERRIDE;
	bool setWindowCloseRequestedHandler(windowCloseRequested handler) OVERRIDE;
	bool setWebMessageReceivedHandler(webMessageReceived handler) OVERRIDE;
	bool stop() OVERRIDE;
	double zoomFactor(const double* newZoomFactor) OVERRIDE;
	
	void destroy() OVERRIDE;
	void detach() OVERRIDE;

	void* getUserData() OVERRIDE;
	bool setUserData(void* userData) OVERRIDE;

	bool setVirtualHostNameToFolderMapping(LPCWSTR hostName, LPCWSTR folderPath, 
		wv2HostResourceAccessKind accessKind) OVERRIDE;

	void freeMemory(void* p) OVERRIDE;

	bool postWebMessageAsJson(LPCWSTR messageAsJson) OVERRIDE;
	bool postWebMessageAsString(LPCWSTR messageAsString) OVERRIDE;

	// wv2 interface	///////////////////////////////////////////////////////

	// 웹뷰 초기화가 완료 여부 (초기화가 성공되었음을 의미하지 않음)
	bool isDone() const;
	CreateStatus createStatus() const;

	inline HRESULT lastError() const { return lastError_; }
private:
	// 모든 리소스 정리
	void clearAll(bool detachController = false);
	// 생성실패로 상태 설정
	HRESULT setStatusCreateFail(HRESULT errorCode);

	void fireWindowCloseRequested();
private:
	HWND parentWindow_ = nullptr;
	ULONG refCount_ = 0;
	CComPtr<ICoreWebView2_3> webview_;
	CComPtr<ICoreWebView2Controller3> controller_;
	
	request lastRequest_;	// 처리되지 않은 마지막 요청정보
	CreateStatus createStatus_;
	void* userData_;
	LPWSTR executeScriptSyncResult_;
	std::wstring virtualHostName_;
	wv2settings settings_;

	executeScriptCompleted executeScriptCompletedHandler_;
	createCompleted createCompletedHandler_;
	historyChanged historyChangedHandler_;
	EventRegistrationToken historyChangedToken_;

	navigationCompleted navigationCompletedHandler_;
	EventRegistrationToken navigationCompletedToken_;

	navigationStarting navigationStartingHandler_;
	EventRegistrationToken navigationStartingToken_;

	domContentLoaded domContentLoadedHandler_;
	EventRegistrationToken domContentLoadedToken_;

	WindowCloseRequested windowCloseRequestedHandler_;

	EventRegistrationToken permissionRequestedToken_;

	webMessageReceived webMessageReceivedHandler_;
	EventRegistrationToken webMessageReceivedToken_;
	
	HRESULT lastError_;
	bool coInitilized_;
};

#endif // _WEBVIEW2_IMPLEMENT_H_