#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D ourTexture; // Тот самый слот GL_TEXTURE0

void main() {
    // texture() выбирает цвет из атласа по вашим UV
    FragColor = texture(ourTexture, vTexCoord);
}
