#pragma once

#include "l2dd3d.h"
#include "l2dPal.h"
#include "l2dDefine.h"
#include "l2dManager.h"

#include <CubismFramework.hpp>
#include <Model/CubismUserModel.hpp>
#include <ICubismModelSetting.hpp>
#include <Type/csmRectF.hpp>
#include <fstream>
#include <vector>
#include <CubismModelSettingJson.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <CubismDefaultParameterId.hpp>
#include <Utils/CubismString.hpp>
#include <Id/CubismIdManager.hpp>
#include <Motion/CubismMotionQueueEntry.hpp>
#include <CubismDefaultParameterId.hpp>


class Live2DModel : public Csm::CubismUserModel
{
public:
    Live2DModel();

    ~Live2DModel();


    void LoadAssets(const Csm::csmChar* dir, const Csm::csmChar* fileName);


    void ReloadRenderer();


    void Update();


    void Draw(Csm::CubismMatrix44& matrix);


    Csm::CubismMotionQueueEntryHandle StartMotion(const Csm::csmChar* group, Csm::csmInt32 no, Csm::csmInt32 priority);


    Csm::CubismMotionQueueEntryHandle StartRandomMotion(const Csm::csmChar* group, Csm::csmInt32 priority);


    void SetExpression(const Csm::csmChar* expressionID);


    void SetRandomExpression();


    virtual void MotionEventFired(const Live2D::Cubism::Framework::csmString& eventValue);

    virtual Csm::csmBool HitTest(const Csm::csmChar* hitAreaName, Csm::csmFloat32 x, Csm::csmFloat32 y);

    void DeleteMark() { deleteModel = true; }

    CubismOffscreenFrame_D3D& GetRenderBuffer();

    void OnDeviceLost();

protected:
    void DoDraw();

private:
    void SetupModel(Csm::ICubismModelSetting* setting);

    void SetupTextures();

    void PreloadMotionGroup(const Csm::csmChar* group);

    void ReleaseMotionGroup(const Csm::csmChar* group) const;

    void ReleaseMotions();

    void ReleaseExpressions();

    Csm::csmByte* CreateBuffer(const Csm::csmChar* path, Csm::csmSizeInt* size)
    {
        return Live2DPal::LoadFileAsBytes(path, size);
    }

    void DeleteBuffer(Csm::csmByte* buffer, const Csm::csmChar* path = "")
    {
        Live2DPal::ReleaseBytes(buffer);
    }
private:

    Csm::ICubismModelSetting* modelSetting; ///< モデルセッティング情報
    Csm::csmString modelHomeDir; ///< モデルセッティングが置かれたディレクトリ
    Csm::csmFloat32 userTimeSeconds; ///< デルタ時間の積算値[秒]
    Csm::csmVector<Csm::CubismIdHandle> eyeBlinkIds; ///<　モデルに設定されたまばたき機能用パラメータID
    Csm::csmVector<Csm::CubismIdHandle> lipSyncIds; ///< モデルに設定されたリップシンク機能用パラメータID
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion*>   motions; ///< 読み込まれているモーションのリスト
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion*>   expressions; ///< 読み込まれている表情のリスト
    Csm::csmVector<Csm::csmRectF> hitArea;
    Csm::csmVector<Csm::csmRectF> userArea;
    struct IdParam
    {
        const Csm::CubismId* AngleX; ///< パラメータID: ParamAngleX
        const Csm::CubismId* AngleY; ///< パラメータID: ParamAngleX
        const Csm::CubismId* AngleZ; ///< パラメータID: ParamAngleX
        const Csm::CubismId* BodyAngleX; ///< パラメータID: ParamBodyAngleX
        const Csm::CubismId* EyeBallX; ///< パラメータID: ParamEyeBallX
        const Csm::CubismId* EyeBallY; ///< パラメータID: ParamEyeBallXY
    } idParam;

    Csm::csmVector<Csm::csmUint64> bindTextureId; ///< テクスチャID 

    bool deleteModel;  ///< 実体消滅予定フラグ Drawを呼ばない 

    CubismOffscreenFrame_D3D   renderBuffer;  ///< フレームバッファ以外の描画先 
};
