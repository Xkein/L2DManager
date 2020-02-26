#pragma once

#include <math.h>

class TouchManager
{
public:

    TouchManager()
        : _startY(0.0f)
        , _startX(0.0f)
        , _lastX(0.0f)
        , _lastY(0.0f)
        , _lastX1(0.0f)
        , _lastY1(0.0f)
        , _lastX2(0.0f)
        , _lastY2(0.0f)
        , _lastTouchDistance(0.0f)
        , _deltaX(0.0f)
        , _deltaY(0.0f)
        , _scale(1.0f)
        , _touchSingle(false)
        , _flipAvailable(false)
    { }

    float GetCenterX() const { return _lastX; }
    float GetCenterY() const { return _lastY; }
    float GetDeltaX() const { return _deltaX; }
    float GetDeltaY() const { return _deltaY; }
    float GetStartX() const { return _startX; }
    float GetStartY() const { return _startY; }
    float GetScale() const { return _scale; }
    float GetX() const { return _lastX; }
    float GetY() const { return _lastY; }
    float GetX1() const { return _lastX1; }
    float GetY1() const { return _lastY1; }
    float GetX2() const { return _lastX2; }
    float GetY2() const { return _lastY2; }
    bool IsSingleTouch() const { return _touchSingle; }
    bool IsFlickAvailable() const { return _flipAvailable; }
    void DisableFlick() { _flipAvailable = false; }

    /*
    * @brief ���å��_ʼ�r���٥��
    *
    * @param[in] deviceY    ���å����������y�΂�
    * @param[in] deviceX    ���å����������x�΂�
    */
    void TouchesBegan(float deviceX, float deviceY)
    {
        _lastX = deviceX;
        _lastY = deviceY;
        _startX = deviceX;
        _startY = deviceY;
        _lastTouchDistance = -1.0f;
        _flipAvailable = true;
        _touchSingle = true;
    }

    /*
    * @brief �ɥ�å��r�Υ��٥��
    *
    * @param[in] deviceX    ���å����������y�΂�
    * @param[in] deviceY    ���å����������x�΂�
    */
    void TouchesMoved(float deviceX, float deviceY)
    {
        _lastX = deviceX;
        _lastY = deviceY;
        _lastTouchDistance = -1.0f;
        _touchSingle = true;
    }

    /*
    * @brief �ɥ�å��r�Υ��٥��
    *
    * @param[in] deviceX1   1�Ĥ�Υ��å����������x�΂�
    * @param[in] deviceY1   1�Ĥ�Υ��å����������y�΂�
    * @param[in] deviceX2   2�Ĥ�Υ��å����������x�΂�
    * @param[in] deviceY2   2�Ĥ�Υ��å����������y�΂�
    */
    void TouchesMoved(float deviceX1, float deviceY1, float deviceX2, float deviceY2)
    {
        float distance = CalculateDistance(deviceX1, deviceY1, deviceX2, deviceY2);
        float centerX = (deviceX1 + deviceX2) * 0.5f;
        float centerY = (deviceY1 + deviceY2) * 0.5f;

        if (_lastTouchDistance > 0.0f)
        {
            _scale = powf(distance / _lastTouchDistance, 0.75f);
            _deltaX = CalculateMovingAmount(deviceX1 - _lastX1, deviceX2 - _lastX2);
            _deltaY = CalculateMovingAmount(deviceY1 - _lastY1, deviceY2 - _lastY2);
        }
        else
        {
            _scale = 1.0f;
            _deltaX = 0.0f;
            _deltaY = 0.0f;
        }

        _lastX = centerX;
        _lastY = centerY;
        _lastX1 = deviceX1;
        _lastY1 = deviceY1;
        _lastX2 = deviceX2;
        _lastY2 = deviceY2;
        _lastTouchDistance = distance;
        _touchSingle = false;
    }

    /*
    * @brief �ե�å��ξ��x�y��
    *
    * @return �ե�å����x
    */
    float GetFlickDistance() const
    {
        return CalculateDistance(_startX, _startY, _lastX, _lastY);
    }

private:
    /*
    * @brief ��1�����2�ؤξ��x������
    *
    * @param[in] x1 1�Ĥ�Υ��å����������x�΂�
    * @param[in] y1 1�Ĥ�Υ��å����������y�΂�
    * @param[in] x2 2�Ĥ�Υ��å����������x�΂�
    * @param[in] y2 2�Ĥ�Υ��å����������y�΂�
    * @return   2��ξ��x
    */
    float CalculateDistance(float x1, float y1, float x2, float y2) const {
        return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    /*
    * ���Ĥ΂����顢�Ƅ��������롣
    * �`������Έ��Ϥ��Ƅ�������ͬ������Έ��Ϥϡ��~������С�������΂�����դ���
    *
    * @param[in] v1    1�Ĥ���Ƅ���
    * @param[in] v2    2�Ĥ���Ƅ���
    *
    * @return   С���������Ƅ���
    */
    float CalculateMovingAmount(float v1, float v2)
    {
        if ((v1 > 0.0f) != (v2 > 0.0f))
        {
            return 0.0f;
        }

        float sign = v1 > 0.0f ? 1.0f : -1.0f;
        float absoluteValue1 = fabsf(v1);
        float absoluteValue2 = fabsf(v2);
        return sign * ((absoluteValue1 < absoluteValue2) ? absoluteValue1 : absoluteValue2);
    }


    float _startY;              // ���å����_ʼ�����r��x�΂�
    float _startX;              // ���å����_ʼ�����r��y�΂�
    float _lastX;               // ���󥰥륿�å��r��x�΂�
    float _lastY;               // ���󥰥륿�å��r��y�΂�
    float _lastX1;              // ���֥륿�å��r��һ��Ŀ��x�΂�
    float _lastY1;              // ���֥륿�å��r��һ��Ŀ��y�΂�
    float _lastX2;              // ���֥륿�å��r�ζ���Ŀ��x�΂�
    float _lastY2;              // ���֥륿�å��r�ζ���Ŀ��y�΂�
    float _lastTouchDistance;   // 2�����Ϥǥ��å������Ȥ���ָ�ξ��x
    float _deltaX;              // ǰ�ؤ΂������ؤ΂��ؤ�x���ƄӾ��x��
    float _deltaY;              // ǰ�ؤ΂������ؤ΂��ؤ�y���ƄӾ��x��
    float _scale;               // ���Υե�`��ǒ줱�Ϥ碌�뒈���ʡ���������������1��
    bool _touchSingle;          // ���󥰥륿�å��r��true
    bool _flipAvailable;        // �ե�åפ��Є����ɤ���

};