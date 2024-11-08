#include <bgfx_shader.sh>

uniform vec4 u_quad_color;

void main() {
	gl_FragColor = u_quad_color;
}