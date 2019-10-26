/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>
#include <cstdlib>
#include <ctime>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

# define M_PI          3.141592653589793238462643383279502884L /* pi */

using namespace glm;
using namespace std;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
void generate_circle(size_t num_vertices, float z, int direction, vector<float> *posBuf, vector<unsigned int> *triBuf)
{
	float angle_between_vertices = 2 * M_PI / (num_vertices - 2);

	posBuf->push_back(0);
	posBuf->push_back(0);
	posBuf->push_back(z);

	float r = 0;
	for (int i = 1; i < num_vertices; i += 1)
	{
		float x = cos(r);
		float y = sin(r);

		posBuf->push_back(x);
		posBuf->push_back(y);
		posBuf->push_back(z);

		if (direction == 1)
		{
			triBuf->push_back(i);
			triBuf->push_back((i + 1));
			triBuf->push_back(0);
		}
		else
		{
			triBuf->push_back(i);
			triBuf->push_back(0);
			triBuf->push_back((i + 1));
		}

		r = r + angle_between_vertices;
	}
}
void generate_cylinder(size_t number_of_vertices, vector<float> *posBuf, vector<unsigned int> *triBuf)
{
	vector<float> posBufCircle1;
	vector<unsigned int> indexBufCircle1;
	vector<float> posBufCircle2;
	vector<unsigned int> indexBufCircle2;
	generate_circle(number_of_vertices, 0, 1, &posBufCircle1, &indexBufCircle1);
	generate_circle(number_of_vertices, -1, -1, &posBufCircle2, &indexBufCircle2);

	vector<unsigned int> connection;

	for (size_t i = 0; i < number_of_vertices * 3; i++)
	{
		posBuf->push_back(posBufCircle1[i]);
	}
	for (size_t i = 0; i < (number_of_vertices - 1) * 3; i++)
	{
		triBuf->push_back(indexBufCircle1[i]);
	}
	for (size_t i = 0; i < number_of_vertices * 3; i++)
	{
		posBuf->push_back(posBufCircle2[i]);
	}
	for (size_t i = 0; i < (number_of_vertices - 1) * 3; i++)
	{
		triBuf->push_back(number_of_vertices + indexBufCircle2[i]);
	}
	for (size_t i = 0; i < number_of_vertices - 1; i++)
	{
		triBuf->push_back(i);
		triBuf->push_back(i + number_of_vertices);
		triBuf->push_back(i + 1);

		triBuf->push_back(i + 1);
		triBuf->push_back(i + number_of_vertices);
		triBuf->push_back(i + number_of_vertices + 1);
	}
}
class mesh
{
public:
	std::shared_ptr<Program> shader_prog;
	unsigned int vertexArrayID;
	unsigned int indexBufferID;
	size_t size;
	GLfloat* P;
	GLfloat* V;
	GLfloat* M;

	void draw()
	{
		shader_prog->bind();
		glUniformMatrix4fv(shader_prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(shader_prog->getUniform("V"), 1, GL_FALSE, V);
		glUniformMatrix4fv(shader_prog->getUniform("M"), 1, GL_FALSE, M);
		glBindVertexArray(vertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
		shader_prog->unbind();
	}

	void setPVM(GLfloat* Pp, GLfloat* Vp, GLfloat* Mp)
	{
		P = Pp;
		V = Vp;
		M = Mp;
	}
};

class camera
{
public:
	vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = vec3(0, 0, -15);
		rot = vec3(0, 0, 0);
	}
	mat4 process(double frametime)
	{
		double ftime = frametime;
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = yangle;// -3 * ftime;
		else if(d==1)
			yangle = yangle;//3*ftime;
		rot.y += yangle;
		mat4 R = rotate(mat4(1), rot.y, vec3(0, 1, 0));
		vec4 dir = vec4(0, 0, speed,1);
		dir = dir*R;
		pos += vec3(dir.x, dir.y, dir.z);
		mat4 T = translate(mat4(1), pos);
		return R*T;
	}
};

camera mycam;
mesh mycylinder;
mesh mysnowflake;

class Application : public EventCallbacks
{

public:
	int kn = 0;
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, cylProg, snowProg, globeProg, shapeprog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;

