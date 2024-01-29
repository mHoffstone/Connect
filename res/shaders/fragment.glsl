#version 400 core

layout(location = 0) out vec4 color;

in vec4 v_color;
in vec2 v_textureCoordinates;

uniform sampler2D u_texture;

void main(){
	if(v_textureCoordinates.x == 0){
		color = v_color;
	}
	else{
		color = texture(u_texture, v_textureCoordinates);
	}
	//color = vec4(1.0, 0.0, 1.0, 1.0);
}
