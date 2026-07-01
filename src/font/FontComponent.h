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
        Material* mat = obj->GetComponent<Material>();
        mat->SetTextureValue(tex, fontID);
        FontData* fd = FontManager::GetFontData(fontID);

        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);
        hb_buffer_guess_segment_properties(buf);
        hb_shape(fd->hb_font, buf, nullptr, 0);

        unsigned int glyphCount;
        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &glyphCount);
        hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &glyphCount);

        float* meshData = new float[glyphCount * 20];
        GLuint* meshIndices = new GLuint[glyphCount * 6];
        int i_counter = 0;
        int i_ptr = 0;
        int d_ptr = 0;
        float x_ptr = 0;
        float y_ptr = 0;

        for (unsigned int i = 0; i < glyphCount; i++) {
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 3;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter + 1;
            i_counter += 4;

            if (fd->glyphs.find(info[i].codepoint) == fd->glyphs.end())
                mat->SetTextureValue(tex, FontManager::LoadGlyphIntoAtlas(fontID, info[i].codepoint));
            Glyph& glyph = fd->glyphs[info[i].codepoint];
            float xpos = x_ptr + glyph.bx + pos[i].x_offset / 64.0f;
            float ypos = y_ptr - (glyph.h - glyph.by) + pos[i].y_offset / 64.0f;

            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v0;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v1;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v1;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v0;
            meshData[d_ptr++] = glyph.layer;

            x_ptr += pos[i].x_advance / 64.0f;
        }

        hb_buffer_destroy(buf);

        Mesh* textMesh = obj->AddComponent<Mesh>(GL_TRIANGLES, glyphCount * 6);
        textMesh->Use();
        textMesh->AllocateData(meshData, sizeof(float) * glyphCount * 20);
        textMesh->AllocateIndices(meshIndices, GL_STATIC_DRAW);
        textMesh->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0,
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(2 * sizeof(float)),
            2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(4 * sizeof(float)));
        textMesh->_subMeshes.emplace_back(obj->GetComponent<Material>(), 0, static_cast<int>(glyphCount * 6));
        delete[] meshData;
        delete[] meshIndices;
    }
    ~StaticText() {}
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

        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);
        hb_buffer_guess_segment_properties(buf);
        hb_shape(_fd->hb_font, buf, nullptr, 0);

        unsigned int glyphCount;
        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &glyphCount);
        hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &glyphCount);

        meshData = new float[maxLength * 20];
        meshIndices = new GLuint[maxLength * 6];
        int i_counter = 0;
        int i_ptr = 0;
        int d_ptr = 0;
        float x_ptr = 0;
        float y_ptr = 0;
        for (unsigned int i = 0; i < glyphCount; i++) {
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 3;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter + 1;
            i_counter += 4;

            if (_fd->glyphs.find(info[i].codepoint) == _fd->glyphs.end())
                FontManager::LoadGlyphIntoAtlas(fontID, info[i].codepoint);
            Glyph& glyph = _fd->glyphs[info[i].codepoint];
            float xpos = x_ptr + glyph.bx + pos[i].x_offset / 64.0f;
            float ypos = y_ptr - (glyph.h - glyph.by) + pos[i].y_offset / 64.0f;

            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v0;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v1;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v1;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v0;
            meshData[d_ptr++] = glyph.layer;

            x_ptr += pos[i].x_advance / 64.0f;
        }

        hb_buffer_destroy(buf);

        _textMesh = obj->AddComponent<Mesh>(GL_TRIANGLES, maxLength * 6);
        _textMesh->Use();
        _textMesh->AllocateData(meshData, sizeof(float) * maxLength * 20);
        _textMesh->AllocateIndices(meshIndices, drawType);
        _textMesh->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0,
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(2 * sizeof(float)),
            2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(4 * sizeof(float)));
        _textMesh->_subMeshes.emplace_back(obj->GetComponent<Material>(), 0, static_cast<int>(glyphCount * 6));
    }
    ~DynamicText() 
    {
        delete[] meshData;
        delete[] meshIndices;
    }

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

        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf8(buf, _text.c_str(), -1, 0, -1);
        hb_buffer_guess_segment_properties(buf);
        hb_shape(_fd->hb_font, buf, nullptr, 0);

        unsigned int glyphCount;
        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &glyphCount);
        hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &glyphCount);

        int i_counter = 0;
        int i_ptr = 0;
        int d_ptr = 0;
        float x_ptr = 0;
        float y_ptr = 0;
        for (unsigned int i = 0; i < glyphCount; i++) {
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 3;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter;
            meshIndices[i_ptr++] = i_counter + 2;
            meshIndices[i_ptr++] = i_counter + 1;
            i_counter += 4;

            if (_fd->glyphs.find(info[i].codepoint) == _fd->glyphs.end())
                FontManager::LoadGlyphIntoAtlas(_fontID, info[i].codepoint);
            Glyph& glyph = _fd->glyphs[info[i].codepoint];
            float xpos = x_ptr + glyph.bx + pos[i].x_offset / 64.0f;
            float ypos = y_ptr - (glyph.h - glyph.by) + pos[i].y_offset / 64.0f;

            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v0;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u1;
            meshData[d_ptr++] = glyph.uv.v1;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos + glyph.h;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v1;
            meshData[d_ptr++] = glyph.layer;

            meshData[d_ptr++] = xpos + glyph.w;
            meshData[d_ptr++] = ypos;
            meshData[d_ptr++] = glyph.uv.u0;
            meshData[d_ptr++] = glyph.uv.v0;
            meshData[d_ptr++] = glyph.layer;

            x_ptr += pos[i].x_advance / 64.0f;
        }

        hb_buffer_destroy(buf);

        _textMesh->Use();
        _textMesh->UpdateData(meshData, sizeof(float) * _text.size() * 20, 0);
        _textMesh->UpdateIndices(meshIndices, sizeof(GLuint) * glyphCount * 6, 0);
        _textMesh->_subMeshes[0].count = static_cast<int>(glyphCount * 6);
    }

