/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

# define M_PI          3.141592653589793238462643383279502884L /* pi */

using namespace std;
using namespace glm;
shared_ptr<Shape> shape;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = vec3(2.2f, 0.0f, 0.0f);
	}
	mat4 process(double ftime, float preRotation)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 1*ftime;
		}
		else if (s == 1)
		{
			speed = -1*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		mat4 R = rotate(mat4(1), rot.y + preRotation, vec3(0, 1, 0));
		vec4 dir = vec4(0, 0, speed,1);
		dir = dir*R;
		pos += vec3(dir.x, dir.y, dir.z);
		mat4 T = translate(mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, greenProg;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;

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

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addUniform("baseColor");
		prog->addUniform("specPowerFactor");
		prog->addUniform("specFactor");
		prog->addUniform("diffuseFactor");
		prog->addUniform("globalLight");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		// Initialize the GLSL program.
		greenProg = std::make_shared<Program>();
		greenProg->setVerbose(true);
		greenProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		greenProg->init();
		greenProg->addUniform("P");
		greenProg->addUniform("V");
		greenProg->addUniform("M");
		greenProg->addUniform("campos");
		greenProg->addAttribute("vertPos");
		greenProg->addAttribute("vertNor");
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

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		mat4 V, M, P; //View, Model and Perspective matrix
		V = mat4(1);
		M = mat4(1);
		// Apply orthographic projection....
		P = ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;
		//w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		mat4 RotateY = rotate(mat4(1.0f), w, vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926/2.0;
		mat4 RotateX = rotate(mat4(1.0f), angle, vec3(1.0f, 0.0f, 0.0f));
		mat4 TransZ = translate(mat4(1.0f), vec3(0.0f, 0.0f, -3 + trans));
		mat4 S = scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));

		M = TransZ * RotateY * RotateX * S;

		V = mycam.process(frametime, float(M_PI/4));

		vec3 campos = -mycam.pos;


		// Draw the box using GLSL.
		prog->bind();
		//send the matrices to the shaders
		vec3 red = vec3(1.0f, 0, 0);
		mat4 redM = M;
		M = redM;
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);
		glUniform3fv(prog->getUniform("baseColor"), 1, &red[0]);
		glUniform3fv(prog->getUniform("globalLight"), 1, &vec3(0.0f)[0]);
		glUniform1f(prog->getUniform("specPowerFactor"), 5.0f);
		glUniform1f(prog->getUniform("specFactor"), 3.0f);
		glUniform1f(prog->getUniform("diffuseFactor"), 0.7f);
		glUniform1f(prog->getUniform("globalLight"), 0.1f);

		shape->draw(prog);
		
		prog->unbind();

		prog->bind();
		//send the matrices to the shaders
		vec3 green = vec3(0, 1.0f, 0);
		mat4 T = translate(mat4(1.0f), vec3(-2.5, 0, 0));
		mat4 greenM = redM * T;
		M = greenM;
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);
		glUniform3fv(prog->getUniform("baseColor"), 1, &green[0]);
		glUniform3fv(prog->getUniform("globalLight"), 1, &vec3(0.1f)[0]);
		glUniform1f(prog->getUniform("specPowerFactor"), 5.0f);
		glUniform1f(prog->getUniform("specFactor"), 2.0f);
		glUniform1f(prog->getUniform("diffuseFactor"), 0.6f);

		shape->draw(prog);

		prog->unbind();

		prog->bind();
		//send the matrices to the shaders
		vec3 blue = vec3(0, 0.0f, 1.0f);
		T = translate(mat4(1.0f), vec3(2.5, 0, 0));
		mat4 blueM = redM * T;
		M = blueM;
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);
		glUniform3fv(prog->getUniform("baseColor"), 1, &blue[0]);
		glUniform3fv(prog->getUniform("globalLight"), 1, &vec3(0.2f)[0]);
		glUniform1f(prog->getUniform("specPowerFactor"), 4.0f);
		glUniform1f(prog->getUniform("specFactor"), 2.0f);
		glUniform1f(prog->getUniform("diffuseFactor"), 0.4f);

		shape->draw(prog);

		prog->unbind();

		prog->bind();
		//send the matrices to the shaders
		vec3 maroon = vec3(0.69, 0.19, 0.38);
		T = translate(mat4(1.0f), vec3(5, 0, 0));
		mat4 cyanM = redM * T;
		M = cyanM;
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);
		glUniform3fv(prog->getUniform("baseColor"), 1, &maroon[0]);
		glUniform3fv(prog->getUniform("globalLight"), 1, &vec3(0.08f)[0]);
		glUniform1f(prog->getUniform("specPowerFactor"), 1.0f);
		glUniform1f(prog->getUniform("specFactor"), 0.2f);
		glUniform1f(prog->getUniform("diffuseFactor"), 0.7f);

		shape->draw(prog);

		prog->unbind();

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
