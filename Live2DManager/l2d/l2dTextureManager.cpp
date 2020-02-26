#include "l2dTextureManager.h"

Live2DTextureManager::TextureInfo* Live2DTextureManager::CreateTextureFromPngFile(std::string fileName, bool isPreMult, UINT width, UINT height, UINT mipLevel, DWORD filter)
{

	IDirect3DDevice* device = Live2DManager::GetD3dDevice();

	IDirect3DTexture* texture = NULL;
	Live2DTextureManager::TextureInfo* textureInfo = NULL;

	D3DXIMAGE_INFO resultInfo;

	// �g�Y�Ȥ��Ƥ����Ǧ��ϳɤ��Ф����Ϥ�MipLevel��1�ˤ��� 
	if (isPreMult)
	{
		mipLevel = 1;
	}

	// Lock������Ϥ����Ƥ� 
	DWORD usage = isPreMult ? D3DUSAGE_DYNAMIC : 0;

	if (SUCCEEDED(D3DXCreateTextureFromFileExA(device, fileName.c_str(), width, height,
		mipLevel,   // �����΂��� 0 �ޤ��� D3DX_DEFAULT �Έ��Ϥϡ���ȫ�ʥߥåץޥå� �����`�����ɤ���롣�� 
		usage,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		filter,
		D3DX_DEFAULT,   // �ߥåץե��륿�O�� �����Υѥ��`���� D3DX_DEFAULT ��ָ�����뤳�Ȥϡ�D3DX_FILTER_BOX ��ָ�����뤳�ȤȵȤ������� 
		0,              // ����`���` 
		&resultInfo,
		NULL,           // �ѥ�å� ʹ�ä��� 
		&texture)))
	{
		// �ƥ���������� 
		textureInfo = new Live2DTextureManager::TextureInfo();

		if (textureInfo)
		{
			// �Τ�ID 
			const Csm::csmUint64 addId = _sequenceId + 1;

			_textures.PushBack(texture);

			// ����{ 
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
