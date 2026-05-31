#pragma once
#include <unordered_map>
#include <glad/glad.h>

struct UV
{
	float u0, v0, u1, v1; // левый нижний и правый верхний
};

class TextureManager
{
public:

	static void Init() {}
	static void Shutdown()
	{
		for (auto& [ID, TexID] : _textures)
			glDeleteTextures(1, &TexID);
		for (auto& [ID, atlas] : _atlases)
			delete atlas;
	}

	static unsigned int LoadTexture(uint16_t height, uint16_t width, const unsigned char* data, GLint format = GL_RGB)
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		_textures[_lastFreeIndex] = texture;
		return _lastFreeIndex++;
	}
	static void SetTexture(unsigned int texID, uint16_t height, uint16_t width, 
		const void* pixels, GLint fromat, int offsetx = 0, int offsety = 0)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetx, offsety, width, height, fromat, GL_UNSIGNED_BYTE, pixels);
	}
	static inline GLuint GetTexture(GLuint texID)
	{
		return _textures[texID];
	}

	static unsigned int AddAtlas(GLint format, uint16_t height = 4096, uint16_t width = 4096)
	{
		Atlas* atlas = new Atlas(format, height, width);
		_atlases[_lastFreeIndex] = atlas;
		_textures[_lastFreeIndex] = atlas->_atlasID;
		return _lastFreeIndex++;
	}

	static void BindTexture(unsigned int ID, int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, _textures[ID]);
	}

	static UV AddTextureToAtlas(unsigned int atlasID, uint16_t height, uint16_t width, const unsigned char* data)
	{
		Atlas* atlas = _atlases[atlasID];
		if (!atlas->SetOnPossible(height, width))
			return { 0.0f, 0.0f, 0.0f, 0.0f };
		return atlas->AddImage(height, width, data);
	}
	static UV AddTextureToAtlas(unsigned int atlasID, uint16_t height, uint16_t width, const unsigned char* data, float offsetx)
	{
		Atlas* atlas = _atlases[atlasID];
		if (!atlas->SetOnPossible(height, width))
			return { 0.0f, 0.0f, 0.0f, 0.0f };
		UV uv = atlas->AddImage(height, width, data);
		atlas->_cursorX += offsetx;
		return uv;
	}
	static UV AddTextureToAtlas(unsigned int atlasID, uint16_t height, uint16_t width, const unsigned char* data, glm::vec2 offset)
	{
		Atlas* atlas = _atlases[atlasID];
		if (!atlas->SetOnPossible(height, width))
			return {0.0f, 0.0f, 0.0f, 0.0f};
		UV uv = atlas->AddImage(height, width, data);
		atlas->_cursorX += offset.x;
		atlas->_cursorY += offset.y;
		return uv;
	}

private:

	class Atlas
	{
		Atlas(GLint fromat, uint16_t height = 4096, uint16_t width = 4096)
			: _dataFormat(fromat), _atlasH(height), _atlasW(width)
		{
			glGenTextures(1, &_atlasID);
			glBindTexture(GL_TEXTURE_2D, _atlasID);
			glTexImage2D(GL_TEXTURE_2D, 0, _dataFormat, _atlasW, _atlasH, 0, _dataFormat, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		bool SetOnPossible(uint16_t h, uint16_t w)
		{
			// начало - низ право
			// если слево занято
			if (_cursorX + w > _atlasW)
			{
				_cursorY = _topBorder;
				_cursorX = 0;
			}
			// есть ли место снизу ?
			if (_atlasH - _cursorY < h)
				return false;
			// записываем topBorder
			if (_cursorY + h > _topBorder)
				_topBorder = _cursorY + h;
			return true;
		}
		UV AddImage(GLsizei height, GLsizei width, const void* pixels)
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, _cursorX, _cursorY, width, height, _dataFormat, GL_UNSIGNED_BYTE, pixels);
			_cursorX += width;
			return {
				float(_cursorX) / _atlasW,			// u0
				float(_cursorY) / _atlasH,			// v0
				float(_cursorX - width) / _atlasW,	// u1
				float(_cursorY + height) / _atlasH	// v1
			};
		}
	private:
		GLint _dataFormat;
		GLuint _atlasID;
		uint16_t _atlasH, _atlasW;
		uint16_t _cursorX = 0, _cursorY = 0;
		uint16_t _topBorder = 0;
		friend TextureManager;
	};

	static inline unsigned int _lastFreeIndex = 1;
	//                 texID  OpenGL_ID
	static inline std::unordered_map<unsigned int, GLuint> _textures;
	static inline std::unordered_map<unsigned int, Atlas*> _atlases;
};