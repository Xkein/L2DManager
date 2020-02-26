#pragma once

#include <Windows.h>
#include <string>
#include <CubismFramework.hpp>
#include <Type/CubismBasicType.hpp>
#include <iostream>
#include <fstream>

class Live2DPal
{
private:
    static LARGE_INTEGER s_frequency;
    static LARGE_INTEGER s_lastFrame;
    static LARGE_INTEGER markedFrames[2];
    static double s_deltaTime;
    static double deltaMarkedTime;

public:

    static void StartTimer()
    {
        QueryPerformanceFrequency(&s_frequency);
    }

    static Csm::csmByte* LoadFileAsBytes(const std::string filePath, Csm::csmSizeInt* outSize)
    {
        //filePath;// 
        const char* path = filePath.c_str();

        int size = 0;
        struct stat statBuf;
        if (stat(path, &statBuf) == 0)
        {
            size = statBuf.st_size;
        }

        std::fstream file;
        char* buf = new char[size];

        file.open(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return NULL;
        }
        file.read(buf, size);
        file.close();

        *outSize = size;
        return reinterpret_cast<Csm::csmByte*>(buf);
    }

    static void ReleaseBytes(Csm::csmByte* byteData)
    {
        delete[] byteData;
    }

    static Csm::csmFloat32 GetDeltaTime()
    {
        return static_cast<Csm::csmFloat32>(s_deltaTime);
    }

    static void UpdateTime()
    {
        if (s_frequency.QuadPart == 0)
        {
            StartTimer();
            QueryPerformanceCounter(&s_lastFrame);
            s_deltaTime = 0.0f;
            return;
        }

        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);

        const LONGLONG BASIS = 1000000;
        LONGLONG dwTime = ((current.QuadPart - s_lastFrame.QuadPart) * (LONGLONG)BASIS / s_frequency.QuadPart);
        s_deltaTime = (double)dwTime / (double)(BASIS);
        s_lastFrame = current;
    }

    static bool MarkTimeIfElapsed(double time)
    {
        if (markedFrames[0].QuadPart == 0) {
            QueryPerformanceCounter(&markedFrames[0]);
            return true;
        }
        QueryPerformanceCounter(&markedFrames[1]);

        constexpr LONGLONG BASIS = 1000000;
        LONGLONG dwTime = ((markedFrames[1].QuadPart - markedFrames[0].QuadPart) * (LONGLONG)BASIS / s_frequency.QuadPart);
        deltaMarkedTime = (double)dwTime / (double)(BASIS);

        if (deltaMarkedTime >= time) {
            markedFrames[0] = markedFrames[1];
            return true; 
        }
        return false;
    }

    static double GetDeltaMarkedTime()
    {
        return deltaMarkedTime;
    }

    static void CoordinateFullScreenToWindow(float clientWidth, float clientHeight, float fullScreenX, float fullScreenY, float& retWindowX, float& retWindowY)
    {
        retWindowX = retWindowY = 0.0f;

        const float width = static_cast<float>(clientWidth);
        const float height = static_cast<float>(clientHeight);

        if (width == 0.0f || height == 0.0f) return;

        retWindowX = (fullScreenX + width) * 0.5f;
        retWindowY = (-fullScreenY + height) * 0.5f;
    }

    static void CoordinateWindowToFullScreen(float clientWidth, float clientHeight, float windowX, float windowY, float& retFullScreenX, float& retFullScreenY)
    {
        retFullScreenX = retFullScreenY = 0.0f;

        const float width = static_cast<float>(clientWidth);
        const float height = static_cast<float>(clientHeight);

        if (width == 0.0f || height == 0.0f) return;

        retFullScreenX = 2.0f * windowX - width;
        retFullScreenY = (2.0f * windowY - height) * -1.0f;
    }
};