	Shape shape;

	vector<vec4> snowflakes;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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
		if (key == GLFW_KEY_N && action == GLFW_PRESS) kn = 1;
		if (key == GLFW_KEY_N && action == GLFW_RELEASE) kn = 0;
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	void init_snowflakes()
	{
		const unsigned int number_of_snowflakes = 20;

		srand(static_cast <unsigned> (time(0)));

		for (int i = 0; i < number_of_snowflakes; i++)
		{
			float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 1.2f - 0.6f;
			float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2 - 1.5f;
			float z = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 1.2f - 0.6f;

			float direction = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 3.0f - 1.5f;
			
			snowflakes.push_back(vec4(x, y, z, direction));
		}
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources";
		shape.loadMesh(resourceDirectory + "/sphere.obj");
		shape.resize();
		shape.init();

		vector<float> posBuf;
		vector<unsigned int> indexBuf;
		const int number_of_vertices = 81;
		const int number_of_cylinder_triangles = number_of_vertices * 2 + (number_of_vertices - 1) * 2;

		generate_cylinder(number_of_vertices, &posBuf, &indexBuf);

		unsigned int cylinderPosID;
		unsigned int cylinderNormID;
		unsigned int cylinderIndexID;
		unsigned int cylinderVertexArrayID;

		vector<float> snowPosBuf;
		vector<unsigned int> snowIndexBuf;
		const int number_of_snow_vertices = 11;
		const int number_of_snow_triangles = number_of_snow_vertices - 1;

		generate_circle(number_of_vertices, 0, 1, &snowPosBuf, &snowIndexBuf);

		unsigned int snowPosID;
		unsigned int snowIndexID;
		unsigned int snowVertexArrayID;

		for (int i = 0; i < number_of_snow_vertices * 3 - 1; i++) {
			cout << "snowPosBuf[i]: " << snowPosBuf[i] << endl;
		}

		for (int i = 0; i < number_of_snow_triangles * 3 - 1; i++) {
			cout << "snowIndexBuf[i]: " << snowIndexBuf[i] << endl;
		}



		// generate snowflakes
		glGenVertexArrays(1, &snowVertexArrayID);
		glBindVertexArray(snowVertexArrayID);

		glGenBuffers(1, &snowPosID);
		glBindBuffer(GL_ARRAY_BUFFER, snowPosID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * number_of_vertices * 3, snowPosBuf.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &snowIndexID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, snowIndexID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (number_of_snow_triangles) * 3, snowIndexBuf.data(), GL_DYNAMIC_DRAW);

		glBindVertexArray(0);

		mysnowflake.indexBufferID = snowIndexID;
		mysnowflake.vertexArrayID = snowVertexArrayID;
		mysnowflake.size = sizeof(unsigned int) * (number_of_snow_triangles) * 3 * 2;

		// generate circle object
		glGenVertexArrays(1, &cylinderVertexArrayID);
		glBindVertexArray(cylinderVertexArrayID);

		glGenBuffers(1, &cylinderPosID);
		glBindBuffer(GL_ARRAY_BUFFER, cylinderPosID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * number_of_vertices * 2 * 3, posBuf.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &cylinderNormID);
		glBindBuffer(GL_ARRAY_BUFFER, cylinderNormID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * number_of_vertices * 2 * 3, posBuf.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &cylinderIndexID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderIndexID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (number_of_cylinder_triangles) * 3, indexBuf.data(), GL_DYNAMIC_DRAW);

		glBindVertexArray(0);

		mycylinder.indexBufferID = cylinderIndexID;
		mycylinder.vertexArrayID = cylinderVertexArrayID;
		mycylinder.size = sizeof(unsigned int) * (number_of_cylinder_triangles) * 3;

		

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		glGenBuffers(1, &VertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			// back
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0,
			//tube 8 - 11
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			//12 - 15
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0

			
		};
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		//color
		GLfloat cube_colors[] = {
			// front colors
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			// back colors
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			// tube colors
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
		};
		glGenBuffers(1, &VertexColorIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexColorIDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {
		
			// front
			0, 1, 2,
			2, 3, 0,
			// back
			7, 6, 5,
			5, 4, 7,
			//tube 8-11, 12-15
			8,12,13,
			8,13,9,
			9,13,14,
			9,14,10,
			10,14,15,
			10,15,11,
			11,15,12,
			11,12,8
			
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

		glBindVertexArray(0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Enable blending/transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);

		// Initialize the GLSL program.
		cylProg = std::make_shared<Program>();
		cylProg->setVerbose(true);
		cylProg->setShaderNames(resourceDirectory + "/cylinder_vertex.glsl", resourceDirectory + "/cylinder_fragment.glsl");
		if (!cylProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		cylProg->addUniform("P");
		cylProg->addUniform("V");
		cylProg->addUniform("M");
		cylProg->addAttribute("vertPos");
		mycylinder.shader_prog = cylProg;
		// Initialize the GLSL program.
		snowProg = std::make_shared<Program>();
		snowProg->setVerbose(true);
		snowProg->setShaderNames(resourceDirectory + "/snow_vertex.glsl", resourceDirectory + "/snow_fragment.glsl");
		if (!snowProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		snowProg->addUniform("P");
		snowProg->addUniform("V");
		snowProg->addUniform("M");
		snowProg->addUniform("alpha");
		snowProg->addUniform("time");
		snowProg->addUniform("direction");
		snowProg->addAttribute("vertPos");
		snowProg->addAttribute("vertNor");
		snowProg->addAttribute("vertTex");
		// Initialize the GLSL program.
		shapeprog = std::make_shared<Program>();
		shapeprog->setVerbose(true);
		shapeprog->setShaderNames(resourceDirectory + "/shape_vertex.glsl", resourceDirectory + "/shape_fragment.glsl");
		if (!shapeprog->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
			}
		shapeprog->addUniform("P");
		shapeprog->addUniform("V");
		shapeprog->addUniform("M");
		shapeprog->addAttribute("vertPos");
		shapeprog->addAttribute("vertNor");
		shapeprog->addAttribute("vertTex");

		// Initialize the GLSL program.
		globeProg = std::make_shared<Program>();
		globeProg->setVerbose(true);
		globeProg->setShaderNames(resourceDirectory + "/globe_vertex.glsl", resourceDirectory + "/globe_fragment.glsl");
		if (!globeProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		globeProg->addUniform("P");
		globeProg->addUniform("V");
		globeProg->addUniform("M");
		globeProg->addAttribute("vertPos");
		globeProg->addAttribute("vertNor");
		globeProg->addAttribute("vertTex");
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
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static float t = 0;
		if (mycam.a == 1)
		{
			t -= 0.05f;
		}
		else if (mycam.d == 1)
		{
			t += 0.05f;
		}
		
		mat4 V, M, P; //View, Model and Perspective matrix
		mat4 T, R, S, Ta, Tpor; //Other matrices
		V = mat4(1);
		M = mat4(1);
		// Apply orthographic projection....
		P = perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		mat4 gS = scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)); 
		mat4 gR = rotate(mat4(1.0f), t, vec3(0, 1.0f, 0));

		V = mycam.process(frametime);

		shapeprog->bind();
		T = translate(mat4(1.0f), vec3(0, -0.8f, 5));
		mat4 BottomSphere = mat4(1) * T * gR;
		M = BottomSphere * gS;
		glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);
		shapeprog->unbind();

		shapeprog->bind();
		S = scale(mat4(1.0f), vec3(0.7f, 0.7f, 0.7f));
		T = translate(mat4(1.0f), vec3(0, 0.7f, 0));
		mat4 MiddleSphere = BottomSphere * T;
		M = MiddleSphere * gS * S;
		glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);
		shapeprog->unbind();

		shapeprog->bind();
		S = scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
		T = translate(mat4(1.0f), vec3(0, 0.5f, 0));
		mat4 HeadSphere = MiddleSphere * T;
		M = HeadSphere * gS * S;
		glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);
		shapeprog->unbind();

		shapeprog->bind();
		S = scale(mat4(1.0f), vec3(0.07f, 0.07f, 0.07f));
		T = translate(mat4(1.0f), vec3(-0.1f, 0, 0.25f));
		mat4 RightEyeSphere = HeadSphere * T;
		M = RightEyeSphere * gS * S;
		glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);
		shapeprog->unbind();

