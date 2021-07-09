#include "cwv2.h"

#include <winnt.h>
#include <string>
#include <ciso646>
#include <codecvt>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _M_IX86	// x86
#pragma comment(lib, "webview2/x86/WebView2Loader.dll.lib")	// DLL����
//#pragma comment(lib, "webview2/x86/WebView2LoaderStatic.lib")	// Static����
#elif _M_X64
#pragma comment(lib, "webview2/x64/WebView2Loader.dll.lib")	// DLL���� 
//#pragma comment(lib, "webview2/x64/WebView2LoaderStatic.lib")	// Static����
#elif _M_ARM64
#pragma comment(lib, "webview2/arm64/WebView2Loader.dll.lib")	// DLL����
//#pragma comment(lib, "webview2/arm64/WebView2LoaderStatic.lib")	// Static����
#endif

using namespace std;
static void wait();

///////////////////////////////////////////////////////////////////////////////
void wait() {
	MSG msg;
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

///////////////////////////////////////////////////////////////////////////////
cwv2::cwv2(HWND parentWindow, 
	createCompleted createCompletedHandler /*=nullptr*/,
	void* userData /*=nullptr*/) 
	:parentWindow_(parentWindow), createCompletedHandler_(createCompletedHandler), 
	userData_(userData) {

	coInitilized_ = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
	createStatus_ = created;
};

cwv2::~cwv2() {
	clearAll();
	if (coInitilized_) CoUninitialize();
}

void cwv2::clearAll() {
	userData_ = nullptr;

	if (executeScriptSyncResult_) {
		free(executeScriptSyncResult_);
		executeScriptSyncResult_ = nullptr;
	}

	if (webview_) {
		windowCloseRequestedHandler_.remove(webview_);
		webview_->remove_PermissionRequested(permissionRequestedToken_);
		webview_->remove_HistoryChanged(historyChangedToken_);
		webview_->remove_DOMContentLoaded(domContentLoadedToken_);
		webview_->remove_NavigationStarting(navigationStartingToken_);
		webview_->remove_NavigationCompleted(navigationCompletedToken_);	
		if (virtualHostName_.length() > 0) {
			webview_->ClearVirtualHostNameToFolderMapping(virtualHostName_.c_str());
		}

		webview_.Release();
	}

	if (controller_) {
		controller_->Close();
		controller_.Release();
	}
}

wv2settings* cwv2::getSettings() {
	if (!webview_) return nullptr;

	CComPtr<ICoreWebView2Settings> s;
	if (FAILED(webview_->get_Settings(&s))) {
		return nullptr;
	}

	BOOL v;
	s->get_AreDefaultContextMenusEnabled(&v);
	settings_.areDefaultContextMenusEnabled = v == TRUE;
	s->get_AreDefaultScriptDialogsEnabled(&v);
	settings_.areDefaultScriptDialogsEnabled = v == TRUE;
	s->get_AreDevToolsEnabled(&v);
	settings_.areDevToolsEnabled= v == TRUE;
	s->get_AreHostObjectsAllowed(&v);
	settings_.areHostObjectsAllowed = v == TRUE;
	s->get_IsBuiltInErrorPageEnabled(&v);
	settings_.isBuiltInErrorPageEnabled = v == TRUE;
	s->get_IsScriptEnabled(&v);
	settings_.isScriptEnabled = v == TRUE;
	s->get_IsStatusBarEnabled(&v);
	settings_.isStatusBarEnabled = v == TRUE;
	s->get_IsWebMessageEnabled(&v);
	settings_.isWebMessageEnabled = v == TRUE;
	s->get_IsZoomControlEnabled(&v);
	settings_.isZoomControlEnabled = v == TRUE;

	return &settings_;
}

bool cwv2::setSettings(const wv2settings* val) {
	if (!webview_ or !val) return false;
	
	CComPtr<ICoreWebView2Settings> s;
	if (FAILED(webview_->get_Settings(&s))) {
		return false;
	}

	settings_ = *val;
	auto& r = settings_;
	if (FAILED(s->put_AreDefaultContextMenusEnabled(
		r.areDefaultContextMenusEnabled))) {
		return false;
	}

	if (FAILED(s->put_AreDefaultScriptDialogsEnabled(
		r.areDefaultScriptDialogsEnabled))) {
		return false;
	}

	if (FAILED(s->put_AreDevToolsEnabled(r.areDevToolsEnabled))) {
		return false;
	}

	if (FAILED(s->put_AreHostObjectsAllowed(r.areHostObjectsAllowed))) {
		return false;
	}

	if (FAILED(s->put_IsBuiltInErrorPageEnabled(r.isBuiltInErrorPageEnabled))) {
		return false;
	}

	if (FAILED(s->put_IsScriptEnabled(r.isScriptEnabled))) {
		return false;
	}

	if (FAILED(s->put_IsStatusBarEnabled(r.isStatusBarEnabled))) {
		return false;
	}

	if (FAILED(s->put_IsWebMessageEnabled(r.isWebMessageEnabled))) {
		return false;
	}

	if (FAILED(s->put_IsZoomControlEnabled(r.isZoomControlEnabled))) {
		return false;
	}

	return true;
}

void* cwv2::getUserData() {
	return userData_;
}

bool cwv2::setUserData(void* userData) {
	userData_ = userData;
	return true;
}

HRESULT cwv2::setStatusCreateFail(const HRESULT errorCode) {
	// ���� ���з� ����
	createStatus_ = failed;

	// createCompleted�ڵ鷯�� �������� ��� ����
	if (createCompletedHandler_) {
		createCompletedHandler_(nullptr, errorCode);
		// ��ü ����
		Release();
	}
	return errorCode;
}

STDMETHODIMP cwv2::Invoke(HRESULT errorCode, ICoreWebView2Environment *env) {
	if (FAILED(errorCode)) {
		return setStatusCreateFail(errorCode);
	}
		
	HRESULT hr = env->CreateCoreWebView2Controller(parentWindow_, this);
	if (FAILED(hr)) {
		createStatus_ = failed;
	}
	return hr;
}

STDMETHODIMP cwv2::Invoke(ICoreWebView2 *sender, ICoreWebView2PermissionRequestedEventArgs *args) {
	if (sender != webview_) return E_UNEXPECTED;
	args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
	return S_OK;
}

STDMETHODIMP cwv2::Invoke(HRESULT errorCode, ICoreWebView2Controller* controller) {
	if (FAILED(errorCode)) {
		return setStatusCreateFail(errorCode);
	}

	// WebView2 ȹ��
	CComPtr<ICoreWebView2> webview2;
	HRESULT hr = controller->get_CoreWebView2(&webview2);
	if (FAILED(hr)) {
		return setStatusCreateFail(hr);
	}

	// WebView2 3��° ���� ȹ��
	hr = webview2->QueryInterface(IID_ICoreWebView2_3, (void**)&webview_);
	if (FAILED(hr)) {
		return setStatusCreateFail(hr);
	}
		
	// �θ�ȭ�� ����� �°� ����
	RECT bounds = { 0, };
	GetClientRect(parentWindow_, &bounds);
	controller->put_Bounds(bounds);

	hr = controller->QueryInterface(IID_ICoreWebView2Controller3, (void**)&controller_);
	if (FAILED(hr)) {
		return setStatusCreateFail(hr);
	}
			
	if (webview_ != nullptr) {
		createStatus_ = completed;
		windowCloseRequestedHandler_.add(webview_);
		webview_->add_PermissionRequested(this, &permissionRequestedToken_);
		webview_->add_HistoryChanged(this, &historyChangedToken_);
		webview_->add_DOMContentLoaded(this, &domContentLoadedToken_);
		webview_->add_NavigationStarting(this, &navigationStartingToken_);
		webview_->add_NavigationCompleted(this, &navigationCompletedToken_);
	}
	else {
		createStatus_ = failed;
	}

	if (lastRequest_.uriOrHtmlContent.size() > 0) {
		LPCWSTR uriOrHtml = lastRequest_.uriOrHtmlContent.c_str();
		if (lastRequest_.isHtmlContent) {
			navigateToString(uriOrHtml);
		}
		else {
			navigate(uriOrHtml);
		}
		lastRequest_.clear();
	}

	controller_->put_IsVisible(TRUE);

	if (createCompletedHandler_) {
		createCompletedHandler_(this, errorCode);
	}

	return S_OK;
}

STDMETHODIMP cwv2::Invoke(HRESULT errorCode, LPCWSTR resultObjectAsJson) {
	if (executeScriptCompletedHandler_) {
		executeScriptCompletedHandler_((wv2_t)this, resultObjectAsJson);
	}
	else {
		// sync ȣ���� ���
		executeScriptSyncResult_ = _wcsdup(resultObjectAsJson);
	}

	return errorCode;
}

STDMETHODIMP cwv2::Invoke(ICoreWebView2 *sender, IUnknown *args) {
	if (sender != webview_) return E_UNEXPECTED;
		
	if (historyChangedHandler_) {
		BOOL canGoBack = FALSE;
		BOOL canGoForward = FALSE;
		sender->get_CanGoBack(&canGoBack);
		sender->get_CanGoForward(&canGoForward);
		historyChangedHandler_(this, canGoBack == TRUE, canGoForward == TRUE);
	}

	return S_OK;
}

STDMETHODIMP cwv2::Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationStartingEventArgs *args) {
	if (sender != webview_) return E_UNEXPECTED;

	if (navigationStartingHandler_) {
		LPWSTR uri = nullptr;
		args->get_Uri(&uri);
		
		if (!navigationStartingHandler_(this, uri)) {
			args->put_Cancel(TRUE);
		}

		CoTaskMemFree(uri);
	}

	return S_OK;
}

