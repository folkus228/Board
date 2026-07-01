#pragma once
#include <map>
#include <iostream>
#include <glad/glad.h>
#include "ft2build.h"
#include <hb.h>
#include <hb-ft.h>
#include "../render/TextureManager.h"
#include FT_FREETYPE_H
#include FT_MODULE_H 
#include <plutosvg-ft.h>

struct Glyph {
    UV uv;                // UV координаты символа в атласе
	int w, h;             // размеры символа
	int bx, by;           // смещения от базовой линии
    int layer;
};

struct FontData
{
    ~FontData() 
    { 
        hb_font_destroy(hb_font);
    }

    std::unordered_map<uint32_t, Glyph> glyphs;
    hb_font_t* hb_font;
    FT_Face face;
    unsigned int atlasArrayID;
};

class FontManager
{
public:
    static void Init() 
    { 
        FT_Init_FreeType(&_ft);
        if (FT_Property_Set(_ft, "ot-svg", "svg-hooks", &plutosvg_ft_hooks)) {
            std::cerr << "Failed to apply SVG-hook PlutoSVG!" << std::endl;
        }
    }
    static void Shutdown() { FT_Done_FreeType(_ft); }

    static unsigned int LoadFont(const char* path, FT_UInt size = 48, unsigned char offset = 3)
    {
        FontData* FData = &_fonts[_lastFreeIndex];
        if (FT_New_Face(_ft, path, 0, &FData->face)) { std::cerr << "font load fail\n"; exit(1); } // create face
        FT_Set_Pixel_Sizes(FData->face, 0, size); // set size
        FData->hb_font = hb_ft_font_create(FData->face, nullptr);
        if (FData->face->face_flags & FT_FACE_FLAG_COLOR)
        {
            FData->atlasArrayID = TextureManager::CreateAtlasArray(GL_RGBA, 1024, 1024, 1);
            std::cout << "Load RGBA font" << std::endl;
        }
        else
        {
            FData->atlasArrayID = TextureManager::CreateAtlasArray(GL_RED, 1024, 1024, 1);
            std::cout << "Load GRAY font" << std::endl;
        }
        return _lastFreeIndex++;
    }
    static unsigned int LoadGlyphIntoAtlas(unsigned int fontID, uint32_t glyph)
    {
        FontData* fd = &_fonts[fontID];
        FT_Load_Glyph(fd->face, glyph, FT_LOAD_COLOR | FT_LOAD_RENDER);
        FT_Bitmap& bmp = fd->face->glyph->bitmap;
        std::vector<unsigned char> pixels;

        if (bmp.pixel_mode == FT_PIXEL_MODE_BGRA)
        {
            pixels.resize(bmp.rows * bmp.width * 4);
            for (unsigned int y = 0; y < bmp.rows; y++)
                for (unsigned int x = 0; x < bmp.width; x++) {
                    int dst = ((bmp.rows - 1 - y) * bmp.width + x) * 4;
                    int src = y * bmp.pitch + x * 4;
                    pixels[dst] = bmp.buffer[src + 2]; // R
                    pixels[dst + 1] = bmp.buffer[src + 1]; // G
                    pixels[dst + 2] = bmp.buffer[src + 0]; // B
                    pixels[dst + 3] = bmp.buffer[src + 3]; // A
                }
            std::cout << "process RGBA glyph: " << glyph << std::endl;
        }
        else if (bmp.pixel_mode == FT_PIXEL_MODE_GRAY)
        {
            pixels.resize(bmp.rows * bmp.width);
            for (unsigned int y = 0; y < bmp.rows; y++)
                for (unsigned int x = 0; x < bmp.width; x++)
                    pixels[(bmp.rows - 1 - y) * bmp.width + x] = bmp.buffer[y * bmp.pitch + x];
            std::cout << "process GRAY glyph: " << glyph << std::endl;
        }
        else
            std::cout << "process None glyph: " << glyph << "  with type: " << int(bmp.pixel_mode) << std::endl;

        UV uv;
        TextureManager::AtlasArray* ar = TextureManager::GetAtlasArray(fd->atlasArrayID);
        if (!TextureManager::AddTextureToEndAtlasArray(fd->atlasArrayID, bmp.rows, bmp.width, pixels.data(), uv))
        {
            TextureManager::GrowAtlasArray(fd->atlasArrayID);
            TextureManager::AddTextureToEndAtlasArray(fd->atlasArrayID, bmp.rows, bmp.width, pixels.data(), uv);
        }
        fd->glyphs[glyph] = {
            uv,
            (int)bmp.width, (int)bmp.rows,
            fd->face->glyph->bitmap_left,
            fd->face->glyph->bitmap_top,
            ar->GetLayers() - 1
        };
        return fd->atlasArrayID;
    }

    static void UseFont(unsigned int fontID, int slot)
    {
        TextureManager::BindTexture(_fonts[fontID].atlasArrayID, slot);
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