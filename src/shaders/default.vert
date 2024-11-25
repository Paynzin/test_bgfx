$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

uniform mat4 u_quad_transform;

void main() {
	gl_Position = vec4(a_position, 0.0, 1.0) * u_quad_transform;
	v_texcoord0 = a_texcoord0;
}