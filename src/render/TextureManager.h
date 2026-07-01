#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION // макрос для включения функций stb_image
#include "../helpers/stb_image.h"

struct UV
{
	float u0, v0, u1, v1; // левый нижний и правый верхний
};

struct TextureData
{
	GLuint handle;
	GLenum type;
};

class TextureManager
{
public:

	class Atlas
	{
	public:
		Atlas(GLint fromat, uint16_t height = 4096, uint16_t width = 4096)
			: _dataFormat(fromat), _atlasH(height), _atlasW(width)
		{
			glGenTextures(1, &_atlasID);
			glBindTexture(GL_TEXTURE_2D, _atlasID);
			glTexImage2D(GL_TEXTURE_2D, 0, _dataFormat, _atlasW, _atlasH, 0, _dataFormat, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		UV AddImage(GLsizei height, GLsizei width, const void* pixels)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

		GLint _dataFormat;
		GLuint _atlasID;
		uint16_t _atlasH, _atlasW;
		uint16_t _cursorX = 0, _cursorY = 0;
		uint16_t _topBorder = 0;
		friend TextureManager;
	};
	class AtlasArray
	{
	public:
		AtlasArray(GLint fromat = GL_RED, uint16_t height = 1024, uint16_t width = 1024, uint16_t layers = 1)
			: _dataFormat(fromat), _atlasH(height), _atlasW(width), _layers(layers)
		{
			glGenTextures(1, &_atlasArrayID);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _atlasArrayID);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, _dataFormat, _atlasW, _atlasH, layers, 0, _dataFormat, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		void AddImage(uint16_t height, uint16_t width, uint16_t layer, const void* pixels, UV& uv)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, _atlasArrayID);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
				_cursorX, _cursorY, layer,
				width, height, 1, _dataFormat, GL_UNSIGNED_BYTE, pixels);
			_cursorX += width;
			uv.u0 = float(_cursorX) / _atlasW;			// u0
			uv.v0 = float(_cursorY) / _atlasH;			// v0
			uv.u1 = float(_cursorX - width) / _atlasW;	// u1
			uv.v1 = float(_cursorY + height) / _atlasH;	// v1
		}
		void GrowAtlasArray()
		{
			GLuint oldArray = _atlasArrayID;
			glGenTextures(1, &_atlasArrayID);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _atlasArrayID);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, _dataFormat, _atlasW, _atlasH, _layers + 1, 0, _dataFormat, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glCopyImageSubData(
				oldArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
				_atlasArrayID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
				_atlasW, _atlasH, _layers++);

			_textures[GetAtlasArrayID(this)].handle = _atlasArrayID;

