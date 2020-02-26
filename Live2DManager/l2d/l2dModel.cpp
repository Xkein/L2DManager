#include "l2dModel.h"
#include "l2dTextureManager.h"

Live2DModel::Live2DModel()
    : CubismUserModel()
    , modelSetting(nullptr)
    , userTimeSeconds(0.0f)
    , deleteModel(false)
{
    using namespace Live2D::Cubism::Framework::DefaultParameterId;

    auto* idManager = Csm::CubismFramework::GetIdManager();
    idParam.AngleX = idManager->GetId(ParamAngleX);
    idParam.AngleY = idManager->GetId(ParamAngleY);
    idParam.AngleZ = idManager->GetId(ParamAngleZ);
    idParam.BodyAngleX = idManager->GetId(ParamBodyAngleX);
    idParam.EyeBallX = idManager->GetId(ParamEyeBallX);
    idParam.EyeBallY = idManager->GetId(ParamEyeBallY);
}

Live2DModel::~Live2DModel()
{

    renderBuffer.DestroyOffscreenFrame();

    ReleaseMotions();
    ReleaseExpressions();

    for (Csm::csmInt32 i = 0; i < modelSetting->GetMotionGroupCount(); i++)
    {
        const Csm::csmChar* group = modelSetting->GetMotionGroupName(i);
        ReleaseMotionGroup(group);
    }

    for (Csm::csmUint32 i = 0; i < bindTextureId.GetSize(); i++)
    {
        Live2DManager::GetInstance()->GetTextureManager()->ReleaseTexture(bindTextureId[i]);
    }
    bindTextureId.Clear();

    delete modelSetting;
}

void Live2DModel::LoadAssets(const Csm::csmChar* dir, const Csm::csmChar* fileName)
{
    using namespace Live2D::Cubism::Framework;

    modelHomeDir = dir;

    csmSizeInt size;
    const csmString path = csmString(dir) + fileName;

    csmByte* buffer = CreateBuffer(path.GetRawString(), &size);
    ICubismModelSetting* setting = new CubismModelSettingJson(buffer, size);
    DeleteBuffer(buffer, path.GetRawString());

    SetupModel(setting);

    CreateRenderer();

    SetupTextures();
}

void Live2DModel::ReloadRenderer()
{
    DeleteRenderer();

    CreateRenderer();

    SetupTextures();
}

void Live2DModel::Update()
{
    using namespace Live2D::Cubism::Framework;
    using namespace Live2D::Cubism::Framework::DefaultParameterId;

    const csmFloat32 deltaTimeSeconds = Live2DPal::GetDeltaTime();
    userTimeSeconds += deltaTimeSeconds;

    _dragManager->Update(deltaTimeSeconds);
    _dragX = _dragManager->GetX();
    _dragY = _dragManager->GetY();

    csmBool motionUpdated = false;

    //-----------------------------------------------------------------
    _model->LoadParameters();
    if (_motionManager->IsFinished())
    {
        using namespace Live2DDefine;
        // モーションの再生がない場合、待機モーションの中からランダムで再生する
        StartRandomMotion(MotionGroupIdle, PriorityIdle);
    }
    else
    {
        motionUpdated = _motionManager->UpdateMotion(_model, deltaTimeSeconds);
    }
    _model->SaveParameters();

    if (!motionUpdated)
    {
        if (_eyeBlink != NULL)
        {
            _eyeBlink->UpdateParameters(_model, deltaTimeSeconds);
        }
    }

    if (_expressionManager != NULL)
    {
        _expressionManager->UpdateMotion(_model, deltaTimeSeconds);
    }

    _model->AddParameterValue(idParam.AngleX, _dragX * 30);
    _model->AddParameterValue(idParam.AngleY, _dragY * 30);
    _model->AddParameterValue(idParam.AngleZ, _dragX * _dragY * -30);

    _model->AddParameterValue(idParam.BodyAngleX, _dragX * 10); 

    _model->AddParameterValue(idParam.EyeBallX, _dragX); 
    _model->AddParameterValue(idParam.EyeBallY, _dragY);

    if (_breath != NULL)
    {
        _breath->UpdateParameters(_model, deltaTimeSeconds);
    }

    if (_physics != NULL)
    {
        _physics->Evaluate(_model, deltaTimeSeconds);
    }

    if (_lipSync)
    {
        csmFloat32 value = 0; 

        for (csmUint32 i = 0; i < lipSyncIds.GetSize(); ++i)
        {
            _model->AddParameterValue(lipSyncIds[i], value, 0.8f);
        }
    }

    if (_pose != NULL)
    {
        _pose->UpdateParameters(_model, deltaTimeSeconds);
    }

    _model->Update();

}