STDMETHODIMP cwv2::Invoke(ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args) {
	if (sender != webview_) return E_UNEXPECTED;
	
	if (navigationCompletedHandler_) {
		navigationCompletedHandler_(this);
	}
	
	return S_OK;
}

STDMETHODIMP cwv2::Invoke(ICoreWebView2 *sender, ICoreWebView2DOMContentLoadedEventArgs *args) {
	if (sender != webview_) return E_UNEXPECTED;

	if (domContentLoadedHandler_) {
		domContentLoadedHandler_(this);
	}
	return S_OK;
}

STDMETHODIMP cwv2::QueryInterface(REFIID riid, LPVOID* ppv) {
	// Always set out parameter to NULL, validating it first.
	if (!ppv) {
		return E_INVALIDARG;
	}
	*ppv = NULL;
	if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler ||
		riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)
	{
		// Increment the reference count and return the pointer.
		*ppv = (LPVOID)this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE cwv2::AddRef() {
	InterlockedIncrement(&refCount_);
	return refCount_;
}

ULONG STDMETHODCALLTYPE cwv2::Release() {
	// Decrement the object's internal counter.
	ULONG ulRefCount = InterlockedDecrement(&refCount_);
	if (0 == refCount_) {
		delete this;
	}
	return ulRefCount;
}

void cwv2::destroy() {
	clearAll();
	Release();
}

bool cwv2::executeScript(LPCWSTR script, executeScriptCompleted handler) {
	if (!webview_) return false;
	if (handler) {
		executeScriptCompletedHandler_ = handler;
	}
	else {
		// empty handler ����
		executeScriptCompletedHandler_ = [](wv2_t sender, LPCWSTR resultObjectAsJson){};
	}
	
	return SUCCEEDED(webview_->ExecuteScript(script, this));
}

LPCWSTR cwv2::executeScriptSync(LPCWSTR script) {
	if (!webview_) return nullptr;
	executeScriptCompletedHandler_ = nullptr;
	if (executeScriptSyncResult_) {
		free(executeScriptSyncResult_);
		executeScriptSyncResult_ = nullptr;
	}

	HRESULT hr = webview_->ExecuteScript(script, this);
	if (FAILED(hr)) {
		return nullptr;
	}

	while (executeScriptSyncResult_ != nullptr) {
		wait();
	}

	return executeScriptSyncResult_;
}

LPCWSTR cwv2::getSource() {
	LPCWSTR source = nullptr;
	if (!webview_) return source;
	
	LPWSTR uri = nullptr;
	if (SUCCEEDED(webview_->get_Source(&uri))) {
		source = _wcsdup(uri);
		CoTaskMemFree(uri);
	}
	return source;
}

bool cwv2::goBack() {
	if (!webview_) return false;
	return SUCCEEDED(webview_->GoBack());
}

bool cwv2::goForward() {
	if (!webview_) return false;
	return SUCCEEDED(webview_->GoForward());
}

bool cwv2::navigate(const wchar_t* url) {
	if (!webview_) {
		// ���� ȹ�� ���� �ܰ��� ����� uri�� �����ϰ� ȹ���Ŀ� navigate �Ѵ�.
		lastRequest_.isHtmlContent = false;
		lastRequest_.uriOrHtmlContent = url;
		return true;
	}
	return SUCCEEDED(webview_->Navigate(url));
}

bool cwv2::navigateToString(const wchar_t* html) {
	if (!webview_) {
		// ���� ȹ�� ���� �ܰ��� ����� ������ �� ȹ�� �� navigate �Ѵ�.
		lastRequest_.isHtmlContent = true;
		lastRequest_.uriOrHtmlContent = html ? html : L"";
		return true;
	}
	return SUCCEEDED(webview_->NavigateToString(html));
}

bool cwv2::reload() {
	if (!webview_) return false;
	return SUCCEEDED(webview_->Reload());
}

bool cwv2::resize(int width, int height) {
	if (!controller_) return false;

	RECT bounds = { 0, };
	bounds.right = bounds.left + width;
	bounds.bottom = bounds.top + height;
	return SUCCEEDED(controller_->put_Bounds(bounds));
}

bool cwv2::setHistoryChangedHandler(historyChanged handler) {
	if (!webview_) return false;
	historyChangedHandler_ = handler;
	return true;
}

bool cwv2::setNavigationStartingHandler(navigationStarting handler) {
	if (!webview_) return false;
	navigationStartingHandler_ = handler;
	return true;
}

bool cwv2::setNavigationCompletedHandler(navigationCompleted handler) {
	if (!webview_) return false;
	navigationCompletedHandler_ = handler;
	return true;
}

bool cwv2::setDomContentLoadedHandler(domContentLoaded handler) {
	if (!webview_) return false;
	domContentLoadedHandler_ = handler;
	return true;
}

bool cwv2::setWindowCloseRequestedHandler(windowCloseRequested handler) {
	if (!webview_) return false;
	windowCloseRequestedHandler_.bind(handler, userData_);
	return true;
}

bool cwv2::stop() {
	// webview�� ���� ��쿡�� stop�� �ǹ̰� ���� ������ �������� �����ϰ�, true����.
	if (!webview_) return true;

	return SUCCEEDED(webview_->Stop());
}
	
double cwv2::zoomFactor(const double* newZoomFactor) {
	double curZoomFactor = -1.0;
	if (!controller_) return curZoomFactor;


	if (newZoomFactor) {
		if (SUCCEEDED(controller_->put_ZoomFactor(*newZoomFactor))) {
			curZoomFactor = *newZoomFactor;
		}
	}
	else {
		controller_->get_ZoomFactor(&curZoomFactor);
	}
	return curZoomFactor;
}

// ���� �ʱ�ȭ�� �Ϸ� ���� (�ʱ�ȭ�� �����Ǿ����� �ǹ����� ����)
bool cwv2::isDone() const {
	return createStatus_ == failed || createStatus_ == completed;
}

cwv2::CreateStatus cwv2::crateStatus() const { 
	return createStatus_; 
}

bool cwv2::setVirtualHostNameToFolderMapping(LPCWSTR hostName,
	LPCWSTR folderPath, wv2HostResourceAccessKind accessKind) {
	if (!webview_) return false;
	if (!hostName or !folderPath) return false;
	
	HRESULT hr = webview_->SetVirtualHostNameToFolderMapping(hostName, 
		folderPath, (COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND)accessKind);

	if (SUCCEEDED(hr)) {
		virtualHostName_ = hostName;
		return true;
	}
	else {
		virtualHostName_.clear();
		return false;
	}
}

void cwv2::freeMemory(void* p) {
	if (!p) return;
	free(p);
}