private:
    int _maxLength;
    FontData* _fd;
    Material* _material;
    Mesh* _textMesh;
    std::string _text;
    float* meshData;
    GLuint* meshIndices;
    unsigned int _fontID;
    bool fontDirty = false;
};

struct TextRun
{
    unsigned int fontID = 1;
    std::string text = "text";
    Material* mat;
};

class ProStaticText : public Component
{
public:
    ProStaticText(Object* obj, const std::vector<TextRun>& runs)
        : Component(obj)
    {
        struct ShapedRun
        {
            Material* material;
            FontData* font;
            hb_buffer_t* buffer;
            unsigned int glyphCount;
            unsigned int fontID;
        };

        std::vector<ShapedRun> shapedRuns;
        unsigned int totalGlyphs = 0;

        //
        // Первый проход — shape текста
        //
        for (const auto& run : runs)
        {
            FontData* fd = FontManager::GetFontData(run.fontID);

            hb_buffer_t* buf = hb_buffer_create();
            hb_buffer_add_utf8(buf, run.text.c_str(), -1, 0, -1);
            hb_buffer_guess_segment_properties(buf);
            hb_shape(fd->hb_font, buf, nullptr, 0);

            unsigned int glyphCount;
            hb_buffer_get_glyph_infos(buf, &glyphCount);

            totalGlyphs += glyphCount;

            shapedRuns.push_back({
                run.mat,
                fd,
                buf,
                glyphCount,
                run.fontID
                });
        }

        std::vector<float> meshData(totalGlyphs * 20);
        std::vector<GLuint> meshIndices(totalGlyphs * 6);

        Mesh* mesh = obj->AddComponent<Mesh>(GL_TRIANGLES, totalGlyphs * 6);
        mesh->Use();

        const std::string textureName = "ourTexture";

        int vertexIndex = 0;
        int dataPtr = 0;
        int indexPtr = 0;
        int indexOffset = 0;

        float penX = 0.0f;
        float penY = 0.0f;

        //
        // Второй проход — строим меш
        //
        for (auto& run : shapedRuns)
        {
            run.material->SetTextureValue(textureName, run.fontID);

            hb_glyph_info_t* info =
                hb_buffer_get_glyph_infos(run.buffer, nullptr);

            hb_glyph_position_t* pos =
                hb_buffer_get_glyph_positions(run.buffer, nullptr);

            mesh->_subMeshes.emplace_back(
                run.material,
                indexOffset,
                run.glyphCount * 6);

            indexOffset += run.glyphCount * 6;

            for (unsigned int i = 0; i < run.glyphCount; ++i)
            {
                meshIndices[indexPtr++] = vertexIndex;
                meshIndices[indexPtr++] = vertexIndex + 3;
                meshIndices[indexPtr++] = vertexIndex + 2;
                meshIndices[indexPtr++] = vertexIndex;
                meshIndices[indexPtr++] = vertexIndex + 2;
                meshIndices[indexPtr++] = vertexIndex + 1;

                vertexIndex += 4;

                auto it = run.font->glyphs.find(info[i].codepoint);

                if (it == run.font->glyphs.end())
                {
                    run.material->SetTextureValue(
                        textureName,
                        FontManager::LoadGlyphIntoAtlas(
                            run.fontID,
                            info[i].codepoint));

                    it = run.font->glyphs.find(info[i].codepoint);
                }

                if (it == run.font->glyphs.end())
                    continue;

                Glyph& glyph = it->second;

                float xpos = penX + glyph.bx + pos[i].x_offset / 64.0f;
                float ypos = penY - (glyph.h - glyph.by) + pos[i].y_offset / 64.0f;

                // bottom-left
                meshData[dataPtr++] = xpos;
                meshData[dataPtr++] = ypos;
                meshData[dataPtr++] = glyph.uv.u1;
                meshData[dataPtr++] = glyph.uv.v0;
                meshData[dataPtr++] = glyph.layer;

                // top-left
                meshData[dataPtr++] = xpos;
                meshData[dataPtr++] = ypos + glyph.h;
                meshData[dataPtr++] = glyph.uv.u1;
                meshData[dataPtr++] = glyph.uv.v1;
                meshData[dataPtr++] = glyph.layer;

                // top-right
                meshData[dataPtr++] = xpos + glyph.w;
                meshData[dataPtr++] = ypos + glyph.h;
                meshData[dataPtr++] = glyph.uv.u0;
                meshData[dataPtr++] = glyph.uv.v1;
                meshData[dataPtr++] = glyph.layer;

                // bottom-right
                meshData[dataPtr++] = xpos + glyph.w;
                meshData[dataPtr++] = ypos;
                meshData[dataPtr++] = glyph.uv.u0;
                meshData[dataPtr++] = glyph.uv.v0;
                meshData[dataPtr++] = glyph.layer;

                penX += pos[i].x_advance / 64.0f;
                penY += pos[i].y_advance / 64.0f;
            }

            hb_buffer_destroy(run.buffer);
        }

        mesh->AllocateData(meshData.data(),
            meshData.size() * sizeof(float));

        mesh->AllocateIndices(meshIndices.data(), GL_STATIC_DRAW);

        mesh->Compile(
            0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0,
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(2 * sizeof(float)),
            2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(4 * sizeof(float)));
    }
};

