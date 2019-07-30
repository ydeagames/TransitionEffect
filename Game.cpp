//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "DebugCamera.h"
#include "GridFloor.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	// ���W�b�N�쐬
	m_myGame = std::make_unique<MyGame>();

	// �}�E�X�̍쐬
	m_pMouse = std::make_unique<Mouse>();
	m_pMouse->SetWindow(window);

	// �L�[�{�[�h�쐬
	m_pKeyboard = std::make_unique<Keyboard>();

    m_deviceResources->SetWindow(window, width, height);
    m_deviceResources->CreateDeviceResources();
	m_deviceResources->CreateWindowSizeDependentResources();

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

	// ���W�b�N�X�V
	m_myGame->Update(*this);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    context;

	// ���W�b�N�`��
	m_myGame->Render(*this);

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;

	// �R�����X�e�[�g�쐬
	m_pState = std::make_unique<CommonStates>(device);

	// ���W�b�N�쐬
	m_myGame->Initialize(*this);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.

	// �E�C���h�E�T�C�Y����A�X�y�N�g����Z�o����
	RECT size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right) / float(size.bottom);

	// ��p��ݒ�
	float fovAngleY = XMConvertToRadians(45.0f);

	// �ˉe�s����쐬����
	GetCamera().projection = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);
	// �r���[�|�[�g�s����쐬����
	GetCamera().viewport = 
		SimpleMath::Matrix::CreateScale(SimpleMath::Vector3(.5f, -.5f, 1.f)) *
		SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(.5f, .5f, 0.f)) *
		SimpleMath::Matrix::CreateScale(SimpleMath::Vector3(float(size.right), float(size.bottom), 1.f));
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

	// ���W�b�N���
	m_myGame->Finalize(*this);
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
