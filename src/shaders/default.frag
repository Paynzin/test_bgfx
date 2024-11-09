uniform vec4 u_quad_color;

#include <bgfx_shader.sh>

void main() {
	gl_FragColor = u_quad_color;
}