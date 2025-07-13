#version 460
in vec2 texCoord;
out vec4 frag_color;

uniform sampler2D tex;
uniform float u_alpha;
uniform vec3 spriteColor;

void main() {
   vec4 textureColor = texture(tex, texCoord);
   frag_color = vec4(textureColor.rgb * spriteColor, textureColor.a * u_alpha);
}