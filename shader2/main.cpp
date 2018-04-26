// ピクセルシェーダプログラムの基礎テストプログラム
// このプログラムに関する詳しい情報は
// ○×つくろ～どっとコム「ゲームつくろ～プログラマブルシェーダ編その４、その５」
// をご覧下さい。

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>

TCHAR gName[100] = _T("ピクセルシェーダプログラムの基礎テストプログラム");


/// 頂点定義 ///
struct CUSTOMVTX
{
	float x, y, z;    // 頂点位置
	DWORD color;      // 頂点カラー
	float u, v;     // テクスチャ座標
};

#define CUSTOMFVF  D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1


/// ウィンドウプロシージャ ///
LRESULT CALLBACK WndProc(HWND hWnd, UINT mes, WPARAM wParam, LPARAM lParam) {
	if (mes == WM_DESTROY) { PostQuitMessage(0); return 0; }
	return DefWindowProc(hWnd, mes, wParam, lParam);
}


/// エントリ ///
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// アプリケーションの初期化（ウィンドウ作成）
	MSG msg; HWND hWnd;
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, NULL,
		(HBRUSH)(COLOR_WINDOW + 1), NULL, (LPCSTR)gName, NULL };
	if (!RegisterClassEx(&wcex)) return 0;
	if (!(hWnd = CreateWindow(gName, gName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL)))
		return 0;

	// Direct3Dの初期化
	LPDIRECT3D9 g_pD3D;				// Idirect3Device9インターフェース
	LPDIRECT3DDEVICE9 g_pD3DDev;	// デバイスのIdirect3Device9インターフェース
	if (!(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return 0;	// 取得失敗　return false;
	D3DPRESENT_PARAMETERS d3dpp = { 0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,
		D3DSWAPEFFECT_DISCARD,NULL,TRUE,0,D3DFMT_UNKNOWN,0,0 };

	// IDirect3d9::CreateDeviceメソッド実行
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
		{
			g_pD3D->Release(); return 0;
		}

	// 頂点の設定（小さな長方形）
	CUSTOMVTX v[] =
	{
		{ -1.0f, +1.5f, 0.0f, 0xffffffff, 1.0f, 0.0f },
		{ -1.0f, -1.5f, 0.0f, 0xffffffff, 1.0f, 1.0f },
		{ +1.0f, +1.5f, 0.0f, 0xffffffff, 0.0f, 0.0f },
		{ +1.0f, -1.5f, 0.0f, 0xffffffff, 0.0f, 1.0f }
	};

	// 頂点バッファ作成と頂点情報の書き込み
	IDirect3DVertexBuffer9* pVertex;
	void *pData;
	if (FAILED(g_pD3DDev->CreateVertexBuffer(sizeof(CUSTOMVTX) * 4, D3DUSAGE_WRITEONLY, CUSTOMFVF,
		D3DPOOL_MANAGED, &pVertex, NULL))) {
		g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}
	if (FAILED(pVertex->Lock(0, sizeof(CUSTOMVTX) * 4, (void**)&pData, 0))) {
		g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}
	memcpy(pData, v, sizeof(CUSTOMVTX) * 4);
	pVertex->Unlock();


	// ピクセルシェーダ命令配列定義
	// 彩度調節シェーダ
	// c1 : 彩度調節係数 (c, c, c, 0) (グレー)0.0f < c < 1.0f(原色)
	const char PxShader[] =
		"ps_1_1                                        \n"
		"def   c0, 0.2989f, 0.5866f, 0.1145f, 0.0f     \n"     // 彩度算出係数
		"tex   t0                                      \n"     // テクスチャ0番使用
		"dp3   r0, t0, c0                              \n"     // 彩度Y算出(r0.aに格納される)
		"mov   r1, r0.a                                \n"     // r1の各成分をYで埋める
		"lrp   r0, c1, t0, r1                          \n";    // 線形補間( t0 + c1*(Y-t0) )

															   // シェーダ命令のコンパイル
	ID3DXBuffer *pShader;   // シェーダ命令格納バッファ
	ID3DXBuffer *pError;     // コンパイルエラー情報格納バッファ
	if (FAILED(D3DXAssembleShader(PxShader, sizeof(PxShader) - 1, 0, NULL, 0, &pShader, &pError))) {
		pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	// シェーダハンドラの生成
	IDirect3DPixelShader9 *pShaderHandler;
	if (FAILED(g_pD3DDev->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), &pShaderHandler))) {
		pShader->Release(); pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	pShader->Release();             // シェーダ命令はもういらない
	if (pError) pError->Release();   // エラー情報はもういらない（NULLの時があるので判定が必要）

									 // 変換行列の設定
	D3DXMATRIX mat, matView, matProj;
	D3DXMatrixIdentity(&mat);
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0, 0, 5), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));
	D3DXMatrixPerspectiveFovLH(&matProj, 0.785398163f, 480.0f / 640.0f, 0.1f, 10000.0f);

	// ピクセルシェーダに切り替え
	g_pD3DDev->SetPixelShader(pShaderHandler);

	// テクスチャの作成
	IDirect3DTexture9 *pTex;
	if (FAILED(D3DXCreateTextureFromFile(g_pD3DDev, _T("Test.JPG"), &pTex))) {
		pShaderHandler->Release(); pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	// テクスチャステージ0番にテクスチャを設定
	g_pD3DDev->SetTexture(0, pTex);


	ShowWindow(hWnd, SW_SHOW);

	// メッセージ ループ
	float sc = 1.0f;        // 彩度調節係数
	float sgn = +1.0f;      // 切り替え符号
	float scale[4] = { 1.0f, 1.0f, 1.0f, 0.0f };      // 彩度
	do {
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else {
			// Direct3Dの処理
			g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
			g_pD3DDev->BeginScene();

			// 彩度計算（0.0f-1.0fの間を繰り返させているだけです）
			sc += 0.0025*sgn;
			if (sc <= 0) { sc = 0.0f;  sgn = +1.0f; }
			else if (sc >= 1) { sc = 1.0f;  sgn = -1.0f; }
			else { sc += 0.0025f*sgn; }
			scale[0] = scale[1] = scale[2] = sc;

			// 描画
			g_pD3DDev->SetPixelShaderConstantF(1, scale, 1);        // 彩度調節係数をGPUの定数レジスタc1に登録
			g_pD3DDev->SetTransform(D3DTS_WORLD, &mat);
			g_pD3DDev->SetTransform(D3DTS_VIEW, &matView);
			g_pD3DDev->SetTransform(D3DTS_PROJECTION, &matProj);
			g_pD3DDev->SetStreamSource(0, pVertex, 0, sizeof(CUSTOMVTX));
			g_pD3DDev->SetFVF(CUSTOMFVF);
			g_pD3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);      // 描画ストリームスタート

			g_pD3DDev->EndScene();
			g_pD3DDev->Present(NULL, NULL, NULL, NULL);
		}
	} while (msg.message != WM_QUIT);

	pShaderHandler->Release();
	pVertex->Release();
	g_pD3DDev->Release();
	g_pD3D->Release();

	return 0;
}