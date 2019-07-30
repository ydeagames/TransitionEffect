//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "GameContext.h"
#include "GameCamera.h"
#include "MyGame.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify, public GameContext
{
public:

	Game() noexcept(false);
	~Game();

	// Initialization and management
	void Initialize(HWND window, int width, int height);

	// Basic game loop
	void Tick();

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowMoved();
	void OnWindowSizeChanged(int width, int height);

	// Properties
	void GetDefaultSize(int& width, int& height) const;

private:

	void Update(DX::StepTimer const& timer);
	void Render();

	void Clear();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	// Window
	HWND									m_window;

	// Device resources.
	std::unique_ptr<DX::DeviceResources>    m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer                           m_timer;

	// �}�E�X
	std::unique_ptr<DirectX::Mouse>			m_pMouse;
	// �}�E�X�g���b�J�[
	DirectX::Mouse::ButtonStateTracker		m_mouseTracker;
	// �L�[�{�[�h
	std::unique_ptr<DirectX::Keyboard>		m_pKeyboard;
	// �L�[�{�[�h�g���b�J�[
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	// �R�����X�e�[�g
	std::unique_ptr<DirectX::CommonStates>	m_pState;

	// �J�����I�u�W�F�N�g
	GameCamera								m_camera;
	// ���W�b�N
	std::unique_ptr<MyGame>					m_myGame;

	// DeviceResource�擾
	DX::DeviceResources& GetDR()
	{
		return *m_deviceResources;
	}
	// �^�C�}�[�擾
	DX::StepTimer& GetTimer()
	{
		return m_timer;
	}
	// �J�����擾
	GameCamera& GetCamera()
	{
		return m_camera;
	}
	// �R�����X�e�[�g�擾
	DirectX::CommonStates& GetStates()
	{
		return *m_pState;
	}
	// �E�B���h�E�擾
	HWND& GetWindowHandle()
	{
		return m_window;
	}
	// �L�[�{�[�h�g���b�J�[�擾
	DirectX::Keyboard::KeyboardStateTracker& GetKeyboardTracker()
	{
		return m_keyboardTracker;
	}
	// �}�E�X�g���b�J�[�擾
	DirectX::Mouse::ButtonStateTracker& GetMouseTracker()
	{
		return m_mouseTracker;
	}
};