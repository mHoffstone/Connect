#include "helper.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>

/*bool overlap(vec2f pos, vec2f dimensions, vec2f pos2){
	return pos2.x >= pos.x && pos2.x < pos.x + dimensions.x &&
		   pos2.y >= pos.y && pos2.y < pos.y + dimensions.y;
}
bool overlap(vec2f pos1, vec2f dimensions1, vec2f pos2, vec2f dimensions2){
	//return pos2.x >= pos1.x && pos2.x < pos1.x + dimensions1.x && pos2.x + dimensions2.x < pos1.x &&
	//	   pos2.y >= pos1.y && pos2.y < pos1.y + dimensions1.y && pos2.y + dimensions2.y < pos1.y;


	return pos2.x + dimensions2.x > pos1.x && pos2.x < pos1.x + dimensions1.x &&
		   pos2.y + dimensions2.y > pos1.y && pos2.y < pos1.y + dimensions1.y;
}*/
bool overlap(const vec2f& v, const rectangle& r){
	return v.x >= r.position.x && v.x < r.position.x + r.dimensions.x &&
		   v.y >= r.position.y && v.y < r.position.y + r.dimensions.y;
}
bool overlap(const rectangle& r1, const rectangle& r2){
	return r2.position.x + r2.dimensions.x > r1.position.x && r2.position.x < r1.position.x + r1.dimensions.x &&
		   r2.position.y + r2.dimensions.y > r1.position.y && r2.position.y < r1.position.y + r1.dimensions.y;
}
rectangle toRect(const vec2f& v1, const vec2f& v2){
	vec2f s1 = vec2f{v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y};
	vec2f s2 = vec2f{v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y};
	return rectangle{
		s1,
		vec2f{s2.x - s1.x, s2.y - s1.y}
	};
}

std::string readFile(const std::string& path){
	std::ifstream reader;
	reader.open(path, std::ios::in|std::ios::binary|std::ios::ate);
	if(reader.is_open()){
		std::streampos size = reader.tellg();
		char* data = new char[size];
		reader.seekg(0, std::ios::beg);
		reader.read(data, size);
		reader.close();
		std::string str(data, size);
		delete[] data;
		reader.close();
		return str;
	}
	else{
		std::cerr << "Could not open file \"" << path << "\"" << std::endl;
	}
	return 0;
}

unsigned char* readImage(const std::string& path, int width, int height){
	std::ifstream reader;
	reader.open(path, std::ios::in|std::ios::binary);

	if(reader.is_open()){
		int size = width*height*4;
		unsigned char* data = new unsigned char[size];
		int rowSize = width*4;
		//char* row = new char[rowSize];
		for(int i = 0; i < height; i++){
			reader.read((char*)(data + (height-i-1)*rowSize), rowSize);
		}
		//reader.read((char*)data, size);
		reader.close();
		return data;
	}
	else return 0;
}

bool setProjectionMatrix(unsigned int program, float left, float right, float top, float bottom){
	float matrix[4*4]{
		2/(right-left), 0.0f,			 0.0f, -(right+left)/(right-left),
		0.0f,			2/(top-bottom),  0.0f, -(top+bottom)/(top-bottom),
		0.0f,			0.0f,			-1.0f,  0.0f,
		0.0f,			0.0f,			 0.0f,  1.0f
	};

	int location = glGetUniformLocation(program, "u_projection");
	if(location == -1) return false;

	glUniformMatrix4fv(location, 1, false, &matrix[0]);

	return true;
}

unsigned int compileShader(const std::string& source, unsigned int type){
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE){
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length];
		glGetShaderInfoLog(id, length, &length, message);

		std::cerr << "Failed to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex " : "fragment ") <<
			"shader!" << std::endl;
		std::cout << message << std::endl;

		delete[] message;

		return 0;
	}

	return id;
}

unsigned int createShader(const std::string& vertexPath, const std::string& fragmentPath){
	unsigned int program = glCreateProgram();
	std::string vertexCode = readFile(vertexPath);
	std::string fragmentCode = readFile(fragmentPath);
	unsigned int vs = compileShader(vertexCode, GL_VERTEX_SHADER);
	unsigned int fs = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

	if(vs == 0 || fs == 0){
		glDeleteShader(vs);
		glDeleteShader(fs);
		return 0;
	}

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glValidateProgram(program);

	int result;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if(result == GL_FALSE){
		int length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

		char* message = new char[length];
		glGetProgramInfoLog(program, length, &length, message);

		std::cerr << "Failed to link program!" << std::endl;
		if(length > 0) std::cout << message << std::endl;

		delete[] message;
		glDeleteProgram(program);

		glDetachShader(program, vs);
		glDetachShader(program, fs);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return 0;
	}

	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if(result == GL_FALSE){
		int length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

		char* message = new char[length];
		glGetProgramInfoLog(program, length, &length, message);

		std::cerr << "Failed to validate program!" << std::endl;
		if(length > 0) std::cout << message << std::endl;

		delete[] message;
		glDeleteProgram(program);

		glDetachShader(program, vs);
		glDetachShader(program, fs);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return 0;
	}

	glDetachShader(program, vs);
	glDetachShader(program, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

vec2f worldToScreen(vec2f point, vec2f translation, float scale){
	return vec2f{(point.x + translation.x)*scale, (point.y + translation.y)*scale};
}
vec2f screenToWorld(vec2f point, vec2f translation, float scale){
	return vec2f{point.x/scale - translation.x, point.y/scale - translation.y};
}
