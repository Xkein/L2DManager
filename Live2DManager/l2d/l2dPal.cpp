#include"l2dPal.h"

double Live2DPal::s_deltaTime = 0.0;
double Live2DPal::deltaMarkedTime = 0.0;
LARGE_INTEGER Live2DPal::s_frequency{};
LARGE_INTEGER Live2DPal::s_lastFrame{};

LARGE_INTEGER Live2DPal::markedFrames[2]{};
