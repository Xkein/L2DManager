#include "l2dManager.h"
#include "l2dModel.h"
#include "l2dTextureManager.h"


Live2DManager* Live2DManager::instance = nullptr;

Live2DManager::Live2DManager()
	: pDirect3D(nullptr)
	, pLostStep(nullptr)
	, pDevice(nullptr)
	, clientRectangle()
	, CommitFunction(nullptr)
{
	clearColor[0] = 0.0f;
	clearColor[1] = 0.0f;
	clearColor[2] = 0.0f;
	clearColor[3] = 0.0f;
	scaleX = 1.0f;
	scaleY = 1.0f;
	deviceToScreen = new Csm::CubismMatrix44();
	viewMatrix = new Csm::CubismViewMatrix();
	textureManager = new Live2DTextureManager();
}

Live2DManager::~Live2DManager()
{
	ReleaseAllModel();

	for (int i = releaseModel.GetSize() - 1; i >= 0; i--)
	{
		if (releaseModel[i].pModel)
		{
			delete releaseModel[i].pModel;
		}
	}
	releaseModel.Clear();

	delete textureManager;

	Csm::CubismFramework::Dispose();
}

Live2DManager* Live2DManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new Live2DManager();
	}

	return instance;
}

void Live2DManager::ReleaseInstance()
{
	if (instance != nullptr) {
		delete instance;
	}

	instance = nullptr;
}

void Live2DManager::Initialize(RECT clientRect, IDirect3D* direct3D, IDirect3DDevice* device, CommitFunctionType commitFunction, int* pLostStep)
{
	pDirect3D = direct3D;
	pDevice = device;
	CommitFunction = commitFunction;
	clientRectangle = clientRect;
	this->pLostStep = pLostStep;

	int width, height;
	GetClientSize(width, height);
	SetClientSize(width, height);

	Csm::CubismFramework::StartUp(&allocator);
	Csm::CubismFramework::Initialize();
	CubismRenderer_D3D::InitializeConstantSettings(1, pDevice);

	Live2DPal::UpdateTime();

}


void Live2DManager::SetClientSize(int width, int height)
{
	using namespace Live2DDefine;
	clientRectangle.right = clientRectangle.left + width;
	clientRectangle.bottom = clientRectangle.top + height;

	float ratio = static_cast<float>(height) / static_cast<float>(width);
	float left = ViewLogicalLeft;
	float right = ViewLogicalRight;
	float bottom = -ratio;
	float top = ratio;

	viewMatrix->SetScreenRect(left, right, bottom, top); // デバイスに対応する画面の範囲。 Xの左端, Xの右端, Yの下端, Yの上端

	float screenW = fabsf(left - right);
	deviceToScreen->LoadIdentity(); // サイズが変わった際などリセット必須 
	deviceToScreen->ScaleRelative(screenW / width, -screenW / width);
	deviceToScreen->TranslateRelative(-width * 0.5f, -height * 0.5f);

	// 表示範囲の設定
	viewMatrix->SetMaxScale(ViewMaxScale); // 限界拡大率
	viewMatrix->SetMinScale(ViewMinScale); // 限界縮小率

											// 表示できる最大範囲
	viewMatrix->SetMaxScreenRect(
		ViewLogicalMaxLeft,
		ViewLogicalMaxRight,
		ViewLogicalMaxBottom,
		ViewLogicalMaxTop
	);
}

void Live2DManager::OpenScene(const Csm::csmChar* dir, const Csm::csmChar* name)
{
	modelDir = dir;
	modelName = name;

	std::string modelJsonName = modelName + ".model3.json";

	ReleaseAllModel();
	models.PushBack(new Live2DModel());
	models[0]->LoadAssets(modelDir.c_str(), modelJsonName.c_str());

	clearColor[0] = 1.0f;
	clearColor[1] = 1.0f;
	clearColor[2] = 1.0f;
	clearColor[3] = 0.0f;
}

void Live2DManager::ReleaseAllTexture()
{
	textureManager->ReleaseTextures();
}

void Live2DManager::ReleaseAllModel()
{
	for (Csm::csmUint32 i = 0; i < models.GetSize(); i++)
	{

		models[i]->DeleteMark();

		ReleaseModel rel;
		rel.pModel = models[i];
		rel.counter = 2;
		releaseModel.PushBack(rel);
	}

	models.Clear();
}

