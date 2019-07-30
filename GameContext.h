#pragma once
#include "StepTimer.h"
#include "DeviceResources.h"

class GameObject;
class GameCamera;

// GameContext�C���^�[�t�F�C�X
class GameContext
{
public:
	GameContext() = default;
	virtual ~GameContext() = default;

public:
	// DeviceResource�擾
	virtual DX::DeviceResources& GetDR() = 0;
	// �^�C�}�[�擾
	virtual DX::StepTimer& GetTimer() = 0;
	// �J�����擾
	virtual GameCamera& GetCamera() = 0;
	// �R�����X�e�[�g�擾
	virtual DirectX::CommonStates& GetStates() = 0;
};
