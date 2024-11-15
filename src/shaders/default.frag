$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 u_quad_color;
SAMPLER2D(s_texture, 0);

void main() {
	gl_FragColor = texture2D(s_texture, v_texcoord0) * u_quad_color;
	//gl_FragColor = texture2D(s_texture, v_texcoord0);
	//gl_FragColor = u_quad_color;
}