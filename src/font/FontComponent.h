#pragma once

#include "FontManager.h"
#include "../object/Component.h"
#include "../object/object.h"
#include "../render/Mesh.h"
#include "../render/Material.h"


class StaticText : public Component
{
public:
    StaticText(Object* obj, unsigned int fontID = 1, std::string text = "text")
		: Component(obj)
	{
        std::string tex = "ourTexture";
        obj->GetComponent<Material>()->SetTextureValue(tex, fontID);

        FontData* fd = FontManager::GetFontData(fontID);
		float* meshData = new float[text.size() * 16];
		GLuint* meshIndices = new GLuint[text.size() * 6];
		int i_counter = 0;
		int i_ptr = 0;
		int d_ptr = 0;
		float x_ptr = 0;
		float y_ptr = 0;
        for (unsigned char c : text) {
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 3;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter + 1;
            i_counter += 4;

            Glyph glyph = fd->glyphs[c - fd->offset];

            float xpos = x_ptr + glyph.bx;
            float ypos = y_ptr - (glyph.h - glyph.by);

            // левый нижний
            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v0;

            // левый верхний
            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v1;

            // правый верхний
            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v1;

            // правый нижний
            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v0;

            x_ptr += glyph.adv >> 6;
        }

		Mesh* textMesh = obj->AddComponent<Mesh>(GL_TRIANGLES, text.size() * 6);
		textMesh->Use();
		textMesh->AllocateData(meshData, sizeof(float) * text.size() * 16);
		textMesh->AllocateIndices(meshIndices, GL_STATIC_DRAW);
		textMesh->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0,
			1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(2 * sizeof(float)));

		delete[] meshData;
		delete[] meshIndices;
	}
	~StaticText() { }	

private:
};

class DynamicText : public Component
{
public:
    DynamicText(Object* obj, GLenum drawType = GL_DYNAMIC_DRAW, unsigned int fontID = 1, std::string text = "dynamic text", unsigned int maxLength = 256)
        : Component(obj), _fontID(fontID), _text(text), _maxLength(maxLength)
    {
        _material = obj->GetComponent<Material>();
        std::string tex = "ourTexture";
        _material->SetTextureValue(tex, fontID);
        _fd = FontManager::GetFontData(fontID);

        float* meshData = new float[maxLength * 16];
        GLuint* meshIndices = new GLuint[text.size() * 6];
        int i_counter = 0;
        int i_ptr = 0;
        int d_ptr = 0;
        float x_ptr = 0;
        float y_ptr = 0;
        for (unsigned char c : text) {
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 3;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter + 1;
            i_counter += 4;

            Glyph glyph = _fd->glyphs[c - _fd->offset];

            float xpos = x_ptr + glyph.bx;
            float ypos = y_ptr - (glyph.h - glyph.by);

            // левый нижний
            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v0;

            // левый верхний
            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v1;

            // правый верхний
            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v1;

            // правый нижний
            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v0;

            x_ptr += glyph.adv >> 6;
        }

        _textMesh = obj->AddComponent<Mesh>(GL_TRIANGLES, maxLength * 6);
        _textMesh->Use();
        _textMesh->AllocateData(meshData, sizeof(float) * maxLength * 16);
        _textMesh->AllocateIndices(meshIndices, drawType);
        _textMesh->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0,
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));
        _textMesh->SetElentsCount(_text.size() * 6);
        delete[] meshData;
        delete[] meshIndices;
    }
    ~DynamicText() {}

    void SetFont(unsigned int fontID)
    {
        _fontID = fontID;
        fontDirty = true;
    }
    void SetText(std::string text)
    {
        _text = text;
    }
    void Recalculate()
    {
        std::string tex = "ourTexture";
        _material->SetTextureValue(tex, _fontID);
        if (fontDirty)
            _fd = FontManager::GetFontData(_fontID);

        float* meshData = new float[_text.size() * 16];
        GLuint* meshIndices = new GLuint[_text.size() * 6];
        int i_counter = 0;
        int i_ptr = 0;
        int d_ptr = 0;
        float x_ptr = 0;
        float y_ptr = 0;
        for (unsigned char c : _text) {
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 3;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter + 1;
            i_counter += 4;

            Glyph glyph = _fd->glyphs[c - _fd->offset];

            float xpos = x_ptr + glyph.bx;
            float ypos = y_ptr - (glyph.h - glyph.by);

            // левый нижний
            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v0;

            // левый верхний
            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v1;

            // правый верхний
            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v1;

            // правый нижний
            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v0;

            x_ptr += glyph.adv >> 6;
        }

        _textMesh->Use();
        _textMesh->UpdateData(meshData, sizeof(float) * _text.size() * 16, 0);
        _textMesh->UpdateIndices(meshIndices, sizeof(GLuint) * _text.size() * 6, 0);
        _textMesh->SetElentsCount(_text.size() * 6);

        delete[] meshData;
        delete[] meshIndices;
    }

private:
    bool fontDirty = false;
    unsigned int _maxLength;
    FontData* _fd;
    Material* _material;
    Mesh* _textMesh;
    std::string _text;
    unsigned int _fontID;
};