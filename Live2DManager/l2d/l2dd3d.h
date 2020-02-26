#pragma once

#if defined(D3D9)
#include <d3d9.h>
#include <Rendering\D3D9\CubismNativeInclude_D3D9.hpp>
#include <Rendering\D3D9\CubismRenderer_D3D9.hpp>

using IDirect3D = IDirect3D9;
using IDirect3DTexture = IDirect3DTexture9;
using IDirect3DDevice = IDirect3DDevice9;
using CubismRenderer_D3D = Live2D::Cubism::Framework::Rendering::CubismRenderer_D3D9;
using CubismOffscreenFrame_D3D = Csm::Rendering::CubismOffscreenFrame_D3D9;
#endif // D3D9