class ProDynamicText : public Component
{
public:
    ProDynamicText(
        Object* obj,
        const std::vector<TextRun>& runs,
        GLenum drawType = GL_DYNAMIC_DRAW,
        unsigned int maxGlyphs = 256)
        : Component(obj),
        _runs(&runs),
        _maxGlyphs(maxGlyphs)
    {
        meshData.resize(maxGlyphs * 20);
        meshIndices.resize(maxGlyphs * 6);

        _mesh = obj->AddComponent<Mesh>(GL_TRIANGLES, maxGlyphs * 6);
        _mesh->Use();

        _mesh->AllocateData(meshData.data(), meshData.size() * sizeof(float));
        _mesh->AllocateIndices(meshIndices.data(), drawType);

        _mesh->Compile(
            0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0,
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(2 * sizeof(float)),
            2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(4 * sizeof(float)));

        Recalculate();
    }

    void Recalculate()
    {
        const std::string tex = "ourTexture";

        int vertexCounter = 0;
        int indexPtr = 0;
        int dataPtr = 0;
        int indexOffset = 0;

        float penX = 0.0f;
        float penY = 0.0f;

        _mesh->_subMeshes.clear();

        unsigned int totalGlyphs = 0;

        for (const auto& run : *_runs)
        {
            FontData* fd = FontManager::GetFontData(run.fontID);

            run.mat->SetTextureValue(tex, run.fontID);

            hb_buffer_t* buf = hb_buffer_create();
            hb_buffer_add_utf8(buf, run.text.c_str(), -1, 0, -1);
            hb_buffer_guess_segment_properties(buf);
            hb_shape(fd->hb_font, buf, nullptr, 0);

            unsigned int glyphCount;
            hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &glyphCount);
            hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &glyphCount);

