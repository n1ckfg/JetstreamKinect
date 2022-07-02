#version 150

// this is how we receive the texture
uniform sampler2DRect uColorTex;

in vec2 texCoordVarying;

out vec4 outputColor;
 
void main() {
    outputColor = texture(uColorTex, texCoordVarying);
}