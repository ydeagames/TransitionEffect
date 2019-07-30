#include "pch.h"
#include <commdlg.h>
#include "MyGame.h"
#include "GameCamera.h"
#include "GameContext.h"
#include "BinaryFile.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

const std::vector<D3D11_INPUT_ELEMENT_DESC> MyGame::INPUT_LAYOUT =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vector3) + sizeof(Vector4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

void MyGame::Initialize(GameContext& context)
{
	auto device = context.GetDR().GetD3DDevice();
	auto ctx = context.GetDR().GetD3DDeviceContext();

	// �f�o�b�O�J�����쐬
	m_pDebugCamera = std::make_unique<DebugCamera>();
	// �O���b�h���쐬
	m_pGridFloor = std::make_unique<GridFloor>(device, ctx, &context.GetStates(), 10.0f, 10);

	// �J����
	context.GetCamera().view = Matrix::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::Up);

	// �v���~�e�B�u�I�u�W�F�N�g����
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>>(ctx);

	// �R���p�C�����ꂽ�V�F�[�_�t�@�C����ǂݍ���
	BinaryFile VSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticleVS.cso");
	BinaryFile PSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticlePS.cso");
	BinaryFile GSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticleGS.cso");

	// �C���v�b�g���C�A�E�g����
	device->CreateInputLayout(INPUT_LAYOUT.data(),
		INPUT_LAYOUT.size(),
		VSData.GetData(), VSData.GetSize(),
		m_inputLayout.GetAddressOf());
	// ���_�V�F�[�_�쐬
	DX::ThrowIfFailed(device->CreateVertexShader(VSData.GetData(), VSData.GetSize(), NULL, m_VertexShader.ReleaseAndGetAddressOf()));
	// �s�N�Z���V�F�[�_�쐬
	DX::ThrowIfFailed(device->CreatePixelShader(PSData.GetData(), PSData.GetSize(), NULL, m_PixelShader.ReleaseAndGetAddressOf()));
	// �W�I���g���V�F�[�_�쐬
	DX::ThrowIfFailed(device->CreateGeometryShader(GSData.GetData(), GSData.GetSize(), NULL, m_GeometryShader.ReleaseAndGetAddressOf()));

	// �e�N�X�`���̃��[�h
	CreateWICTextureFromFile(device, L"Resources/Textures/melon.png", nullptr, m_texture.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources/Textures/171.png", nullptr, m_texture2.GetAddressOf());

	// �o�b�t�@�̍쐬
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	device->CreateBuffer(&bd, nullptr, &m_CBuffer);

	// ���f���쐬
	auto fxFactory = EffectFactory(context.GetDR().GetD3DDevice());
	fxFactory.SetDirectory(L"Resources");
	m_model = DirectX::Model::CreateFromCMO(context.GetDR().GetD3DDevice(), L"Resources/cup.cmo", fxFactory);
}

namespace
{
	BOOL GetFileName(HWND hWnd, wchar_t* fname, int sz, wchar_t* initDir) {
		OPENFILENAME o;
		fname[0] = L'\0';
		ZeroMemory(&o, sizeof(o));
		o.lStructSize = sizeof(o);
		o.hwndOwner = hWnd;
		o.lpstrInitialDir = initDir;
		o.lpstrFile = fname;
		o.nMaxFile = sz;
		o.lpstrFilter = L"png�t�@�C��(*.png)\0*.png\0" L"�S�Ẵt�@�C��(*.*)\0*.*\0";
		o.lpstrDefExt = L"TXT";
		o.lpstrTitle = L"���[���摜���w��";
		o.nFilterIndex = 1;
		return GetOpenFileName(&o);
	}
}

void MyGame::Update(GameContext& context)
{
	// �f�o�b�O�J�����X�V
	m_pDebugCamera->update();
	context.GetCamera().view = m_pDebugCamera->getViewMatrix();

	if (context.GetKeyboardTracker().IsKeyPressed(Keyboard::Keys::Space))
	{
		wchar_t fname[256];
		if (GetFileName(context.GetWindowHandle(), fname, 256, nullptr))
		{
			CreateWICTextureFromFile(context.GetDR().GetD3DDevice(), fname, nullptr, m_texture2.ReleaseAndGetAddressOf());
		}
	}
}

void MyGame::Render(GameContext& context)
{
	auto ctx = context.GetDR().GetD3DDeviceContext();

	// �O���b�h���`��
	//m_pGridFloor->draw(context.GetDR().GetD3DDeviceContext(), context.GetCamera().view, context.GetCamera().projection);

	// ���f��
	{
		// �`��
		m_model->Draw(ctx, context.GetStates(), Matrix::Identity, context.GetCamera().view, context.GetCamera().projection);
	}

	// �I�u�W�F
	auto Draw = [&](const Matrix& world, const Matrix& view, const Matrix& proj)
	{
		ID3D11BlendState* blendstate = context.GetStates().NonPremultiplied();
		// �������菈��
		ctx->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);
		// �[�x�o�b�t�@�ɏ������ݎQ�Ƃ���
		ctx->OMSetDepthStencilState(context.GetStates().DepthRead(), 0);
		// �J�����O�͉E����i���v���j
		ctx->RSSetState(context.GetStates().CullClockwise());

		ID3D11SamplerState* sampler[1] = { context.GetStates().LinearClamp() };
		ctx->PSSetSamplers(0, 1, sampler);

		ctx->IASetInputLayout(m_inputLayout.Get());

		// �萔�o�b�t�@�X�V
		ConstBuffer cbuff;
		cbuff.matView = view.Transpose();
		cbuff.matProj = proj.Transpose();
		cbuff.matWorld = world.Transpose();
		cbuff.Diffuse = Vector4(1, 1, 1, 1);
		cbuff.time = float(context.GetTimer().GetTotalSeconds());
		cbuff.range = std::max(0.f, (std::sin(cbuff.time) + 1) / 2 * 1.01f);

		// �萔�o�b�t�@�̓��e�X�V
		ctx->UpdateSubresource(m_CBuffer.Get(), 0, NULL, &cbuff, 0, 0);

		// �萔�o�b�t�@���f
		ID3D11Buffer* cb[1] = { m_CBuffer.Get() };
		ctx->VSSetConstantBuffers(0, 1, cb);
		ctx->GSSetConstantBuffers(0, 1, cb);
		ctx->PSSetConstantBuffers(0, 1, cb);

		// �`��
		ctx->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		ctx->PSSetShader(m_PixelShader.Get(), nullptr, 0);
		ctx->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
		ctx->PSSetShaderResources(1, 1, m_texture2.GetAddressOf());
		ctx->GSSetShader(m_GeometryShader.Get(), nullptr, 0);
		static std::vector<VertexPositionColorTexture> vertices = {
			VertexPositionColorTexture(Vector3(0.5f, 0.5f, 0.0f), Vector4::One, Vector2(1.0f, 0.0f)),
			VertexPositionColorTexture(Vector3(-0.5f, 0.5f, 0.0f), Vector4::One, Vector2(0.0f, 0.0f)),
			VertexPositionColorTexture(Vector3(-0.5f,-0.5f, 0.0f), Vector4::One, Vector2(0.0f, 1.0f)),
			VertexPositionColorTexture(Vector3(0.5f, -0.5f, 0.0f), Vector4::One, Vector2(1.0f, 1.0f)),
		};
		static std::vector<uint16_t> indices = {
			0, 1, 2, 0, 2, 3
		};
		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indices.data(), indices.size(), vertices.data(), vertices.size());
		m_primitiveBatch->End();

		// Shader Reset
		ctx->VSSetShader(nullptr, nullptr, 0);
		ctx->PSSetShader(nullptr, nullptr, 0);
		ctx->GSSetShader(nullptr, nullptr, 0);
	};
	Draw(Matrix::CreateScale(2), Matrix::Identity, Matrix::Identity);
}

void MyGame::Finalize(GameContext& context)
{

}
