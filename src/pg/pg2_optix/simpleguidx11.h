#pragma once
#include "simpleguidx11.h"
#include "structs.h"
#include "imgui_internal.h"

class SimpleGuiDX11
{
public:	
	SimpleGuiDX11( const int width, const int height );	
	~SimpleGuiDX11();		
	
	int MainLoop();	

protected:
	int Init();
	int Cleanup();	

	void CreateRenderTarget();
	void CleanupRenderTarget();
	HRESULT CreateDeviceD3D( HWND hWnd );
	void CleanupDeviceD3D();

	HRESULT CreateTexture();
	LRESULT WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK s_WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );		

	virtual int Ui();
	virtual Color3f get_pixel( const int x, const int y, const float t = 0.0f );
	virtual int get_image(BYTE * buffer);


	void Producer();

	int width() const;
	int height() const;
	ImRect imageRect;

	bool vsync_{ true };
	float gamma_{ 2.4f };
	float mouseSensitivity = { 0.5 };
	int speed = { 5 };

	ImVec2 mousePoss;

private:	
	WNDCLASSEX wc_;
	HWND hwnd_;

	ID3D11Device * g_pd3dDevice{ nullptr };
	ID3D11DeviceContext * g_pd3dDeviceContext{ nullptr };
	IDXGISwapChain * g_pSwapChain{ nullptr };
	ID3D11RenderTargetView * g_mainRenderTargetView{ nullptr };

	ID3D11Texture2D * tex_id_{ nullptr };
	ID3D11ShaderResourceView * tex_view_{ nullptr };
	int width_{ 800 };
	int height_{ 600 };
	BYTE * tex_data_{ nullptr }; // DXGI_FORMAT_R8G8B8A8_UNORM
	std::mutex tex_data_lock_;		
		
	// https://stackoverflow.com/questions/44685403/do-i-need-stdatomicbool-or-is-pod-bool-good-enough	
	std::atomic<bool> finish_request_{ false };	
	std::atomic<bool> repaint_request_{ false };
};
