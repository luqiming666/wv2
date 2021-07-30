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
		none,		// ������
		created,	// ����
		completed,	// �غ�Ϸ�
		failed,		// ����	
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

	// ���� �ʱ�ȭ�� �Ϸ� ���� (�ʱ�ȭ�� �����Ǿ����� �ǹ����� ����)
	bool isDone() const;
	CreateStatus crateStatus() const;

	inline HRESULT lastError() const { return lastError_; }
private:
	// ��� ���ҽ� ����
	void clearAll(bool detachController = false);
	// �������з� ���� ����
	HRESULT setStatusCreateFail(HRESULT errorCode);

	void fireWindowCloseRequested();
private:
	HWND parentWindow_ = nullptr;
	ULONG refCount_ = 0;
	CComPtr<ICoreWebView2_3> webview_;
	CComPtr<ICoreWebView2Controller3> controller_;
	
	request lastRequest_;	// ó������ ���� ������ ��û����
	CreateStatus createStatus_ = none;
	void* userData_ = nullptr;
	LPWSTR executeScriptSyncResult_ = nullptr;
	std::wstring virtualHostName_;
	wv2settings settings_ = { true, };

	executeScriptCompleted executeScriptCompletedHandler_ = nullptr;
	createCompleted createCompletedHandler_ = nullptr;
	historyChanged historyChangedHandler_ = nullptr;
	EventRegistrationToken historyChangedToken_ = { 0, };

	navigationCompleted navigationCompletedHandler_ = nullptr;
	EventRegistrationToken navigationCompletedToken_ = { 0, };

	navigationStarting navigationStartingHandler_ = nullptr;
	EventRegistrationToken navigationStartingToken_ = { 0, };

	domContentLoaded domContentLoadedHandler_ = nullptr;
	EventRegistrationToken domContentLoadedToken_ = { 0, };

	WindowCloseRequested windowCloseRequestedHandler_;

	EventRegistrationToken permissionRequestedToken_ = { 0, };

	webMessageReceived webMessageReceivedHandler_ = nullptr;
	EventRegistrationToken webMessageReceivedToken_ = { 0, };
	
	HRESULT lastError_ = S_OK;
	bool coInitilized_ = false;
};

#endif // _WEBVIEW2_IMPLEMENT_H_