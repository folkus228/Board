#version 330 core
void main() {
    // Игнорируем входящие данные, рисуем треугольник в центре экрана
    const vec2 pos[3] = vec2[3](vec2(-0.5, -0.5), vec2(0.5, -0.5), vec2(0.0, 0.5));
    gl_Position = vec4(pos[gl_VertexID % 3], 0.0, 1.0);
}
