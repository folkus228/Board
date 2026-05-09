#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

// Данные передаются для каждого объекта отдельно перед glDraw...
uniform vec3 uPosition;    // x, y, z
uniform float uRotation; // угол в радианах
uniform vec2 uScale;     // ширина, высота

uniform mat4 uViewProj;

out vec2 TexCoord;

void main() {
    // 1. Масштабируем локальные координаты вершины
    vec2 pos = aPos * uScale;

    // 2. Поворачиваем
    float s = sin(uRotation);
    float c = cos(uRotation);
    mat2 rotMat = mat2(c, s, -s, c);
    pos = rotMat * pos;

    // 3. Сдвигаем и проецируем
    // uPosition.z отвечает за глубину в 2D (Z-buffer)
    gl_Position = uViewProj * vec4(pos + uPosition.xy, uPosition.z, 1.0);
    
    TexCoord = aTexCoord;
}
