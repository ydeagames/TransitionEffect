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

	// デバッグカメラ作成
	m_pDebugCamera = std::make_unique<DebugCamera>();
	// グリッド床作成
	m_pGridFloor = std::make_unique<GridFloor>(device, ctx, &context.GetStates(), 10.0f, 10);

	// カメラ
	context.GetCamera().view = Matrix::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::Up);

	// プリミティブオブジェクト生成
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>>(ctx);

	// コンパイルされたシェーダファイルを読み込み
	BinaryFile VSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticleVS.cso");
	BinaryFile PSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticlePS.cso");
	BinaryFile GSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticleGS.cso");

	// インプットレイアウト生成
	device->CreateInputLayout(INPUT_LAYOUT.data(),
		INPUT_LAYOUT.size(),
		VSData.GetData(), VSData.GetSize(),
		m_inputLayout.GetAddressOf());
	// 頂点シェーダ作成
	DX::ThrowIfFailed(device->CreateVertexShader(VSData.GetData(), VSData.GetSize(), NULL, m_VertexShader.ReleaseAndGetAddressOf()));
	// ピクセルシェーダ作成
	DX::ThrowIfFailed(device->CreatePixelShader(PSData.GetData(), PSData.GetSize(), NULL, m_PixelShader.ReleaseAndGetAddressOf()));
	// ジオメトリシェーダ作成
	DX::ThrowIfFailed(device->CreateGeometryShader(GSData.GetData(), GSData.GetSize(), NULL, m_GeometryShader.ReleaseAndGetAddressOf()));

	// テクスチャのロード
	CreateWICTextureFromFile(device, L"Resources/Textures/melon.png", nullptr, m_texture.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources/Textures/171.png", nullptr, m_texture2.GetAddressOf());

	// バッファの作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	device->CreateBuffer(&bd, nullptr, &m_CBuffer);

	// モデル作成
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
		o.lpstrFilter = L"pngファイル(*.png)\0*.png\0" L"全てのファイル(*.*)\0*.*\0";
		o.lpstrDefExt = L"TXT";
		o.lpstrTitle = L"ルール画像を指定";
		o.nFilterIndex = 1;
		return GetOpenFileName(&o);
	}
}

void MyGame::Update(GameContext& context)
{
	// デバッグカメラ更新
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

	// グリッド床描画
	//m_pGridFloor->draw(context.GetDR().GetD3DDeviceContext(), context.GetCamera().view, context.GetCamera().projection);

	// モデル
	{
		// 描画
		m_model->Draw(ctx, context.GetStates(), Matrix::Identity, context.GetCamera().view, context.GetCamera().projection);
	}

	// オブジェ
	auto Draw = [&](const Matrix& world, const Matrix& view, const Matrix& proj)
	{
		ID3D11BlendState* blendstate = context.GetStates().NonPremultiplied();
		// 透明判定処理
		ctx->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);
		// 深度バッファに書き込み参照する
		ctx->OMSetDepthStencilState(context.GetStates().DepthRead(), 0);
		// カリングは右周り（時計回り）
		ctx->RSSetState(context.GetStates().CullClockwise());

		ID3D11SamplerState* sampler[1] = { context.GetStates().LinearClamp() };
		ctx->PSSetSamplers(0, 1, sampler);

		ctx->IASetInputLayout(m_inputLayout.Get());

		// 定数バッファ更新
		ConstBuffer cbuff;
		cbuff.matView = view.Transpose();
		cbuff.matProj = proj.Transpose();
		cbuff.matWorld = world.Transpose();
		cbuff.Diffuse = Vector4(1, 1, 1, 1);
		cbuff.time = float(context.GetTimer().GetTotalSeconds());
		cbuff.range = std::max(0.f, (std::sin(cbuff.time) + 1) / 2 * 1.01f);

		// 定数バッファの内容更新
		ctx->UpdateSubresource(m_CBuffer.Get(), 0, NULL, &cbuff, 0, 0);

		// 定数バッファ反映
		ID3D11Buffer* cb[1] = { m_CBuffer.Get() };
		ctx->VSSetConstantBuffers(0, 1, cb);
		ctx->GSSetConstantBuffers(0, 1, cb);
		ctx->PSSetConstantBuffers(0, 1, cb);

		// 描画
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
