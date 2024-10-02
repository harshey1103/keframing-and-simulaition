#include "camera.hpp"

#include <iostream>

using namespace COL781;
namespace GL = COL781::OpenGL;
using namespace glm;

GL::Rasterizer r;
GL::ShaderProgram program;

const int len = 5;
const int nv = len*len;
const int nt = 2*(len-1)*(len-1);

vec3 vertices[nv];
vec3 velocities[nv];
vec3 accelerations[nv];
vec3 normals[nv];
ivec3 triangles[nt];

GL::Object object;
GL::AttribBuf vertexBuf, normalBuf;

CameraControl camCtl;

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}

void debug(vec3* a, int n){
	
	for(int i = 0; i < len; i++)
		for(int j = 0; j < len; j++){
			std::cout << i*len+j << " " << vertices[i*len+j] << "\n";
			std::cout << i*len+j << " " << normals[i*len+j] << "\n";
		}

	for(int i = 0; i < len-1; i++)
		for(int j = 0; j < len-1; j++){
			std::cout << 2*(i*(len-1)+j) << " " << triangles[2*(i*(len-1)+j)] << "\n";		
			std::cout << 2*(i*(len-1)+j)+1 << " " << triangles[2*(i*(len-1)+j)+1] << "\n";
		}

}
void initializeScene() {
	object = r.createObject();
	
	for(int i = 0; i < len; i++)
		for(int j = 0; j < len; j++)
			vertices[i*len+j] = vec3(i, 0, j);
	
	vertexBuf = r.createVertexAttribs(object, 0, nv, vertices);
	
	for(int i = 0; i < len; i++)
		for(int j = 0; j < len; j++)       
			normals[i*len+j] = vec3(0, 1, 0);

	normalBuf = r.createVertexAttribs(object, 1, nv, normals);
	
	for(int i = 0; i < len-1; i++)
		for(int j = 0; j < len-1; j++){
			triangles[2*(i*(len-1)+j)] = ivec3(len+j+i*len, 0+j+i*len, 1+j+i*len);
			triangles[2*(i*(len-1)+j)+1] = ivec3(len+j+i*len, 1+j+i*len, len+1+j+i*len);
		}
	
	// debug();

	r.createTriangleIndices(object, nt, triangles);
}

bool inRange(int i)
{
	if(i >= nv || i < 0) return 0;
	return 1;
}

void calculateNormals(int i){
	vec3 normal = ivec3(0.0, 0.0, 0.0);
	// top left
	if(inRange(i-len+1)){
		normal += glm::cross(vertices[i-len]-vertices[i], vertices[i-len+1]-vertices[i]);	
		normal += glm::cross(vertices[i-len+1]-vertices[i], vertices[i+1]-vertices[i]);	
	}
	// bottom left
	if(inRange(i-len-1)){
		normal += glm::cross(vertices[i-1]-vertices[i], vertices[i-len]-vertices[i]);	
	}
	// bottom right
	if(inRange(i+len-1)){
		normal += glm::cross(vertices[i-1]-vertices[i], vertices[i+len-1]-vertices[i]);	
		normal += glm::cross(vertices[i+len-1]-vertices[i], vertices[i+len]-vertices[i]);	
	}
	// top right
	if(inRange(i+len-1)){
		normal += glm::cross(vertices[i+len]-vertices[i], vertices[i+1]-vertices[i]);	
	}
	normals[i] = glm::normalize(normal);
}

void updateScene(float t) {
	std::cout << vertices[6] << " " << velocities[6] << " " << accelerations[6] <<"\n";
	// calculate accelerations 
	for (int i = 0; i < nv; i++)
	{
		if(i/len==0 || i/len==len-1 || i%len==0 || i%len==len-1)
			accelerations[i] = ivec3(0.0, 0.0, 0.0);
		else 
			accelerations[i] = ivec3(0.0, -1.0, 0.0); 
	}

	// calclate velocites 
	for (int i = 0; i < nv; i++)
	{
		velocities[i] += accelerations[i]*t; 
	}

	// calculate positions
	for (int i = 0; i < nv; i++)
	{
		vertices[i] += velocities[i]*t; 
	}

	r.updateVertexAttribs(vertexBuf, nv, vertices);
	
	// calculate normals 
	for (int i = 0; i < nv; i++)
		calculateNormals(i);	

	r.updateVertexAttribs(normalBuf, nv, normals);
}

int main() {
	int width = 640, height = 480;
	if (!r.initialize("Animation", width, height)) {
		return EXIT_FAILURE;
	}
	camCtl.initialize(width, height);
	vec3 cameraPosition((len-1) / 2.0f, len/2.0, -len/2.0);  // Camera above and offset backward
	vec3 lookAtPoint((len-1) / 2.0f, 0, (len-1) / 2.0f);  // Look at the center of the mesh
	vec3 upDirection(0.0f, 1.0f, 0.0f);  // Up is along the y-axis

	camCtl.camera.setCameraView(cameraPosition, lookAtPoint, upDirection);

	program = r.createShaderProgram(
		r.vsBlinnPhong(),
		r.fsBlinnPhong()
	);

	initializeScene();
	while (!r.shouldQuit()) {
        float t = SDL_GetTicks64()*1e-3;
		updateScene(t);

		camCtl.update();
		Camera &camera = camCtl.camera;

		r.clear(vec4(1.0, 1.0, 1.0, 1.0));
		r.enableDepthTest();
		r.useShaderProgram(program);

		r.setUniform(program, "model", glm::mat4(1.0));
		r.setUniform(program, "view", camera.getViewMatrix());
		r.setUniform(program, "projection", camera.getProjectionMatrix());
		r.setUniform(program, "lightPos", camera.position);
		r.setUniform(program, "viewPos", camera.position);
		r.setUniform(program, "lightColor", vec3(1.0f, 1.0f, 1.0f));

		r.setupFilledFaces();
        glm::vec3 orange(1.0f, 0.6f, 0.2f);
        glm::vec3 white(1.0f, 1.0f, 1.0f);
        r.setUniform(program, "ambientColor", 0.4f*orange);
        r.setUniform(program, "diffuseColor", 0.9f*orange);
        r.setUniform(program, "specularColor", 0.8f*white);
        r.setUniform(program, "phongExponent", 100.f);
		r.drawObject(object);

		r.setupWireFrame();
        glm::vec3 black(0.0f, 0.0f, 0.0f);
        r.setUniform(program, "ambientColor", black);
        r.setUniform(program, "diffuseColor", black);
        r.setUniform(program, "specularColor", black);
        r.setUniform(program, "phongExponent", 0.f);
		r.drawObject(object);

		r.show();
	}
}