			glDeleteTextures(1, &oldArray);
		}
		void SetOnZero() { _cursorX = 0, _cursorY = 0, _topBorder = 0; }
		inline uint16_t GetLayers() { return _layers; }
	private:
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

		GLint _dataFormat;
		GLuint _atlasArrayID;
		uint16_t _atlasH, _atlasW, _layers;
		uint16_t _topBorder = 0;
		uint16_t _cursorX = 0, _cursorY = 0;
		friend TextureManager;
	};
	
	static void Init() {}
	static void Shutdown()
	{
		for (auto& [ID, TexID] : _textures)
			glDeleteTextures(1, &TexID.handle);
		for (auto& [ID, atlas] : _atlases)
			delete atlas;
	}
	static unsigned int GetMaxSizeOfTexture(GLint& maxTextureSize)
	{
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	}
	static unsigned int CreateTexture(uint16_t height, uint16_t width, const unsigned char* data, GLint format = GL_RGB)
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		_textures[_lastFreeIndex] = { texture, GL_TEXTURE_2D };
		return _lastFreeIndex++;
	}
	static unsigned int LoadTexture(char const* filename, GLenum format = GL_RGBA,
		GLuint filter = GL_LINEAR, GLuint wrap = GL_REPEAT)
	{
		stbi_set_flip_vertically_on_load(true);	// настройка stb_image (переворачиваем Y, чтобы соответствовать OpenGL)

		int width, height, channels; // переменные параметров изображения
		unsigned char* data = stbi_load(filename, &width, &height, &channels, 0); // загружаем все данные изображения

		// проверка коректной загрузки
		if (!data) {
			std::cout << "Failed to loadt texture!" << std::endl;
			return 0;
		}
		GLuint texture;
		//GLuint texture; // создаем идентификатор текстуры
		glGenTextures(1, &texture); // говорим создать Textures в количестве 1 и положить (id) идентификатор в VAO
		glBindTexture(GL_TEXTURE_2D, texture); // берет объект texture в контекст редактирования с типом привязки GL_TEXTURE_2D -<(тип привязки так же важен в типе переменной шейдера)
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, format == GL_RGBA ? 4 : 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); // GL_TEXTURE_MIN_FILTER - поведение при уменьшении текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); // GL_TEXTURE_MAG_FILTER - поведение при увеличении текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap); // GL_TEXTURE_WRAP_S - по горизонтали (U координата) X
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap); // GL_TEXTURE_WRAP_T - по вертикали (V координата) Y
		// GL_REPEAT — текстура просто циклически повторяется.
		// GL_CLAMP_TO_EDGE — крайние пиксели текстуры растягиваются до бесконечности.
		// GL_MIRRORED_REPEAT — текстура повторяется, но каждый раз зеркально отражается.
		// GL_CLAMP_TO_BORDER — за пределами текстуры выводится сплошной цвет(цвет задается отдельно через glTexParameterfv).
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // загружаем данные в GPU
		stbi_image_free(data); // удаляем данные из оператики после загрузки на GPU
		_textures[_lastFreeIndex] = { texture, GL_TEXTURE_2D };
		return _lastFreeIndex++;
	}
	static void SetTexture(unsigned int texID, uint16_t height, uint16_t width, 
		const void* pixels, GLint fromat, int offsetx = 0, int offsety = 0)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetx, offsety, width, height, fromat, GL_UNSIGNED_BYTE, pixels);
	}
	static inline unsigned int GetTexture(GLuint texID)
	{
		return _textures[texID].handle;
	}
	static Atlas* GetAtlas(unsigned int atlasID) { return _atlases[atlasID]; }
	static AtlasArray* GetAtlasArray(unsigned int atlasArrayID) { return _atlasArrays[atlasArrayID]; }
	static unsigned int CreateAtlas(GLint format, uint16_t height = 4096, uint16_t width = 4096)
	{
		Atlas* atlas = new Atlas(format, height, width);
		_atlases[_lastFreeIndex] = atlas;
		_textures[_lastFreeIndex] = { atlas->_atlasID, GL_TEXTURE_2D };
		return _lastFreeIndex++;
	}
	static void BindTexture(unsigned int ID, int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		TextureData td = _textures[ID];
		glBindTexture(td.type, td.handle);
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

	static unsigned int CreateAtlasArray(GLint format, uint16_t height = 4096, uint16_t width = 4096, uint16_t layers = 4)
	{
		AtlasArray* atlasArray = new AtlasArray(format, height, width, layers);
		_atlasArrays[_lastFreeIndex] = atlasArray;
		_textures[_lastFreeIndex] = { atlasArray->_atlasArrayID, GL_TEXTURE_2D_ARRAY };
		return _lastFreeIndex++;
	}
	static bool AddTextureToAtlasArray(unsigned int atlasID,
		uint16_t height, uint16_t width, uint16_t layer, const unsigned char* data, UV& uv)
	{
		AtlasArray* atlasArray = _atlasArrays[atlasID];
		if (!atlasArray->SetOnPossible(height, width))
			return false;
		atlasArray->AddImage(height, width, layer, data, uv);
		return true;
	}
	static bool AddTextureToEndAtlasArray(unsigned int atlasID,
		uint16_t height, uint16_t width, const unsigned char* data, UV& uv)
	{
		AtlasArray* atlasArray = _atlasArrays[atlasID];
		if (!atlasArray->SetOnPossible(height, width))
			return false;
		atlasArray->AddImage(height, width, atlasArray->_layers-1, data, uv);
		return true;
	}
	static bool AddTextureToAtlasArray(unsigned int atlasID, uint16_t height, uint16_t width, uint16_t layer,
		const unsigned char* data, UV& uv, float offsetx)
	{
		AtlasArray* atlasArray = _atlasArrays[atlasID];
		if (!atlasArray->SetOnPossible(height, width))
			return false;
		atlasArray->AddImage(height, width, layer, data, uv);
		atlasArray->_cursorX += offsetx;
		return true;
	}
	static bool AddTextureToAtlasArray(unsigned int atlasID, uint16_t height, uint16_t width, uint16_t layer,
		const unsigned char* data, UV& uv, glm::vec2 offset)
	{
		AtlasArray* atlasArray = _atlasArrays[atlasID];
		if (!atlasArray->SetOnPossible(height, width))
			return false;
		atlasArray->AddImage(height, width, layer, data, uv);
		atlasArray->_cursorX += offset.x;
		atlasArray->_cursorY += offset.y;
		return true;
	}
	static void GrowAtlasArray(unsigned int atlasArrayID)
	{
		_atlasArrays[atlasArrayID]->GrowAtlasArray();
		_atlasArrays[atlasArrayID]->SetOnZero();
	}

	static unsigned int GetAtlasArrayID(AtlasArray* targetAtlasArray)
	{
		for (const auto& [id, atlasArray] : _atlasArrays)
			if (atlasArray == targetAtlasArray)
				return id;
		return 0;
	}
	static unsigned int GetAtlasID(Atlas* targetAtlas)
	{
		for (const auto& [id, atlas] : _atlases)
			if (atlas == targetAtlas)
				return id;
		return 0;
	}


private:
	friend Atlas;
	friend AtlasArray;
	static inline unsigned int _lastFreeIndex = 1;
	//                 texID  OpenGL_ID
	static inline std::unordered_map<unsigned int, TextureData> _textures;
	static inline std::unordered_map<unsigned int, Atlas*> _atlases;
	static inline std::unordered_map<unsigned int, AtlasArray*> _atlasArrays;
};