		shapeprog->bind();
		S = scale(mat4(1.0f), vec3(0.07f, 0.07f, 0.07f));
		T = translate(mat4(1.0f), vec3(0.1f, 0, 0.25f));
		mat4 LeftEyeSphere = HeadSphere * T;
		M = LeftEyeSphere * gS * S;
		glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);
		shapeprog->unbind();
		
		static float w = 0.0f;
		w += 0.05f;

		T = translate(mat4(1.0f), vec3(-0.25f, 0.10f, 0));
		R = rotate(mat4(1.0f), -float(-M_PI / 2), vec3(0, 1.0f, 0)) * 
			rotate(mat4(1.0f), sin(w) * 0.5f, vec3(1.0f, 1.0f, 1.0f));
		S = scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.5f));
		mat4 RightArm = MiddleSphere * T * R;
		M = RightArm * S;
		mycylinder.setPVM(&P[0][0], &V[0][0], &M[0][0]);
		mycylinder.draw();

		Ta = translate(mat4(1.0f), vec3(0, 0, -0.5f));
		Tpor = translate(mat4(1.0f), vec3(0.0f, 0, 0));
		R = rotate(mat4(1.0f), 0.5f * sin(w * 1.5f), vec3(1.0f, 1.0f, 1.0f));
		S = scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.5f));
		mat4 RightLowerArm = RightArm * Ta * R;
		M = RightLowerArm * S;
		mycylinder.setPVM(&P[0][0], &V[0][0], &M[0][0]);
		mycylinder.draw();

		T = translate(mat4(1.0f), vec3(0.25f, 0.10f, 0));
		R = rotate(mat4(1.0f), float(-M_PI / 2), vec3(0, 1.0f, 0)) * 
			rotate(mat4(1.0f), -sin(w) * 0.5f, vec3(1.0f, 1.0f, 1.0f));
		S = scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.5f));
		mat4 LeftArm = MiddleSphere * T * R;
		M = LeftArm * S;
		mycylinder.setPVM(&P[0][0], &V[0][0], &M[0][0]);
		mycylinder.draw();

		Ta = translate(mat4(1.0f), vec3(0, 0, -0.5f));
		Tpor = translate(mat4(1.0f), vec3(0.0f, 0, 0));
		R = rotate(mat4(1.0f), -0.5f * sin(w * 1.5f), vec3(0.2f, 1.0f, 0));
		S = scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.5f));
		mat4 LeftLowerArm = LeftArm * Ta * R;
		M = LeftLowerArm * S;
		mycylinder.setPVM(&P[0][0], &V[0][0], &M[0][0]);
		mycylinder.draw();

		glFrontFace(GL_CCW);

		globeProg->bind();
		S = scale(mat4(1.0f), vec3(1.5f, 1.5f, 1.5f));
		T = translate(mat4(1.0f), vec3(0, 0, 0));
		mat4 GlobeSphere = MiddleSphere * T * S;
		M = GlobeSphere;
		glUniformMatrix4fv(globeProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(globeProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(globeProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(globeProg);
		globeProg->unbind();

		glFrontFace(GL_CW);

		static float s = 0;
		s += 0.05f;

		for (int n = 0; n < snowflakes.size(); n++)
		{
			snowProg->bind();
			S = scale(mat4(1.0f), vec3(0.05f, 0.05f, 0.05f));
			R = rotate(mat4(1.0f), s, vec3(0.0f, 1.0f, 0.0f));
			T = translate(mat4(1.0f), vec3(snowflakes[n].x, snowflakes[n].y, snowflakes[n].z));
			mat4 Snowflake = HeadSphere * T * R * S;
			M = Snowflake;
			glUniform1f(snowProg->getUniform("alpha"), 0.5f);
			glUniform1f(snowProg->getUniform("time"), float(glfwGetTime()));
			glUniform1f(snowProg->getUniform("direction"), snowflakes[n][3]);
			glUniformMatrix4fv(snowProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(snowProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(snowProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			shape.draw(snowProg);
			snowProg->unbind();

		}

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.

	application->init_snowflakes();
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
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
