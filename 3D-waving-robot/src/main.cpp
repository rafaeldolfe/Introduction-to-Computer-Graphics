/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <math.h>

#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

# define M_PI          3.141592653589793238462643383279502884L /* pi */

using namespace glm;
using namespace std;

glm::mat4 CreateTranslateMatrix(float x, float y, float z)
{
	return mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(x, y, z, 1));
}
glm::mat4 CreateScaleMatrix(float x, float y, float z)
{
	return mat4(vec4(x, 0, 0, 0), vec4(0, y, 0, 0), vec4(0, 0, z, 0), vec4(0, 0, 0, 1));
}
glm::mat4 CreateRotattionMatrixZ(float angle)
{
	return mat4(vec4(cos(angle), -sin(angle), 0, 0), vec4(sin(angle), cos(angle), 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
}

glm::mat4 CreateRotattionMatrixY(float angle)
{
	return mat4(vec4(cos(angle), 0, -sin(angle), 0), vec4(0, 1, 0, 0), vec4(sin(angle), 0, cos(angle), 0), vec4(0, 0, 0, 1));
}

glm::mat4 CreateRotattionMatrixX(float angle)
{
	return mat4(vec4(1, 0, 0, 0), vec4(0, cos(angle), sin(angle), 0), vec4(0, -sin(angle), cos(angle), 0), vec4(0, 0, 0, 1));
}

glm::mat4 MatrixMultiply(glm::mat4 A, glm::mat4 B) 
{
	mat4 r = mat4(0);
	for (int row = 0; row < 4; row++)
	{
		vec4 res;
		for (int col = 0; col < 4; col++)
		{
			vec4 row1 = vec4(A[0][row], A[1][row], A[2][row], A[3][row]);
			vec4 col1 = vec4(B[col][0], B[col][1], B[col][2], B[col][3]);

			r[col][row] = row1[0] * col1[0] + row1[1] * col1[1] + row1[2] * col1[2] + row1[3] * col1[3];
		}
	}
	return r;
}

bool moveTowardsAngle(float current, float target, float* shift)
{
	if (current > target - 0.01 && current < target + 0.01)
	{
		return false;
	}
	else 
	{
		if (current - target < 0)
		{
			*shift += 0.01 * abs(current - target) + 0.01;
		}
		else
		{
			*shift += -0.01 * abs(current - target) - 0.01;
		}
		return true;
	}
}

bool moveTowards(float current, float target, float *shift) 
{
	/*cout << "current: " << current << endl;
	cout << "target: " << target << endl;*/
	if (current > target - 0.03 && current < target + 0.03)
	{	
		return false;
	}
	else {
		if (current - target < 0)
		{
			*shift += 0.05 * abs(current - target) + 0.02;
		}
		else
		{
			*shift += -0.05 * abs(current - target) - 0.02;
		}
		return true;
	}
}

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
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, -1);
	}
	glm::mat4 process()
	{
		double ftime = get_last_elapsed_time();
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
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;
int N;
int M;
int X;
int C;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

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

		if (key == GLFW_KEY_N && action == GLFW_PRESS)
		{
			N = 1;
		}
		if (key == GLFW_KEY_N && action == GLFW_RELEASE)
		{
			N = 0;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			M = 1;
		}
		if (key == GLFW_KEY_M && action == GLFW_RELEASE)
		{
			M = 0;
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS)
		{
			X = 1;
		}
		if (key == GLFW_KEY_X && action == GLFW_RELEASE)
		{
			X = 0;
		}
		if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			C = 1;
		}
		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			C = 0;
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
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
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
		prog->addAttribute("vertPos");
		prog->addAttribute("vertColor");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, Model, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		Model = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		glm::mat4 T = glm::mat4(1.0f);
		glm::mat4 S = CreateScaleMatrix(0.1f, 0.1f, 0.1f);

		
		// Draw the box using GLSL.
		prog->bind();

		V = mycam.process();
		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);

		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		
		glm::mat4 TransX = CreateTranslateMatrix(0, 0, 0);
		mat4 M1 = MatrixMultiply(Model, TransX);
		Model = MatrixMultiply(M1, S);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		mat4 T1Attach = CreateTranslateMatrix(0, 0.15f, 0);
		mat4 R1 = CreateRotattionMatrixY(float(M_PI/4));
		mat4 S1 = CreateScaleMatrix(0.5f, 0.5f, 0.5f);
		mat4 M2 = MatrixMultiply(M1, MatrixMultiply(T1Attach, R1));
		Model = MatrixMultiply(M2, MatrixMultiply(S1, S));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		static float arm1 = 0;
		static float arm2 = 0;
		static float innerM = 3;
		static float outerM = 0.5f;

		static float offset1 = 0;
		static float offset2 = 0;

		static float animInnerM = 1;
		static float animOuterM = 1;
		static float animOffset = 0;
		static float animArm1 = 0;

		static float waveAnim = 0;

		float current = sin(arm1 * innerM) * outerM + offset1;

		cout << "arm1: " << arm1 << endl;

		if (X == 1)
		{
				float shift = 0;

				bool checks[4] = {
					moveTowards(offset1, 0.5f, &shift)
				};

				if (!checks[0])
				{
					cout << "waving now!" << endl;
					offset1 = 0.5f;
					arm1 += 0.05;
					
					arm1 = fmod(arm1, 2 * M_PI);
				}
				else
				{
					offset1 += shift;
				}
			
		}
		if (N == 1 && offset2 < 1)
		{
			offset2 += 0.05;
		}
		if (N == 1 && offset1 > -1)
		{
			offset1 += -0.05;
		}
		if (M == 1 && offset2 > -1)
		{
			offset2 += -0.05;
		}
		if (M == 1 && offset1 < 1)
		{
			offset1 += 0.05;
		}

		mat4 TArm1Attach = CreateTranslateMatrix(-0.10f, 0.09f, 0);
		mat4 TArm1PointOfRot = CreateTranslateMatrix(-0.09f, 0.0f, 0);
		mat4 RArm1 = CreateRotattionMatrixZ(offset1);
		mat4 SArm1 = CreateScaleMatrix(1, 0.09f, 0.09f);
		mat4 MArm1 = MatrixMultiply(M1, MatrixMultiply(TArm1Attach, MatrixMultiply(RArm1, TArm1PointOfRot)));
		Model = MatrixMultiply(MArm1, MatrixMultiply(SArm1, S));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		mat4 TArm1Arm2Attach = CreateTranslateMatrix(-0.10f, 0, 0);
		mat4 TArm1Arm2PointOfRot = CreateTranslateMatrix(-0.09f, 0.0f, 0);
		mat4 RArm1Arm2 = CreateRotattionMatrixZ(sin(arm1 * innerM) * outerM + offset1);//offset1);
		mat4 SArm1Arm2 = CreateScaleMatrix(1, 0.09f, 0.09f);
		mat4 MArm1Arm2 = MatrixMultiply(MArm1, MatrixMultiply(TArm1Arm2Attach, MatrixMultiply(RArm1Arm2, TArm1Arm2PointOfRot)));
		Model = MatrixMultiply(MArm1Arm2, MatrixMultiply(SArm1, S));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		mat4 TArm2Attach = CreateTranslateMatrix(0.10f, 0.09f, 0);
		mat4 TArm2PointOfRot = CreateTranslateMatrix(0.09f, 0.0f, 0);
		mat4 RArm2 = CreateRotattionMatrixZ(offset2);
		mat4 SArm2 = CreateScaleMatrix(1, 0.09f, 0.09f);
		mat4 MArm2 = MatrixMultiply(M1, MatrixMultiply(TArm2Attach, MatrixMultiply(RArm2, TArm2PointOfRot)));
		Model = MatrixMultiply(MArm2, MatrixMultiply(SArm2, S));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		mat4 TArm2Arm2Attach = CreateTranslateMatrix(0.10f, 0.0f, 0);
		mat4 TArm2Arm2PointOfRot = CreateTranslateMatrix(0.09f, 0.0f, 0);
		mat4 RArm2Arm2 = CreateRotattionMatrixZ(sin(arm2 * innerM) * outerM * -1 + offset2);
		mat4 SArm2Arm2 = CreateScaleMatrix(1, 0.09f, 0.09f);
		mat4 MArm2Arm2 = MatrixMultiply(MArm2, MatrixMultiply(TArm2Arm2Attach, MatrixMultiply(RArm2Arm2, TArm2Arm2PointOfRot)));
		Model = MatrixMultiply(MArm2Arm2, MatrixMultiply(SArm2, S));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &Model[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
				
		glBindVertexArray(0);

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
