#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

struct Particle {
    vec2 position;
    vec2 velocity;
    float life;
    float maxLife;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform vec3 uPosition;
uniform float uRotation;
uniform vec2 uScale;
uniform mat4 uViewProj;

out vec4 vColor;
out float vLifeRatio;

void main() {
    Particle p = particles[gl_InstanceID];

    if (p.life >= p.maxLife) {
        gl_Position = vec4(10.0, 10.0, 10.0, 1.0);
        return;
    }

    // трансформируем квад
    vec2 scaledPos = aPos * 1;
    float cosR = cos(uRotation);
    float sinR = sin(uRotation);
    vec2 rotatedPos = vec2(
        scaledPos.x * cosR - scaledPos.y * sinR,
        scaledPos.x * sinR + scaledPos.y * cosR
    );

    // позиция частицы тоже с учётом поворота и скейла объекта
    vec2 particleWorldPos = vec2(
        p.position.x * cosR - p.position.y * sinR,
        p.position.x * sinR + p.position.y * cosR
    ) * uScale;

    vec2 finalPos = rotatedPos + particleWorldPos + uPosition.xy;
    gl_Position = uViewProj * vec4(finalPos, uPosition.z, 1.0);

    vColor = vec4(aUV, 1.0f, 1.0f);
    vLifeRatio = p.life / p.maxLife;
}