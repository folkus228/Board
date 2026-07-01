#version 450 core
out vec4 FragColor;
in vec2 vTexCoord;
uniform sampler2DArray ourTexture;
uniform int uLayer;
void main() {
    FragColor = texture(ourTexture, vec3(vTexCoord, float(uLayer)));
}