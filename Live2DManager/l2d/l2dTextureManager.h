#pragma once

#include "l2dd3d.h"
#include "l2dManager.h"

#include <string>

#include <Rendering/D3D9/CubismNativeInclude_D3D9.hpp>
#include <Type/CubismBasicType.hpp>
#include <Type/csmVector.hpp>

class Live2DTextureManager
{
public:

    struct TextureInfo
    {
        Csm::csmUint64 id = 0;      ///< �ƥ�������ID 
        int width = 0;              ///< ���
        int height = 0;             ///< �ߤ�
        std::string fileName;       ///< �ե�������
    };

    Live2DTextureManager()
    {
        _sequenceId = 0;
    }

    ~Live2DTextureManager()
    {
        ReleaseTextures();
    }

    inline unsigned int Premultiply(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
    {
        return static_cast<unsigned>(\
            (red * (alpha + 1) >> 8) | \
            ((green * (alpha + 1) >> 8) << 8) | \
            ((blue * (alpha + 1) >> 8) << 16) | \
            (((alpha)) << 24)   \
            );
    }

    TextureInfo* CreateTextureFromPngFile(std::string fileName, bool isPreMult, UINT width, UINT height, UINT mipLevel, DWORD filter);

    void ReleaseTextures()
    {
        for (Csm::csmUint32 i = 0; i < _texturesInfo.GetSize(); i++)
        {
            // info��ȥ 
            delete _texturesInfo[i];

            // �g���ȥ 
            if (_textures[i])
            {
                _textures[i]->Release();
                _textures[i] = NULL;
            }
        }

        _texturesInfo.Clear();
        _textures.Clear();
    }

    void ReleaseTexture(Csm::csmUint64 textureId)
    {
        for (Csm::csmUint32 i = 0; i < _texturesInfo.GetSize(); i++)
        {
            if (_texturesInfo[i]->id != textureId)
            {
                continue;
            }
            // IDһ�� 

            // info��ȥ 
            delete _texturesInfo[i];
            _texturesInfo.Remove(i);

            // �g���ȥ 
            // getBaseAddress��Framework::loadTextureFromGnf���_������������ȡ��� 
            if (_textures[i])
            {
                _textures[i]->Release();
                _textures[i] = NULL;
            }
            _textures.Remove(i);

            break;
        }

        if (_textures.GetSize() == 0)
        {
            _textures.Clear();
        }
        if (_texturesInfo.GetSize() == 0)
        {
            _texturesInfo.Clear();
        }
    }
    
    void ReleaseTexture(std::string fileName)
    {
        for (Csm::csmUint32 i = 0; i < _texturesInfo.GetSize(); i++)
        {
            if (_texturesInfo[i]->fileName == fileName)
            {
                // info��ȥ 
                delete _texturesInfo[i];
                _texturesInfo.Remove(i);

                // �g���ȥ 
                if (_textures[i])
                {
                    _textures[i]->Release();
                    _textures[i] = NULL;
                }
                _textures.Remove(i);

                break;
            }
        }

        if (_textures.GetSize() == 0)
        {
            _textures.Clear();
        }
        if (_texturesInfo.GetSize() == 0)
        {
            _texturesInfo.Clear();
        }
    }

    bool GetTexture(Csm::csmUint64 textureId, IDirect3DTexture*& retTexture) const
    {
        retTexture = NULL;
        for (Csm::csmUint32 i = 0; i < _texturesInfo.GetSize(); i++)
        {
            if (_texturesInfo[i]->id == textureId)
            {
                retTexture = _textures[i];
                return true;
            }
        }

        return false;
    }

    TextureInfo* GetTextureInfoByName(std::string& fileName) const
    {
        for (Csm::csmUint32 i = 0; i < _texturesInfo.GetSize(); i++)
        {
            if (_texturesInfo[i]->fileName == fileName)
            {
                return _texturesInfo[i];
            }
        }

        return NULL;
    }

private:

    Csm::csmVector<IDirect3DTexture*>  _textures;      ///< DX9�ƥ������� 
    Csm::csmVector<TextureInfo*> _texturesInfo;         ///< �ƥ���������� 

    Csm::csmUint64   _sequenceId;    ///< ���뤹�뤿���ͨ��ID 
};
