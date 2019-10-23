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

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

# define M_PI          3.141592653589793238462643383279502884L /* pi */

using namespace glm;
using namespace std;

// This number must be even, or else one adjacent pair of vertices will not be moving according to the pattern
const int number_of_vertices = 81;
const int number_of_triangles = number_of_vertices - 1;

const GLuint vertex_buffer_size = number_of_vertices * 3;
const GLuint index_buffer_size = number_of_triangles * 3;
const GLuint index_attri_size = number_of_vertices;
const GLuint direction_attri_size = number_of_vertices * 2;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
	// Our shader program
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow* window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	std::shared_ptr<Program> sun_prog;
	std::shared_ptr<Program> bg_prog;
	GLuint sun_vertex_array_id, bg_vertex_array_id;
	GLuint vertex_buffer_id, index_buffer_id, index_attri_id, direction_attri_id;
	GLuint bg_vertex_buffer_id, bg_index_buffer_id;
	void initGeom(float vertex_buffer[], unsigned int index_buffer[], unsigned int index_attri[], float direction_attri[])
	{
		glGenVertexArrays(1, &sun_vertex_array_id);
		glBindVertexArray(sun_vertex_array_id);

		// 0: vertPos attribute
		glGenBuffers(1, &vertex_buffer_id);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
		
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size * sizeof(float), vertex_buffer, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		// 1: index buffer
		glGenBuffers(1, &index_buffer_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size * sizeof(unsigned int), index_buffer, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 0, (void*)0);

		// 5: index attribute
		glGenBuffers(1, &index_attri_id);
		glBindBuffer(GL_ARRAY_BUFFER, index_attri_id);

		glBufferData(GL_ARRAY_BUFFER, index_attri_size * sizeof(unsigned int), index_attri, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (void*)0);

		// 6: direction attribute
		glGenBuffers(1, &direction_attri_id);
		glBindBuffer(GL_ARRAY_BUFFER, direction_attri_id);

		glBufferData(GL_ARRAY_BUFFER, direction_attri_size * sizeof(float), direction_attri, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);

		glGenVertexArrays(1, &bg_vertex_array_id);
		glBindVertexArray(bg_vertex_array_id);

		// 0: background vertPos attribute
		glGenBuffers(1, &bg_vertex_buffer_id);
		glBindBuffer(GL_ARRAY_BUFFER, bg_vertex_buffer_id);

		static const GLfloat bg_vertex_buffer[] =
		{
			1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,

			1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(bg_vertex_buffer), bg_vertex_buffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 1: background index buffer
		glGenBuffers(1, &bg_index_buffer_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_index_buffer_id);

		static const GLuint bg_index_buffer[] =
		{
			0, 1, 2,
			3, 4, 5,
		};

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bg_index_buffer), bg_index_buffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 0, (void*)0);

	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_DEBUG_OUTPUT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		// Enable z-buffer test.
		//glEnable(GL_DEPTH_TEST);

		// Initialize the sun GLSL program.
		sun_prog = std::make_shared<Program>();
		sun_prog->setVerbose(true);
		sun_prog->setShaderNames(resourceDirectory + "/sun_shader_vertex.glsl", resourceDirectory + "/sun_shader_fragment.glsl");
		sun_prog->init();
		sun_prog->addAttribute("vertPos");
		sun_prog->addAttribute("index");
		sun_prog->addAttribute("direction");
		sun_prog->addUniform("speed");

		// Initialize the background GLSL program.
		bg_prog = std::make_shared<Program>();
		bg_prog->setVerbose(true);
		bg_prog->setShaderNames(resourceDirectory + "/bg_shader_vertex.glsl", resourceDirectory + "/bg_shader_fragment.glsl");
		bg_prog->init();
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

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT);

		bg_prog->bind();

		glBindVertexArray(bg_vertex_array_id);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_index_buffer_id);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		bg_prog->unbind();

		sun_prog->bind();

		unsigned int remainder = 7;

		float bound = fmod(glfwGetTime(), remainder * 2);
		if (bound > remainder)
			bound = remainder - fmod(glfwGetTime(), remainder);
		cout << "bound: " << bound << endl;

		glUniform1f(sun_prog->getUniform("speed"), 0.0000000001f * bound * -1);

		glBindVertexArray(sun_vertex_array_id);

		//actually draw from vertex 0, 3 vertices
		//glDrawArrays(GL_TRIANGLES, 0, 15);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);

		glDrawElements(GL_TRIANGLES, index_buffer_size, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		sun_prog->unbind();

	}

};

void get_xy(float radian, float(&point)[2])
{
	point[0] = cos(radian);
	point[1] = sin(radian);
}
unsigned int index_3(unsigned int i)
{
	return i * 3;
}
unsigned int index_2(unsigned int i)
{
	return i * 2;
}
void add_vertex(int i, float x, float y, float z, float posBuf[])
{
	posBuf[i] = x;
	posBuf[i + 1] = y;
	posBuf[i + 2] = z;
}
void add_triangle(unsigned int index, unsigned int i, unsigned int j, unsigned int k, unsigned int triBuf[])
{
	triBuf[index] = i;
	triBuf[index + 1] = j;
	triBuf[index + 2] = k;
}
void add_direction(int i, float x, float y, float posBuf[])
{
	posBuf[i] = x;
	posBuf[i + 1] = y;
}
void generate_circle(float posBuf[], unsigned int triBuf[], unsigned int indexAttri[], float directionAttri[])
{
	float angle_between_vertices = 2 * M_PI / (number_of_vertices - 1);

	add_vertex(index_3(number_of_vertices - 1), 0, 0, 0, posBuf);
	indexAttri[number_of_vertices - 1] = 0;
	add_direction(index_2(number_of_vertices - 1), 0, 0, directionAttri);

	float r = 0;
	float xy[2];
	for (int i = 0; i < number_of_triangles; i += 1)
	{
		get_xy(r, xy);
		add_vertex(index_3(i), xy[0], xy[1], 0, posBuf);
		indexAttri[i] = i % 2;
		add_direction(index_2(i), xy[0], xy[1], directionAttri);
		add_triangle(index_3(i), i, (i + 1) % (number_of_vertices - 1), number_of_vertices - 1, triBuf);

		r += angle_between_vertices;
	}
}

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
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.


	float posBuf[number_of_vertices * 3];
	unsigned int triBuf[number_of_triangles * 3];
	unsigned int indexAttri[number_of_vertices];
	float directionAttri[number_of_vertices * 2];

	generate_circle(posBuf, triBuf, indexAttri, directionAttri);

	application->init(resourceDir);
	application->initGeom(posBuf, triBuf, indexAttri, directionAttri);

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