void Live2DManager::OnDrag(Csm::csmFloat32 x, Csm::csmFloat32 y) const
{
	for (Csm::csmUint32 i = 0; i < models.GetSize(); i++)
	{
		auto* model = models[i];

		model->SetDragging(x, y);
	}
}

void Live2DManager::OnTap(Csm::csmFloat32 x, Csm::csmFloat32 y)
{
	using namespace Live2DDefine;

	for (Csm::csmUint32 i = 0; i < models.GetSize(); i++)
	{
		if (models[i]->HitTest(HitAreaNameHead, x, y))
		{
			models[i]->SetRandomExpression();
		}
		else if (models[i]->HitTest(HitAreaNameBody, x, y))
		{
			models[i]->StartRandomMotion(MotionGroupTapBody, PriorityNormal);
		}
	}
}

void Live2DManager::OnUpdate() const
{
	using namespace Live2D::Cubism::Framework;
	int width, height;
	GetClientSize(width, height);

	CubismRenderer_D3D::StartFrame(GetD3dDevice(), width, height);

	csmUint32 modelCount = models.GetSize();
	for (csmUint32 i = 0; i < modelCount; ++i)
	{
		auto* model = models[i];
		CubismMatrix44 projection;
		auto pModel = model->GetModel();
		//projection.Scale(model->GetModel()->GetCanvasWidth(), model->GetModel()->GetCanvasHeight());
		projection.Scale(1.0f, static_cast<float>(width) / static_cast<float>(height));
		if (viewMatrix) {
			projection.MultiplyByMatrix(viewMatrix);
		}
		projection.ScaleRelative(2.0f, 2.0f);
		projection.ScaleRelative(scaleX, scaleY);

		PreModelDraw(*model);
		model->Update();
		model->Draw(projection);

		PostModelDraw(*model);
		CommitChange(*model);
	}

	CubismRenderer_D3D::EndFrame(GetD3dDevice());

}

void Live2DManager::EndFrame()
{
	for (int i = releaseModel.GetSize() - 1; i >= 0; i--)
	{
		releaseModel[i].counter--;

		if (releaseModel[i].counter <= 0)
		{
			if (releaseModel[i].pModel)
			{
				delete releaseModel[i].pModel;
			}
			releaseModel.Remove(i);
			continue;
		}
	}
}

void Live2DManager::PreModelDraw(Live2DModel& refModel) const
{
	/*
	if (!IsLostStep())
	{

		CubismOffscreenFrame_D3D* useTarget = NULL;

		useTarget = &refModel.GetRenderBuffer();

		if (!useTarget->IsValid())
		{
			int width, height;
			GetClientSize(width, height);

			if (width != 0 && height != 0)
			{
				// モデル描画キャンバス 
				useTarget->CreateOffscreenFrame(GetD3dDevice(),
					static_cast<Csm::csmUint32>(width), static_cast<Csm::csmUint32>(height));
			}
		}

		useTarget->BeginDraw(GetD3dDevice());
		useTarget->Clear(GetD3dDevice(), clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	}
	*/
}

void Live2DManager::PostModelDraw(Live2DModel& refModel) const
{
	/*
	if (!IsLostStep())
	{

		CubismOffscreenFrame_D3D* useTarget = &refModel.GetRenderBuffer();

		useTarget->EndDraw(GetD3dDevice());

	}
	*/
}

void Live2DManager::OnDeviceLost(IDirect3DDevice* device)
{
	for (Csm::csmUint32 i = 0; i < models.GetSize(); i++)
	{
		models[i]->OnDeviceLost();
	}

	pDevice = nullptr;

	ReleaseAllTexture();

	CubismRenderer_D3D::OnDeviceLost();
}

void Live2DManager::RestoreDeviceLost(IDirect3DDevice* device)
{
	pDevice = device;
	CubismRenderer_D3D::InitializeConstantSettings(1, pDevice);
	for (Csm::csmUint32 i = 0; i < models.GetSize(); i++)
	{
		models[i]->ReloadRenderer();
	}
}

void Live2DManager::CommitChange(Live2DModel& refModel) const
{
	auto* matrix = refModel.GetModelMatrix();
	CommitFunction();
}

Csm::csmUint32 Live2DManager::GetModelNum() const
{
	return models.GetSize();
}

Live2DModel* Live2DManager::GetModel(Csm::csmUint32 no) const
{
	if (no < models.GetSize())
	{
		return models[no];
	}

	return NULL;
}
