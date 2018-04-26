// 頂点シェーダプログラムの基礎テストプログラム
// このプログラムに関する詳しい情報は
// ○×つくろ～どっとコム「ゲームつくろ～プログラマブルシェーダ編その２、その３」
// をご覧下さい。

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>

TCHAR gName[100] = _T("頂点シェーダプログラムの基礎テストプログラム");


/// 頂点フォーマット定義 ///
struct CUSTOMVTX
{
	float x, y, z;    // 頂点位置
	DWORD color;         // 頂点カラー
};

#define CUSTOMFVF  D3DFVF_XYZ | D3DFVF_DIFFUSE


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
	LPDIRECT3D9 g_pD3D;   LPDIRECT3DDEVICE9 g_pD3DDev;
	if (!(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return 0;
	D3DPRESENT_PARAMETERS d3dpp = { 0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,
		D3DSWAPEFFECT_DISCARD,NULL,TRUE,0,D3DFMT_UNKNOWN,0,0 };

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
	{
		g_pD3D->Release(); return 0;
	}

	// 頂点の設定（小さな長方形）
	CUSTOMVTX v[] =
	{
		{ -0.7f, -1.5f, 0.0f, 0xff0000ff },
		{ +0.7f, -0.5f, 0.0f, 0xffff0000 },
		{ +0.7f, +0.5f, 0.0f, 0xff00ff00 },
		{ -0.7f, +0.5f, 0.0f, 0xffffffff }
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

	// 頂点データ宣言（シェーダに伝える）
	D3DVERTEXELEMENT9 VtxElem[] = {
		{ 0,     0,  D3DDECLTYPE_FLOAT3  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },		  // 頂点座標
		{ 0,     12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 0 },     // 頂点カラー（Diffuse）
		D3DDECL_END()
	};
	IDirect3DVertexDeclaration9 *pDec;
	if (FAILED(g_pD3DDev->CreateVertexDeclaration(VtxElem, &pDec))) {
		pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	// シェーダ命令配列定義
	const char VShader[] =
		"vs_1_1                              \n"
		"dcl_position    v0                  \n"
		"dcl_color       v1                  \n"
		"m4x4            oPos,   v0,     c0  \n"     // 頂点に変換行列を掛け算　oPos=v0・c0
		"add             oD0 ,   v1,     c4  \n";    // 頂点カラーに定数を足し算して出力

													 // シェーダ命令のコンパイル
	ID3DXBuffer *pShader;   // シェーダ命令格納バッファ
	ID3DXBuffer *pError;     // コンパイルエラー情報格納バッファ
	if (FAILED(D3DXAssembleShader(VShader, sizeof(VShader) - 1, 0, NULL, 0, &pShader, &pError))) {
		pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}
	//ソース上から読み込む場合はD3DXAssembleShader関数
	//ファイルから読み込む場合はD3DXAssembleShaderFromFile関数

	// シェーダハンドラの生成
	IDirect3DVertexShader9 *pShaderHandler;
	// ピクセルシェーダプログラムのコンパイルが成功したら、シェーダ命令がバッファに格納されます
	// それを扱ってGPUとやり取りするためのピクセルシェーダハンドラ（IDirect3DPixelShader9）を次に生成します
	// 生成する関数はIDirect3DDevice9::CreatePixelShader
	if (FAILED(g_pD3DDev->CreateVertexShader((DWORD*)pShader->GetBufferPointer(), &pShaderHandler))) {
		pShader->Release(); pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	pShader->Release();                             // シェーダ命令はもういらない
	if (pError) pError->Release();   // エラー情報はもういらない（NULLの時があるので判定が必要）

									 // 変換行列の設定
	D3DXMATRIX mat, matView, matProj;
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0, 0, 5), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));
	D3DXMatrixPerspectiveFovLH(&matProj, 0.785398163f, 480.0f / 640.0f, 0.1f, 10000.0f);
	D3DXMatrixMultiply(&mat, &matView, &matProj);

	// 変換行列を転置して定数レジスタc0～c3に登録する
	D3DXMatrixTranspose(&mat, &mat);
	g_pD3DDev->SetVertexShaderConstantF(0, (float*)&mat, 4);

	/*
	ピクセルのレンダリング
	// 描画
	pD3DDev9->SetStreamSource( 0, m_pVertex, sizeof(CUSTOMVTX) ); 
	pD3DDev9->SetFVF( CUSTOMFVF );
	pD3DDev9->SetPixelShader( pShaderHandler );
	pD3DDev9->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
	*/

	// 頂点シェーダに切り替え
	g_pD3DDev->SetVertexShader(pShaderHandler);

	ShowWindow(hWnd, SW_SHOW);

	// メッセージ ループ
	float c = 0.0f;
	float addcolor[4];      // 頂点カラー(RGB)に足す値

	do {
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else {
			// Direct3Dの処理
			g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
			g_pD3DDev->BeginScene();	// シーンの描画開始

			// 描画
			c += 0.0f;    // 足す値
			addcolor[0] = addcolor[1] = addcolor[2] = addcolor[3] = c;    // R,G,B値に加算値を振り分け

			if (c >= 1.0f)
				c = 0.0f;

			g_pD3DDev->SetVertexShaderConstantF(4, (float*)&addcolor, 1); // 色情報を更新
			g_pD3DDev->SetStreamSource(0, pVertex, 0, sizeof(CUSTOMVTX));
			g_pD3DDev->SetVertexDeclaration(pDec);  // 頂点宣言を登録
			//g_pD3DDev->SetFVF(CUSTOMFVF);

			g_pD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);      // ストリームスタート

			g_pD3DDev->EndScene();	// シーンの描画終了
			g_pD3DDev->Present(NULL, NULL, NULL, NULL);
		}
	} while (msg.message != WM_QUIT);

	pShaderHandler->Release();
	pVertex->Release();
	g_pD3DDev->Release();
	g_pD3D->Release();

	return 0;
}