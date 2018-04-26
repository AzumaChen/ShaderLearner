// �s�N�Z���V�F�[�_�v���O�����̊�b�e�X�g�v���O����
// ���̃v���O�����Ɋւ���ڂ�������
// ���~����`�ǂ��ƃR���u�Q�[������`�v���O���}�u���V�F�[�_�҂��̂S�A���̂T�v
// �������������B

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>

TCHAR gName[100] = _T("�s�N�Z���V�F�[�_�v���O�����̊�b�e�X�g�v���O����");


/// ���_��` ///
struct CUSTOMVTX
{
	float x, y, z;    // ���_�ʒu
	DWORD color;      // ���_�J���[
	float u, v;     // �e�N�X�`�����W
};

#define CUSTOMFVF  D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1


/// �E�B���h�E�v���V�[�W�� ///
LRESULT CALLBACK WndProc(HWND hWnd, UINT mes, WPARAM wParam, LPARAM lParam) {
	if (mes == WM_DESTROY) { PostQuitMessage(0); return 0; }
	return DefWindowProc(hWnd, mes, wParam, lParam);
}


/// �G���g�� ///
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// �A�v���P�[�V�����̏������i�E�B���h�E�쐬�j
	MSG msg; HWND hWnd;
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, NULL,
		(HBRUSH)(COLOR_WINDOW + 1), NULL, (LPCSTR)gName, NULL };
	if (!RegisterClassEx(&wcex)) return 0;
	if (!(hWnd = CreateWindow(gName, gName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL)))
		return 0;

	// Direct3D�̏�����
	LPDIRECT3D9 g_pD3D;				// Idirect3Device9�C���^�[�t�F�[�X
	LPDIRECT3DDEVICE9 g_pD3DDev;	// �f�o�C�X��Idirect3Device9�C���^�[�t�F�[�X
	if (!(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return 0;	// �擾���s�@return false;
	D3DPRESENT_PARAMETERS d3dpp = { 0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,
		D3DSWAPEFFECT_DISCARD,NULL,TRUE,0,D3DFMT_UNKNOWN,0,0 };

	// IDirect3d9::CreateDevice���\�b�h���s
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
		{
			g_pD3D->Release(); return 0;
		}

	// ���_�̐ݒ�i�����Ȓ����`�j
	CUSTOMVTX v[] =
	{
		{ -1.0f, +1.5f, 0.0f, 0xffffffff, 1.0f, 0.0f },
		{ -1.0f, -1.5f, 0.0f, 0xffffffff, 1.0f, 1.0f },
		{ +1.0f, +1.5f, 0.0f, 0xffffffff, 0.0f, 0.0f },
		{ +1.0f, -1.5f, 0.0f, 0xffffffff, 0.0f, 1.0f }
	};

	// ���_�o�b�t�@�쐬�ƒ��_���̏�������
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


	// �s�N�Z���V�F�[�_���ߔz���`
	// �ʓx���߃V�F�[�_
	// c1 : �ʓx���ߌW�� (c, c, c, 0) (�O���[)0.0f < c < 1.0f(���F)
	const char PxShader[] =
		"ps_1_1                                        \n"
		"def   c0, 0.2989f, 0.5866f, 0.1145f, 0.0f     \n"     // �ʓx�Z�o�W��
		"tex   t0                                      \n"     // �e�N�X�`��0�Ԏg�p
		"dp3   r0, t0, c0                              \n"     // �ʓxY�Z�o(r0.a�Ɋi�[�����)
		"mov   r1, r0.a                                \n"     // r1�̊e������Y�Ŗ��߂�
		"lrp   r0, c1, t0, r1                          \n";    // ���`���( t0 + c1*(Y-t0) )

															   // �V�F�[�_���߂̃R���p�C��
	ID3DXBuffer *pShader;   // �V�F�[�_���ߊi�[�o�b�t�@
	ID3DXBuffer *pError;     // �R���p�C���G���[���i�[�o�b�t�@
	if (FAILED(D3DXAssembleShader(PxShader, sizeof(PxShader) - 1, 0, NULL, 0, &pShader, &pError))) {
		pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	// �V�F�[�_�n���h���̐���
	IDirect3DPixelShader9 *pShaderHandler;
	if (FAILED(g_pD3DDev->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), &pShaderHandler))) {
		pShader->Release(); pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	pShader->Release();             // �V�F�[�_���߂͂�������Ȃ�
	if (pError) pError->Release();   // �G���[���͂�������Ȃ��iNULL�̎�������̂Ŕ��肪�K�v�j

									 // �ϊ��s��̐ݒ�
	D3DXMATRIX mat, matView, matProj;
	D3DXMatrixIdentity(&mat);
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0, 0, 5), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));
	D3DXMatrixPerspectiveFovLH(&matProj, 0.785398163f, 480.0f / 640.0f, 0.1f, 10000.0f);

	// �s�N�Z���V�F�[�_�ɐ؂�ւ�
	g_pD3DDev->SetPixelShader(pShaderHandler);

	// �e�N�X�`���̍쐬
	IDirect3DTexture9 *pTex;
	if (FAILED(D3DXCreateTextureFromFile(g_pD3DDev, _T("Test.JPG"), &pTex))) {
		pShaderHandler->Release(); pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	// �e�N�X�`���X�e�[�W0�ԂɃe�N�X�`����ݒ�
	g_pD3DDev->SetTexture(0, pTex);


	ShowWindow(hWnd, SW_SHOW);

	// ���b�Z�[�W ���[�v
	float sc = 1.0f;        // �ʓx���ߌW��
	float sgn = +1.0f;      // �؂�ւ�����
	float scale[4] = { 1.0f, 1.0f, 1.0f, 0.0f };      // �ʓx
	do {
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else {
			// Direct3D�̏���
			g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
			g_pD3DDev->BeginScene();

			// �ʓx�v�Z�i0.0f-1.0f�̊Ԃ��J��Ԃ����Ă��邾���ł��j
			sc += 0.0025*sgn;
			if (sc <= 0) { sc = 0.0f;  sgn = +1.0f; }
			else if (sc >= 1) { sc = 1.0f;  sgn = -1.0f; }
			else { sc += 0.0025f*sgn; }
			scale[0] = scale[1] = scale[2] = sc;

			// �`��
			g_pD3DDev->SetPixelShaderConstantF(1, scale, 1);        // �ʓx���ߌW����GPU�̒萔���W�X�^c1�ɓo�^
			g_pD3DDev->SetTransform(D3DTS_WORLD, &mat);
			g_pD3DDev->SetTransform(D3DTS_VIEW, &matView);
			g_pD3DDev->SetTransform(D3DTS_PROJECTION, &matProj);
			g_pD3DDev->SetStreamSource(0, pVertex, 0, sizeof(CUSTOMVTX));
			g_pD3DDev->SetFVF(CUSTOMFVF);
			g_pD3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);      // �`��X�g���[���X�^�[�g

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