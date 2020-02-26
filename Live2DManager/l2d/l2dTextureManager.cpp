#include "l2dTextureManager.h"

Live2DTextureManager::TextureInfo* Live2DTextureManager::CreateTextureFromPngFile(std::string fileName, bool isPreMult, UINT width, UINT height, UINT mipLevel, DWORD filter)
{

	IDirect3DDevice* device = Live2DManager::GetD3dDevice();

	IDirect3DTexture* texture = NULL;
	Live2DTextureManager::TextureInfo* textureInfo = NULL;

	D3DXIMAGE_INFO resultInfo;

	// 実験としてここでα合成を行う場合はMipLevelを1にする 
	if (isPreMult)
	{
		mipLevel = 1;
	}

	// Lockする場合は立てる 
	DWORD usage = isPreMult ? D3DUSAGE_DYNAMIC : 0;

	if (SUCCEEDED(D3DXCreateTextureFromFileExA(device, fileName.c_str(), width, height,
		mipLevel,   // 「この値が 0 または D3DX_DEFAULT の場合は、完全なミップマップ チェーンが作成される。」 
		usage,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		filter,
		D3DX_DEFAULT,   // ミップフィルタ設定 「このパラメータに D3DX_DEFAULT を指定することは、D3DX_FILTER_BOX を指定することと等しい。」 
		0,              // カラーキー 
		&resultInfo,
		NULL,           // パレット 使用せず 
		&texture)))
	{
		// テクスチャ情報 
		textureInfo = new Live2DTextureManager::TextureInfo();

		if (textureInfo)
		{
			// 次のID 
			const Csm::csmUint64 addId = _sequenceId + 1;

			_textures.PushBack(texture);

			// 情報格納 
			textureInfo->fileName = fileName;
			textureInfo->width = static_cast<int>(resultInfo.Width);
			textureInfo->height = static_cast<int>(resultInfo.Height);
			textureInfo->id = addId;
			_texturesInfo.PushBack(textureInfo);

			_sequenceId = addId;

			if (isPreMult)
			{
				D3DLOCKED_RECT locked;
				HRESULT hr = texture->LockRect(0, &locked, NULL, 0);
				if (SUCCEEDED(hr))
				{
					for (unsigned int htLoop = 0; htLoop < resultInfo.Height; htLoop++)
					{
						unsigned char* pixel4 = reinterpret_cast<unsigned char*>(locked.pBits) + locked.Pitch * htLoop;
						unsigned int* pixel32 = reinterpret_cast<unsigned int*>(pixel4);

						for (int i = 0; i < locked.Pitch; i += 4)
						{
							unsigned int val = Premultiply(pixel4[i + 0], pixel4[i + 1], pixel4[i + 2], pixel4[i + 3]);
							pixel32[(i >> 2)] = val;
						}
					}

					texture->UnlockRect(0);
				}
			}

			return textureInfo;
		}
	}


	delete textureInfo;
	if (texture)
	{
		texture->Release();
		texture = NULL;
	}

	return NULL;
}
