/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <numeric>

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

shared_ptr<Shape> shape;
shared_ptr<Shape> crystalDragon;
shared_ptr<Shape> smoothDragon;

vec3 average(vector<vec3> faceNormals, size_t size)
{
	vector<float> x_values;
	vector<float> y_values;
	vector<float> z_values;
	float average_x = 0;
	float average_y = 0;
	float average_z = 0;
	for (size_t i = 0; i < size; i++)
	{
		x_values.push_back(faceNormals[i][0]);
		y_values.push_back(faceNormals[i][1]);
		z_values.push_back(faceNormals[i][2]);
	}

	average_x = accumulate(x_values.begin(), x_values.end(), 0.0f) / size;
	average_y = accumulate(y_values.begin(), y_values.end(), 0.0f) / size;
	average_z = accumulate(z_values.begin(), z_values.end(), 0.0f) / size;

	return vec3(average_x, average_y, average_z);
}

void group_same_vertices(vector<float> *posBuf, vector<unsigned int> *duplicates, vec3 vertex, unsigned int index)
{
	for (int i = index; i < (*posBuf).size(); i+=3)
	{
		if (abs((*posBuf)[i] - vertex[0]) < 0.0000001f && abs((*posBuf)[i + 1] - vertex[1]) < 0.0000001f && abs((*posBuf)[i + 2] - vertex[2]) < 0.0000001f)
		{
			(*duplicates).push_back(i/3);
		}
	}
}

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		rot = glm::vec3(0, 0, 0);
		pos = glm::vec3(0, 0, -5);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 1 * ftime;
		}
		else if (s == 1)
		{
			speed = -1 * ftime;
		}
		float yangle = 0;
		if (a == 1)
			yangle = -1 * ftime;
		else if (d == 1)
			yangle = 1 * ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager* windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Contains vertex information for OpenGL
	GLuint vao;

	// Data necessary to give our box to OpenGL
	GLuint vertex_buffer, nor_buffer, tex_buffer;

	size_t dragonSize;

	//texture data
	GLuint Texture;

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX << " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, vao);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 6, sizeof(float) * 2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow* window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources";
		// Initialize mesh.
		/*shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();*/

		int width, height, channels;
		char filepath[1000];

		smoothDragon = make_shared<Shape>();
		smoothDragon->loadMesh(resourceDirectory + "/alduin.obj");

		crystalDragon = make_shared<Shape>();
		crystalDragon->loadMesh(resourceDirectory + "/alduin.obj");
		

		vector<unsigned int> *eleBuf = smoothDragon->eleBuf;
		vector<float>* posBuf = smoothDragon->posBuf;
		vector<float>* texBuf = smoothDragon->texBuf;

		
		vector<float> normals((*eleBuf).size() * 3);
		vector<float> expandedPosBuf((*eleBuf).size() * 3);
		vector<float> expandedTexBuf((*eleBuf).size() * 2);

		for (int i = 0; i < (*eleBuf).size(); i += 3) 
		{
			//vertex 1
			expandedPosBuf[i * 3]     = (*posBuf)[(*eleBuf)[i] * 3];
			expandedPosBuf[i * 3 + 1] = (*posBuf)[(*eleBuf)[i] * 3 + 1];
			expandedPosBuf[i * 3 + 2] = (*posBuf)[(*eleBuf)[i] * 3 + 2];

			expandedTexBuf[i * 2]	  = (*texBuf)[(*eleBuf)[i] * 2];
			expandedTexBuf[i * 2 + 1] = (*texBuf)[(*eleBuf)[i] * 2 + 1];

			//vertex 2
			expandedPosBuf[i * 3 + 3] = (*posBuf)[(*eleBuf)[i + 1] * 3];
			expandedPosBuf[i * 3 + 4] = (*posBuf)[(*eleBuf)[i + 1] * 3 + 1];
			expandedPosBuf[i * 3 + 5] = (*posBuf)[(*eleBuf)[i + 1] * 3 + 2];

			expandedTexBuf[i * 2 + 2] = (*texBuf)[(*eleBuf)[i + 1] * 2];
			expandedTexBuf[i * 2 + 3] = (*texBuf)[(*eleBuf)[i + 1] * 2 + 1];

			//vertex 3
			expandedPosBuf[i * 3 + 6] = (*posBuf)[(*eleBuf)[i + 2] * 3];
			expandedPosBuf[i * 3 + 7] = (*posBuf)[(*eleBuf)[i + 2] * 3 + 1];
			expandedPosBuf[i * 3 + 8] = (*posBuf)[(*eleBuf)[i + 2] * 3 + 2];

			expandedTexBuf[i * 2 + 4] = (*texBuf)[(*eleBuf)[i + 2] * 2];
			expandedTexBuf[i * 2 + 5] = (*texBuf)[(*eleBuf)[i + 2] * 2 + 1];
		}

		for (int i = 0; i < expandedPosBuf.size(); i += 9)
		{
			vec3 vertex0 = vec3(expandedPosBuf[i], expandedPosBuf[i + 1], expandedPosBuf[i + 2]);
			vec3 vertex1 = vec3(expandedPosBuf[i + 3], expandedPosBuf[i + 4], expandedPosBuf[i + 5]);
			vec3 vertex2 = vec3(expandedPosBuf[i + 6], expandedPosBuf[i + 7], expandedPosBuf[i + 8]);

			vec3 faceNormal = normalize(cross(vertex2 - vertex0, vertex1 - vertex0));

			normals[i] = faceNormal[0];
			normals[i + 1] = faceNormal[1];
			normals[i + 2] = faceNormal[2];
			normals[i + 3] = faceNormal[0];
			normals[i + 4] = faceNormal[1];
			normals[i + 5] = faceNormal[2];
			normals[i + 6] = faceNormal[0];
			normals[i + 7] = faceNormal[1];
			normals[i + 8] = faceNormal[2];
		}

		vector<float> smoothNorBuf(expandedPosBuf.size());

		vector<unsigned int> skipIndices;
		vector<unsigned int> duplicates;

		for (int i = 0; i < expandedPosBuf.size(); i += 3)
		{
			if (find(skipIndices.begin(), skipIndices.end(), i/3) == skipIndices.end())
			{
				vec3 vertex = vec3(expandedPosBuf[i], expandedPosBuf[i + 1], expandedPosBuf[i + 2]);
				duplicates.push_back(i/3);
				group_same_vertices(&expandedPosBuf, &duplicates, vertex, i+3);
				vector<vec3> faceNormals;
				for (int d : duplicates)
				{
					faceNormals.push_back(vec3(normals[d * 3], normals[d * 3 + 1], normals[d * 3 + 2]));
					skipIndices.push_back(d);
				}
				vec3 avg = normalize(average(faceNormals, faceNormals.size()));
				for (int d : duplicates)
				{
					smoothNorBuf[d * 3] = avg[0];
					smoothNorBuf[d * 3 + 1] = avg[1];
					smoothNorBuf[d * 3 + 2] = avg[2];
				}
				duplicates.clear();
			}
		}

		smoothDragon->posBuf = &expandedPosBuf;
		smoothDragon->texBuf = &expandedTexBuf;
		smoothDragon->norBuf = &smoothNorBuf;

		crystalDragon->posBuf = &expandedPosBuf;
		crystalDragon->texBuf = &expandedTexBuf;
		crystalDragon->norBuf = &normals;

		dragonSize = expandedPosBuf.size();

		smoothDragon->resize();
		smoothDragon->init();
		crystalDragon->resize();
		crystalDragon->init();

		//texture 1
		string str = resourceDirectory + "/alduin.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("lightpos");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransZ * RotateY * RotateX * S;
		vec3 lightdirection = vec3(-30, 40, 100);

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("lightpos"), 1, &lightdirection[0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		smoothDragon->draw(prog, FALSE, dragonSize);
		prog->unbind();

		glm::mat4 TransX = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
		M = TransX * RotateY * RotateX * S;
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("lightpos"), 1, &lightdirection[0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		crystalDragon->draw(prog, FALSE, dragonSize);
		prog->unbind();

	}

};
//******************************************************************************************
int main(int argc, char** argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application* application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager* windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
		// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
