#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLayout;

out vec3 vTexCoord;

uniform vec3 uPosition;
uniform float uRotation;
uniform vec2 uScale;
uniform float time; // Добавлено для анимации волны
uniform mat4 uViewProj;

void main() {
    // Сохраняем логику текстурных координат из нового шейдера
    vTexCoord = vec3(aTexCoord, aLayout);
    
    // Масштабирование
    vec2 scaledPos = aPos * uScale;
    
    // Эффект волны (применяется до поворота, чтобы волна шла вдоль строки текста)
    float waveY = sin(scaledPos.x * 2.0 + time * 5.0) * 0.1;
    vec2 wavePos = vec2(scaledPos.x, scaledPos.y + waveY);
    
    // Поворот измененной волной позиции
    float cosR = cos(uRotation);
    float sinR = sin(uRotation);
    vec2 rotatedPos = vec2(
        wavePos.x * cosR - wavePos.y * sinR,
        wavePos.x * sinR + wavePos.y * cosR
    );

    // Финальная позиция
    vec4 worldPos = vec4(rotatedPos + uPosition.xy, uPosition.z, 1.0);
    gl_Position = uViewProj * worldPos;
}
