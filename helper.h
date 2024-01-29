#ifndef HELPER_H
#define HELPER_H

#include <string>

struct vec2f{
	float x, y;
	vec2f& operator+=(const vec2f& rhs){
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	friend vec2f operator+(vec2f lhs, const vec2f& rhs){
		lhs += rhs;
		return lhs;
	}
	vec2f& operator-=(const vec2f& rhs){
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	friend vec2f operator-(vec2f lhs, const vec2f& rhs){
		lhs -= rhs;
		return lhs;
	}
};
struct vec3f{
	float x, y, z;
};
struct vec4f{
	float x, y, z, w;
};

struct vertex{
	vec2f position;
	vec4f color;
	vec2f textureCoordinates;
};

struct rectangle{
	vec2f position;
	vec2f dimensions;
};
struct vertex_rectangle : public rectangle{
	vec4f color;
	int texture;
};

struct line{
	vec2f position1;
	vec2f position2;
	vec4f color;
};

bool overlap(const vec2f& v, const rectangle& r);
bool overlap(const rectangle& r1, const rectangle& r2);

rectangle toRect(const vec2f& v1, const vec2f& v2);

std::string readFile(const std::string& path);

unsigned char* readImage(const std::string& path, int width, int height);

bool setProjectionMatrix(unsigned int program, float left, float right, float top, float bottom);

unsigned int createShader(const std::string& vertexPath, const std::string& fragmentPath);

vec2f worldToScreen(vec2f point, vec2f translation, float scale);
vec2f screenToWorld(vec2f point, vec2f translation, float scale);

#endif // HELPER_H