            if (totalGlyphs + glyphCount > _maxGlyphs)
            {
                hb_buffer_destroy(buf);
                break;
            }

            _mesh->_subMeshes.emplace_back(
                run.mat,
                indexOffset,
                glyphCount * 6);

            indexOffset += glyphCount * 6;

            for (unsigned int i = 0; i < glyphCount; ++i)
            {
                meshIndices[indexPtr++] = vertexCounter;
                meshIndices[indexPtr++] = vertexCounter + 3;
                meshIndices[indexPtr++] = vertexCounter + 2;
                meshIndices[indexPtr++] = vertexCounter;
                meshIndices[indexPtr++] = vertexCounter + 2;
                meshIndices[indexPtr++] = vertexCounter + 1;

                vertexCounter += 4;

                auto it = fd->glyphs.find(info[i].codepoint);

                if (it == fd->glyphs.end())
                {
                    FontManager::LoadGlyphIntoAtlas(run.fontID, info[i].codepoint);
                    it = fd->glyphs.find(info[i].codepoint);
                }

                if (it == fd->glyphs.end())
                    continue;

                Glyph& glyph = it->second;

                float xpos = penX + glyph.bx + pos[i].x_offset / 64.0f;
                float ypos = penY - (glyph.h - glyph.by) + pos[i].y_offset / 64.0f;

                meshData[dataPtr++] = xpos;
                meshData[dataPtr++] = ypos;
                meshData[dataPtr++] = glyph.uv.u1;
                meshData[dataPtr++] = glyph.uv.v0;
                meshData[dataPtr++] = glyph.layer;

                meshData[dataPtr++] = xpos;
                meshData[dataPtr++] = ypos + glyph.h;
                meshData[dataPtr++] = glyph.uv.u1;
                meshData[dataPtr++] = glyph.uv.v1;
                meshData[dataPtr++] = glyph.layer;

                meshData[dataPtr++] = xpos + glyph.w;
                meshData[dataPtr++] = ypos + glyph.h;
                meshData[dataPtr++] = glyph.uv.u0;
                meshData[dataPtr++] = glyph.uv.v1;
                meshData[dataPtr++] = glyph.layer;

                meshData[dataPtr++] = xpos + glyph.w;
                meshData[dataPtr++] = ypos;
                meshData[dataPtr++] = glyph.uv.u0;
                meshData[dataPtr++] = glyph.uv.v0;
                meshData[dataPtr++] = glyph.layer;

                penX += pos[i].x_advance / 64.0f;
                penY += pos[i].y_advance / 64.0f;
            }

            totalGlyphs += glyphCount;

            hb_buffer_destroy(buf);
        }

        _mesh->Use();

        _mesh->UpdateData(
            meshData.data(),
            totalGlyphs * 20 * sizeof(float),
            0);

        _mesh->UpdateIndices(
            meshIndices.data(),
            totalGlyphs * 6 * sizeof(GLuint),
            0);
    }

private:
    const std::vector<TextRun>* _runs;

    Mesh* _mesh = nullptr;

    std::vector<float> meshData;
    std::vector<GLuint> meshIndices;

    unsigned int _maxGlyphs;
};
