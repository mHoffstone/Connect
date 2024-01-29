#version 400 core

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec2 i_textureCoordinates;

out vec4 v_color;
out vec2 v_textureCoordinates;

uniform float u_scale;
uniform vec2 u_translation;
uniform mat4 u_projection;

void main(){
	v_color = i_color;
	v_textureCoordinates = i_textureCoordinates;

	gl_Position = u_projection * vec4((i_position + u_translation)*u_scale, 0.0, 1.0);
}
