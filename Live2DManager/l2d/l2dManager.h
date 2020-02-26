#pragma once

#include "l2dAllocator.h"
#include "l2dd3d.h"
#include "l2dTouchManager.h"

#include <Math\CubismViewMatrix.hpp>

#include <Rendering\CubismRenderer.hpp>
#include <string>

class Live2DModel;
class Live2DTextureManager;

class Live2DManager
{
private:
	Live2DManager();

	~Live2DManager();

	static Live2DManager* instance;
	typedef void (*CommitFunctionType)();

public:

	static Live2DManager* GetInstance();

	static void ReleaseInstance();

	void Initialize(RECT clientRect, IDirect3D* direct3D, IDirect3DDevice* device, CommitFunctionType commitFunction, int* pLostStep);


	void OpenScene(const Csm::csmChar* dir, const Csm::csmChar* name);


	void ReleaseAllModel();

	void ReleaseAllTexture();

	void OnDrag(Csm::csmFloat32 x, Csm::csmFloat32 y) const;

	void OnTap(Csm::csmFloat32 x, Csm::csmFloat32 y);


	void OnUpdate() const;

	void EndFrame();

	void PreModelDraw(Live2DModel& refModel) const;

	void PostModelDraw(Live2DModel& refModel) const;

	void GetClientSize(int& rWidth, int& rHeight) const
	{
		rWidth = (clientRectangle.right - clientRectangle.left);
		rHeight = (clientRectangle.bottom - clientRectangle.top);
	}

	void SetClientSize(int width, int height);

	bool IsLostStep() const
	{
		return *pLostStep != 0;
	}

	float GetSpriteAlpha(int assign) const
	{
		float alpha = 0.25f + static_cast<float>(assign) * 0.5f;
		if (alpha > 1.0f)
		{
			alpha = 1.0f;
		}
		if (alpha < 0.1f)
		{
			alpha = 0.1f;
		}

		return alpha;
	}

	void OnDeviceLost(IDirect3DDevice* device);

	void RestoreDeviceLost(IDirect3DDevice* device);

	void CommitChange(Live2DModel& refModel) const;

	static IDirect3DDevice* GetD3dDevice()
	{
		if (instance == NULL)
		{
			return NULL;
		}
		return instance->pDevice;
	}

	Live2DTextureManager* GetTextureManager()
	{
		return textureManager;
	}

	Csm::csmUint32 GetModelNum() const;


	Live2DModel* GetModel(Csm::csmUint32 no) const;

	void TouchesBegan(float deviceX, float deviceY)
	{
		touchManager.TouchesBegan(deviceX, deviceY);
	}

	void TouchesMoved(float deviceX, float deviceY)
	{
		float viewX = TransformViewX(touchManager.GetX());
		float viewY = TransformViewY(touchManager.GetY());

		touchManager.TouchesMoved(deviceX, deviceY);

		OnDrag(viewX, viewY);
	}

	void TouchesEnded(float px, float py)
	{
		OnDrag(0.0f, 0.0f);
		float x = deviceToScreen->TransformX(px); // 論理座標変換した座標を取得。
		float y = deviceToScreen->TransformY(py); // 論理座標変換した座標を取得。
		OnTap(x, y);
	}

	float TransformViewX(float deviceX)
	{
		float screenX = deviceToScreen->TransformX(deviceX); // 論理座標変換した座標を取得。
		return viewMatrix->InvertTransformX(screenX); // 拡大、縮小、移動後の値。
	}

	float TransformViewY(float deviceY)
	{
		float screenY = deviceToScreen->TransformY(deviceY); // 論理座標変換した座標を取得。
		return viewMatrix->InvertTransformY(screenY); // 拡大、縮小、移動後の値。
	}

	void SetRenderScale(float x, float y)
	{
		scaleX = x;
		scaleY = y;
	}

public:
	struct ReleaseModel
	{
		ReleaseModel()
		{
			pModel = NULL;
			counter = 0;
		}

		Live2DModel* pModel;
		int         counter;
	};
private:

	Live2DAllocator allocator;
	TouchManager touchManager;
	Live2DTextureManager* textureManager;

	//Csm::CubismMatrix44* viewMatrix;
	Csm::CubismMatrix44* deviceToScreen;    ///< デバイスからスクリーンへの行列
	Csm::CubismViewMatrix* viewMatrix;      ///< viewMatrix


	Csm::csmVector<Live2DModel*> models;
	Csm::csmVector<ReleaseModel> releaseModel;
	Csm::csmMap<Live2DModel*, int> releaseModels;

	// must be init

	CommitFunctionType CommitFunction;
	RECT clientRectangle;
	IDirect3D* pDirect3D;
	IDirect3DDevice* pDevice;
	std::string modelDir;
	std::string modelName;
	float clearColor[4];
	int* pLostStep;
	float scaleX;
	float scaleY;
};
