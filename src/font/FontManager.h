#pragma once
#include <map>
#include <iostream>
#include <glad/glad.h>
#include "ft2build.h"
#include "../render/TextureManager.h"
#include FT_FREETYPE_H

struct Glyph {
    UV uv;                // UV координаты символа в атласе
	int w, h;             // размеры символа
	int bx, by;           // смещения от базовой линии
	unsigned int adv;     // advance.x
};

struct FontData
{
    FontData() { }
    FontData(int glyphsSize)
        : glyphs(glyphsSize) { }
    
    uint32_t offset;
    std::vector<Glyph> glyphs;
    GLuint atlasID;
};

class FontManager
{
public:
    static void Init() { FT_Init_FreeType(&_ft); }
    static void Shutdown() { FT_Done_FreeType(_ft); }

	static unsigned int AddFont(const char* path, FT_UInt size = 48, uint32_t startIdx = 32, uint32_t endIdx = 128, unsigned char offset = 3)
	{
        FT_Face face; // данные спаршенного файла
        if (FT_New_Face(_ft, path, 0, &face)) { std::cerr << "font load fail\n"; exit(1); }

        FT_Set_Pixel_Sizes(face, 0, size); // размер символов
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // что бы символы не поехали

        FontData FData(endIdx - startIdx);
        FData.offset = startIdx;
        unsigned int atlasMaxHeight = 0;
        unsigned int atlasWidth = 0;
        for (uint32_t c = startIdx; c < endIdx; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            if (atlasMaxHeight < face->glyph->bitmap.rows)
                atlasMaxHeight = face->glyph->bitmap.rows;
            atlasWidth += face->glyph->bitmap.width + offset;
        }
        FData.atlasID = TextureManager::AddAtlas(GL_RED, atlasMaxHeight, atlasWidth);

        // проблема где-то здесь!!!
        for (uint32_t c = startIdx; c < endIdx; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            FT_Bitmap& bmp = face->glyph->bitmap;
            std::vector<unsigned char> pixels(bmp.rows * bmp.width);
            // Копируем bitmap
            for (unsigned int y = 0; y < bmp.rows; y++) {
                for (unsigned int x = 0; x < bmp.width; x++) {
                    pixels[(bmp.rows-1-y) * bmp.width + x] = bmp.buffer[y * bmp.pitch + x];
                }
            }

            UV uv = TextureManager::AddTextureToAtlas(FData.atlasID, bmp.rows, bmp.width, pixels.data(), offset);

            FData.glyphs[c-startIdx] = {
                uv,
                (int)bmp.width, (int)bmp.rows,
                face->glyph->bitmap_left,
                face->glyph->bitmap_top,
                (unsigned int)face->glyph->advance.x
            };
        }
        _fonts[_lastFreeIndex] = FData;
        FT_Done_Face(face);
        return _lastFreeIndex++;
	}

    static void UseFont(unsigned int fontID, int slot)
    {
        TextureManager::BindTexture(_fonts[fontID].atlasID, slot);
    }

    inline static FontData* GetFontData(unsigned int fontID) // добавить const в возвращаемом типе
    {
        return &_fonts[fontID];
    }

private:
    static inline unsigned int _lastFreeIndex = 1;
    static inline std::unordered_map<unsigned int, FontData> _fonts; // можно на вектор поменять
	static inline FT_Library _ft;
};