void Live2DModel::Draw(Csm::CubismMatrix44& matrix)
{
    CubismRenderer_D3D* renderer = GetRenderer<CubismRenderer_D3D>();

    if (_model == NULL || deleteModel || renderer == NULL)
    {
        return;
    }
    matrix.MultiplyByMatrix(_modelMatrix);

    renderer->SetMvpMatrix(&matrix);

    DoDraw();
}

Csm::CubismMotionQueueEntryHandle Live2DModel::StartMotion(const Csm::csmChar* group, Csm::csmInt32 no, Csm::csmInt32 priority)
{
    using namespace Live2D::Cubism::Framework;
    using namespace Live2D::Cubism::Framework::DefaultParameterId;
    using namespace Live2DDefine;

    if (priority == PriorityForce)
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    const csmString fileName = modelSetting->GetMotionFileName(group, no);

    //ex) idle_0
    csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, no);
    CubismMotion* motion = static_cast<CubismMotion*>(motions[name.GetRawString()]);
    csmBool autoDelete = false;

    if (motion == NULL)
    {
        csmString path = fileName;
        path = modelHomeDir + path;

        csmByte* buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        motion = static_cast<CubismMotion*>(LoadMotion(buffer, size, NULL));
        csmFloat32 fadeTime = modelSetting->GetMotionFadeInTimeValue(group, no);
        if (fadeTime >= 0.0f)
        {
            motion->SetFadeInTime(fadeTime);
        }

        fadeTime = modelSetting->GetMotionFadeOutTimeValue(group, no);
        if (fadeTime >= 0.0f)
        {
            motion->SetFadeOutTime(fadeTime);
        }
        motion->SetEffectIds(eyeBlinkIds, lipSyncIds);
        autoDelete = true; 

        DeleteBuffer(buffer, path.GetRawString());
    }

    //voice
    csmString voice = modelSetting->GetMotionSoundFileName(group, no);
    if (strcmp(voice.GetRawString(), "") != 0)
    {
        csmString path = voice;
        path = modelHomeDir + path;
    }

    return  _motionManager->StartMotionPriority(motion, autoDelete, priority);
}

Csm::CubismMotionQueueEntryHandle Live2DModel::StartRandomMotion(const Csm::csmChar* group, Csm::csmInt32 priority)
{
    using namespace Live2D::Cubism::Framework;

    if (modelSetting->GetMotionCount(group) == 0)
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    csmInt32 no = rand() % modelSetting->GetMotionCount(group);

    return StartMotion(group, no, priority);
}

void Live2DModel::SetExpression(const Csm::csmChar* expressionID)
{
    using namespace Live2DDefine;

    Csm::ACubismMotion* motion = expressions[expressionID];

    if (motion != NULL)
    {
        _expressionManager->StartMotionPriority(motion, false, PriorityForce);
    }
}

void Live2DModel::SetRandomExpression()
{
    using namespace Live2D::Cubism::Framework;

    if (expressions.GetSize() == 0)
    {
        return;
    }

    csmInt32 no = rand() % expressions.GetSize();
    csmMap<csmString, ACubismMotion*>::const_iterator map_ite;
    csmInt32 i = 0;
    for (map_ite = expressions.Begin(); map_ite != expressions.End(); map_ite++)
    {
        if (i == no)
        {
            csmString name = (*map_ite).First;
            SetExpression(name.GetRawString());
            return;
        }
        i++;
    }
}

void Live2DModel::MotionEventFired(const Live2D::Cubism::Framework::csmString& eventValue)
{
    CubismLogInfo("%s is fired on LAppModel!!", eventValue.GetRawString());
}

