// ���_�V�F�[�_�v���O�����̊�b�e�X�g�v���O����
// ���̃v���O�����Ɋւ���ڂ�������
// ���~����`�ǂ��ƃR���u�Q�[������`�v���O���}�u���V�F�[�_�҂��̂Q�A���̂R�v
// �������������B

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>

TCHAR gName[100] = _T("���_�V�F�[�_�v���O�����̊�b�e�X�g�v���O����");


/// ���_�t�H�[�}�b�g��` ///
struct CUSTOMVTX
{
	float x, y, z;    // ���_�ʒu
	DWORD color;         // ���_�J���[
};

#define CUSTOMFVF  D3DFVF_XYZ | D3DFVF_DIFFUSE


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
	LPDIRECT3D9 g_pD3D;   LPDIRECT3DDEVICE9 g_pD3DDev;
	if (!(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return 0;
	D3DPRESENT_PARAMETERS d3dpp = { 0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,
		D3DSWAPEFFECT_DISCARD,NULL,TRUE,0,D3DFMT_UNKNOWN,0,0 };

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
	{
		g_pD3D->Release(); return 0;
	}

	// ���_�̐ݒ�i�����Ȓ����`�j
	CUSTOMVTX v[] =
	{
		{ -0.7f, -1.5f, 0.0f, 0xff0000ff },
		{ +0.7f, -0.5f, 0.0f, 0xffff0000 },
		{ +0.7f, +0.5f, 0.0f, 0xff00ff00 },
		{ -0.7f, +0.5f, 0.0f, 0xffffffff }
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

	// ���_�f�[�^�錾�i�V�F�[�_�ɓ`����j
	D3DVERTEXELEMENT9 VtxElem[] = {
		{ 0,     0,  D3DDECLTYPE_FLOAT3  , D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },		  // ���_���W
		{ 0,     12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR   , 0 },     // ���_�J���[�iDiffuse�j
		D3DDECL_END()
	};
	IDirect3DVertexDeclaration9 *pDec;
	if (FAILED(g_pD3DDev->CreateVertexDeclaration(VtxElem, &pDec))) {
		pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	// �V�F�[�_���ߔz���`
	const char VShader[] =
		"vs_1_1                              \n"
		"dcl_position    v0                  \n"
		"dcl_color       v1                  \n"
		"m4x4            oPos,   v0,     c0  \n"     // ���_�ɕϊ��s����|���Z�@oPos=v0�Ec0
		"add             oD0 ,   v1,     c4  \n";    // ���_�J���[�ɒ萔�𑫂��Z���ďo��

													 // �V�F�[�_���߂̃R���p�C��
	ID3DXBuffer *pShader;   // �V�F�[�_���ߊi�[�o�b�t�@
	ID3DXBuffer *pError;     // �R���p�C���G���[���i�[�o�b�t�@
	if (FAILED(D3DXAssembleShader(VShader, sizeof(VShader) - 1, 0, NULL, 0, &pShader, &pError))) {
		pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}
	//�\�[�X�ォ��ǂݍ��ޏꍇ��D3DXAssembleShader�֐�
	//�t�@�C������ǂݍ��ޏꍇ��D3DXAssembleShaderFromFile�֐�

	// �V�F�[�_�n���h���̐���
	IDirect3DVertexShader9 *pShaderHandler;
	// �s�N�Z���V�F�[�_�v���O�����̃R���p�C��������������A�V�F�[�_���߂��o�b�t�@�Ɋi�[����܂�
	// �����������GPU�Ƃ���肷�邽�߂̃s�N�Z���V�F�[�_�n���h���iIDirect3DPixelShader9�j�����ɐ������܂�
	// ��������֐���IDirect3DDevice9::CreatePixelShader
	if (FAILED(g_pD3DDev->CreateVertexShader((DWORD*)pShader->GetBufferPointer(), &pShaderHandler))) {
		pShader->Release(); pVertex->Release(); g_pD3DDev->Release(); g_pD3D->Release(); return 0;
	}

	pShader->Release();                             // �V�F�[�_���߂͂�������Ȃ�
	if (pError) pError->Release();   // �G���[���͂�������Ȃ��iNULL�̎�������̂Ŕ��肪�K�v�j

									 // �ϊ��s��̐ݒ�
	D3DXMATRIX mat, matView, matProj;
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0, 0, 5), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));
	D3DXMatrixPerspectiveFovLH(&matProj, 0.785398163f, 480.0f / 640.0f, 0.1f, 10000.0f);
	D3DXMatrixMultiply(&mat, &matView, &matProj);

	// �ϊ��s���]�u���Ē萔���W�X�^c0�`c3�ɓo�^����
	D3DXMatrixTranspose(&mat, &mat);
	g_pD3DDev->SetVertexShaderConstantF(0, (float*)&mat, 4);

	/*
	�s�N�Z���̃����_�����O
	// �`��
	pD3DDev9->SetStreamSource( 0, m_pVertex, sizeof(CUSTOMVTX) ); 
	pD3DDev9->SetFVF( CUSTOMFVF );
	pD3DDev9->SetPixelShader( pShaderHandler );
	pD3DDev9->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
	*/

	// ���_�V�F�[�_�ɐ؂�ւ�
	g_pD3DDev->SetVertexShader(pShaderHandler);

	ShowWindow(hWnd, SW_SHOW);

	// ���b�Z�[�W ���[�v
	float c = 0.0f;
	float addcolor[4];      // ���_�J���[(RGB)�ɑ����l

	do {
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else {
			// Direct3D�̏���
			g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
			g_pD3DDev->BeginScene();	// �V�[���̕`��J�n

			// �`��
			c += 0.0f;    // �����l
			addcolor[0] = addcolor[1] = addcolor[2] = addcolor[3] = c;    // R,G,B�l�ɉ��Z�l��U�蕪��

			if (c >= 1.0f)
				c = 0.0f;

			g_pD3DDev->SetVertexShaderConstantF(4, (float*)&addcolor, 1); // �F�����X�V
			g_pD3DDev->SetStreamSource(0, pVertex, 0, sizeof(CUSTOMVTX));
			g_pD3DDev->SetVertexDeclaration(pDec);  // ���_�錾��o�^
			//g_pD3DDev->SetFVF(CUSTOMFVF);

			g_pD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);      // �X�g���[���X�^�[�g

			g_pD3DDev->EndScene();	// �V�[���̕`��I��
			g_pD3DDev->Present(NULL, NULL, NULL, NULL);
		}
	} while (msg.message != WM_QUIT);

	pShaderHandler->Release();
	pVertex->Release();
	g_pD3DDev->Release();
	g_pD3D->Release();

	return 0;
}