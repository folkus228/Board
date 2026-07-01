#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord; // UV-���������� �� ������

out vec2 vTexCoord;

uniform vec3 uPosition;
uniform float uRotation;
uniform vec2 uScale;
uniform float time;

uniform mat4 uViewProj;

void main() {
    vTexCoord = aTexCoord;
    vec2 scaledPos = aPos * uScale;
    float cosR = cos(uRotation);
    float sinR = sin(uRotation);
    vec2 rotatedPos = vec2(
        scaledPos.x * cosR - scaledPos.y * sinR,
        scaledPos.x * sinR + scaledPos.y * cosR
    );

    vec4 worldPos = vec4(rotatedPos + uPosition.xy, uPosition.z, 1.0);
    gl_Position = uViewProj * worldPos;
}