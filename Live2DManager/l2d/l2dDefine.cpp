#include "l2dDefine.h"


namespace Live2DDefine {

    using namespace Csm;

    // ����
    const csmFloat32 ViewMaxScale = 2.0f;
    const csmFloat32 ViewMinScale = 0.8f;

    const csmFloat32 ViewLogicalLeft = -1.0f;
    const csmFloat32 ViewLogicalRight = 1.0f;

    const csmFloat32 ViewLogicalMaxLeft = -2.0f;
    const csmFloat32 ViewLogicalMaxRight = 2.0f;
    const csmFloat32 ViewLogicalMaxBottom = -2.0f;
    const csmFloat32 ViewLogicalMaxTop = 2.0f;


    // �ⲿ���x�ե�����(json)�ȺϤ碌��
    const csmChar* MotionGroupIdle = "Idle"; // �����ɥ��
    const csmChar* MotionGroupTapBody = "TapBody"; // ��򥿥åפ����Ȥ�

    // �ⲿ���x�ե�����(json)�ȺϤ碌��
    const csmChar* HitAreaNameHead = "Head";
    const csmChar* HitAreaNameBody = "Body";

    // ��`�����΃��ȶȶ���
    const csmInt32 PriorityNone = 0;
    const csmInt32 PriorityIdle = 1;
    const csmInt32 PriorityNormal = 2;
    const csmInt32 PriorityForce = 3;

    // �ǥХå��å��α�ʾ���ץ����
    const csmBool DebugLogEnable = true;
    const csmBool DebugTouchLogEnable = false;

    // Framework�������������Υ�٥��O��
    const CubismFramework::Option::LogLevel CubismLoggingLevel = CubismFramework::Option::LogLevel_Verbose;

}