Csm::csmBool Live2DModel::HitTest(const Csm::csmChar* hitAreaName, Csm::csmFloat32 x, Csm::csmFloat32 y)
{

    if (_opacity < 1)
    {
        return false;
    }
    const Csm::csmInt32 count = modelSetting->GetHitAreasCount();
    for (Csm::csmInt32 i = 0; i < count; i++)
    {
        if (strcmp(modelSetting->GetHitAreaName(i), hitAreaName) == 0)
        {
            const Csm::CubismIdHandle drawID = modelSetting->GetHitAreaId(i);
            return IsHit(drawID, x, y);
        }
    }
    return false; 
}

CubismOffscreenFrame_D3D& Live2DModel::GetRenderBuffer()
{
    return renderBuffer;
}

void Live2DModel::OnDeviceLost()
{
    renderBuffer.DestroyOffscreenFrame();

    DeleteRenderer();
}

void Live2DModel::DoDraw()
{
    if (_model == NULL)
    {
        return;
    }

    GetRenderer<CubismRenderer_D3D>()->DrawModel();
}

void Live2DModel::SetupModel(Csm::ICubismModelSetting* setting)
{
    using namespace Live2D::Cubism::Framework;
    using namespace Live2D::Cubism::Framework::DefaultParameterId;

    _updating = true;
    _initialized = false;

    modelSetting = setting;

    csmByte* buffer;
    csmSizeInt size;

    //Cubism Model
    if (strcmp(modelSetting->GetModelFileName(), "") != 0)
    {
        csmString path = modelSetting->GetModelFileName();
        path = modelHomeDir + path;

        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadModel(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    //Expression
    if (modelSetting->GetExpressionCount() > 0)
    {
        const csmInt32 count = modelSetting->GetExpressionCount();
        for (csmInt32 i = 0; i < count; i++)
        {
            csmString name = modelSetting->GetExpressionName(i);
            csmString path = modelSetting->GetExpressionFileName(i);
            path = modelHomeDir + path;

            buffer = CreateBuffer(path.GetRawString(), &size);
            ACubismMotion* motion = LoadExpression(buffer, size, name.GetRawString());

            if (expressions[name] != NULL)
            {
                ACubismMotion::Delete(expressions[name]);
                expressions[name] = NULL;
            }
            expressions[name] = motion;

            DeleteBuffer(buffer, path.GetRawString());
        }
    }

    //Physics
    if (strcmp(modelSetting->GetPhysicsFileName(), "") != 0)
    {
        csmString path = modelSetting->GetPhysicsFileName();
        path = modelHomeDir + path;

        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadPhysics(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    //Pose
    if (strcmp(modelSetting->GetPoseFileName(), "") != 0)
    {
        csmString path = modelSetting->GetPoseFileName();
        path = modelHomeDir + path;

        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadPose(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    //EyeBlink
    if (modelSetting->GetEyeBlinkParameterCount() > 0)
    {
        _eyeBlink = CubismEyeBlink::Create(modelSetting);
    }

    //Breath
    {
        _breath = CubismBreath::Create();

        csmVector<CubismBreath::BreathParameterData> breathParameters;

        breathParameters.PushBack(CubismBreath::BreathParameterData(idParam.AngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(idParam.AngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(idParam.AngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(idParam.BodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(CubismFramework::GetIdManager()->GetId(ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));

        _breath->SetParameters(breathParameters);
    }

    //UserData
    if (strcmp(modelSetting->GetUserDataFile(), "") != 0)
    {
        csmString path = modelSetting->GetUserDataFile();
        path = modelHomeDir + path;
        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadUserData(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // EyeBlinkIds
    {
        csmInt32 eyeBlinkIdCount = modelSetting->GetEyeBlinkParameterCount();
        for (csmInt32 i = 0; i < eyeBlinkIdCount; ++i)
        {
            eyeBlinkIds.PushBack(modelSetting->GetEyeBlinkParameterId(i));
        }
    }

    // LipSyncIds
    {
        csmInt32 lipSyncIdCount = modelSetting->GetLipSyncParameterCount();
        for (csmInt32 i = 0; i < lipSyncIdCount; ++i)
        {
            lipSyncIds.PushBack(modelSetting->GetLipSyncParameterId(i));
        }
    }

    //Layout
    csmMap<csmString, csmFloat32> layout;
    modelSetting->GetLayoutMap(layout);
    _modelMatrix->SetupFromLayout(layout);

    _model->SaveParameters();

    for (csmInt32 i = 0; i < modelSetting->GetMotionGroupCount(); i++)
    {
        const csmChar* group = modelSetting->GetMotionGroupName(i);
        PreloadMotionGroup(group);
    }

    _motionManager->StopAllMotions();

    _updating = false;
    _initialized = true;
}

void Live2DModel::SetupTextures()
{
    using namespace Live2D::Cubism::Framework;
#ifdef PREMULTIPLIED_ALPHA_ENABLE
    const bool isPreMult = true;
    const bool isTextureMult = false;
#else
    const bool isPreMult = false;
    const bool isTextureMult = false;
#endif

    bindTextureId.Clear();

    for (csmInt32 modelTextureNumber = 0; modelTextureNumber < modelSetting->GetTextureCount(); modelTextureNumber++)
    {
        if (strcmp(modelSetting->GetTextureFileName(modelTextureNumber), "") == 0)
        {
            continue;
        }

        csmString texturePath = modelSetting->GetTextureFileName(modelTextureNumber);
        texturePath = modelHomeDir + texturePath;

        Live2DTextureManager::TextureInfo* texture = Live2DManager::GetInstance()->GetTextureManager()->CreateTextureFromPngFile(texturePath.GetRawString(), isTextureMult,
            D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DX_FILTER_BOX | D3DX_FILTER_DITHER_DIFFUSION);

        // 
        if (texture)
        {
            const Csm::csmUint64 textureManageId = texture->id;

            IDirect3DTexture* texture = NULL;
            if (Live2DManager::GetInstance()->GetTextureManager()->GetTexture(textureManageId, texture))
            {
                GetRenderer<CubismRenderer_D3D>()->BindTexture(modelTextureNumber, texture);
                bindTextureId.PushBack(textureManageId);
            }
        }
    }

    GetRenderer<CubismRenderer_D3D>()->IsPremultipliedAlpha(isPreMult);
}

void Live2DModel::PreloadMotionGroup(const Csm::csmChar* group)
{
    using namespace Live2D::Cubism::Framework;

    const csmInt32 count = modelSetting->GetMotionCount(group);

    for (csmInt32 i = 0; i < count; i++)
    {
        //ex) idle_0
        csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, i);
        csmString path = modelSetting->GetMotionFileName(group, i);
        path = modelHomeDir + path;


        csmByte* buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        CubismMotion* tmpMotion = static_cast<CubismMotion*>(LoadMotion(buffer, size, name.GetRawString()));

        csmFloat32 fadeTime = modelSetting->GetMotionFadeInTimeValue(group, i);
        if (fadeTime >= 0.0f)
        {
            tmpMotion->SetFadeInTime(fadeTime);
        }

        fadeTime = modelSetting->GetMotionFadeOutTimeValue(group, i);
        if (fadeTime >= 0.0f)
        {
            tmpMotion->SetFadeOutTime(fadeTime);
        }
        tmpMotion->SetEffectIds(eyeBlinkIds, lipSyncIds);

        if (motions[name] != NULL)
        {
            ACubismMotion::Delete(motions[name]);
        }
        motions[name] = tmpMotion;

        DeleteBuffer(buffer, path.GetRawString());
    }
}

void Live2DModel::ReleaseMotionGroup(const Csm::csmChar* group) const
{
    using namespace Live2D::Cubism::Framework;

    const csmInt32 count = modelSetting->GetMotionCount(group);
    for (csmInt32 i = 0; i < count; i++)
    {
        csmString voice = modelSetting->GetMotionSoundFileName(group, i);
        if (strcmp(voice.GetRawString(), "") != 0)
        {
            csmString path = voice;
            path = modelHomeDir + path;
        }
    }
}

void Live2DModel::ReleaseMotions()
{
    using namespace Live2D::Cubism::Framework;

    for (csmMap<csmString, ACubismMotion*>::const_iterator iter = motions.Begin(); iter != motions.End(); ++iter)
    {
        ACubismMotion::Delete(iter->Second);
    }

    motions.Clear();
}

void Live2DModel::ReleaseExpressions()
{
    using namespace Live2D::Cubism::Framework;

    for (csmMap<csmString, ACubismMotion*>::const_iterator iter = expressions.Begin(); iter != expressions.End(); ++iter)
    {
        ACubismMotion::Delete(iter->Second);
    }

    expressions.Clear();
}
