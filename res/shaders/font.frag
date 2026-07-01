#version 330 core
out vec4 FragColor;
in vec3 vTexCoord;
uniform vec4 uniColor;
uniform sampler2DArray ourTexture;

void main() {
    vec4 tex = texture(ourTexture, vTexCoord);
    // если RGB пустой (GL_RED атлас даЄт 0 в G и B по дефолту) Ч это чб глиф, alpha = R канал
    // если RGB не пустой Ч цветной глиф, используем как есть
    float isColor = step(0.001, tex.g + tex.b);
    vec4 grayResult = vec4(uniColor.rgb, uniColor.a * tex.r);
    FragColor = mix(grayResult, tex, isColor);
}