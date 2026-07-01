#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;

uniform vec4 uniColor;
uniform sampler2D ourTexture; // Тот самый слот GL_TEXTURE0

void main() {
    float alpha = texture(ourTexture, vTexCoord).r;
    FragColor = vec4(uniColor.rgb, uniColor.a * alpha);
}
