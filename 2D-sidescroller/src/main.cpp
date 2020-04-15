/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <algorithm>
#include <vector>

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GRAVITY_PULL 0.6f
#define FRICTION 0.3f
#define M_PI 3.14f

using namespace std;
using namespace glm;
shared_ptr<Shape> shape;

enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT,
	Z_IN,
	Z_OUT,
	STILL
};

typedef std::tuple<GLboolean, Direction> Collision;

float Linear(float v0, float v1, float t)
{
	return v0 + t * (v1 - v0);
}
float SmoothStep(float v0, float v1, float t)
{
	return Linear(v0, v1, pow(t, 2) * (3 - 2 * t));
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
	vec3 pos, rot, prev;
	float time;
	bool transitioning = false;
	int t, f, g, h;
	camera()
	{
		t = f = g = h = 0;
		pos = glm::vec3(0, 0, 0);
		rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double frametime)
	{
		double ftime = frametime;
		float speed = 0;
		if (t == 1)
		{
			speed = 10*ftime;
		}
		else if (g == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (f == 1)
			yangle = -1*ftime;
		else if(h==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 Ry = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*Ry*Rx;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return Ry*Rx*T;
	}
};

class gameObject {
public:
	vec3 bottomleft, size, velocity, xyz;
	vec3 color;
};

class Player : public gameObject {
public:
	void Move(vec3 vec)
	{
		bottomleft = bottomleft + vec;
		xyz = xyz + vec;
	}
};

camera mycam;
Player player;
vector<gameObject> nonPlayerObjects;
vector<gameObject> deadlyObjects;
vector<gameObject> winObjects;

class Application : public EventCallbacks
{

public:
	int kn = 0;
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, pwin, psky, shapeprog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID, VertexArrayIDGameWin;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;
	GLuint VertexBufferIDGameWin, VertexColorIDBoxGameWin, IndexBufferIDBoxGameWin, VertexTexBoxGameWin;

	GLuint Texture, Texture2;

	int a, w, s, d, space;
	bool gameWon = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_T && action == GLFW_PRESS)
		{
			mycam.t = 1;
		}
		if (key == GLFW_KEY_T && action == GLFW_RELEASE)
		{
			mycam.t = 0;
		}
		if (key == GLFW_KEY_G && action == GLFW_PRESS)
		{
			mycam.g = 1;
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE)
		{
			mycam.g = 0;
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			mycam.f = 1;
		}
		if (key == GLFW_KEY_F && action == GLFW_RELEASE)
		{
			mycam.f = 0;
		}
		if (key == GLFW_KEY_H && action == GLFW_PRESS)
		{
			mycam.h = 1;
		}
		if (key == GLFW_KEY_H && action == GLFW_RELEASE)
		{
			mycam.h = 0;
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			d = 0;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			space = 1;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			space = 0;
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

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{

		string resourceDirectory = "../resources";
		// Initialize mesh.
		shape = make_shared<Shape>();
		//shape->loadMesh(resourceDirectory + "/t800.obj");
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();


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

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayIDGameWin);
		glBindVertexArray(VertexArrayIDGameWin);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDGameWin);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDGameWin);

		GLfloat billboard_vertices[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
		};
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(billboard_vertices), billboard_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat billboard_colors[] = {
			// front colors
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
		};
		glGenBuffers(1, &VertexColorIDBoxGameWin);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexColorIDBoxGameWin);
		glBufferData(GL_ARRAY_BUFFER, sizeof(billboard_colors), billboard_colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//texcoords
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBoxGameWin);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBoxGameWin);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBoxGameWin);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxGameWin);
		GLushort billboard_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,

		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(billboard_elements), billboard_elements, GL_STATIC_DRAW);



		glBindVertexArray(0);

		int width, height, channels;
		char filepath[1000];

		string str = resourceDirectory + "/gamewin.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/nebula.jpeg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint Tex1Location = glGetUniformLocation(pwin->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(pwin->pid, "tex2");//tex, tex2... sampler in the fragment shader
		////// Then bind the uniform samplers to texture units:
		glUseProgram(pwin->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Enable blending/transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
			}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("colour");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertColor");

		pwin = std::make_shared<Program>();
		pwin->setVerbose(true);
		pwin->setShaderNames(resourceDirectory + "/win_vertex.glsl", resourceDirectory + "/win_fragment.glsl");
		if (!pwin->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		pwin->addUniform("P");
		pwin->addUniform("V");
		pwin->addUniform("M");
		pwin->addUniform("colour");
		pwin->addAttribute("vertPos");
		pwin->addAttribute("vertColor");
		pwin->addAttribute("vertTex");

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

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
	}

	void CreatePlayer(vec3 pos, vec3 size, vec3 color)
	{
		player.xyz = pos;
		player.size = size;
		player.bottomleft = player.xyz - player.size / 2.0f;
		player.velocity = vec3(0, 0, 0);
		player.color = color;
	}

	void CreateObject(vec3 pos, vec3 size, vec3 color)
	{
		gameObject go;
		go.xyz = pos;
		go.size = size;
		go.bottomleft = go.xyz - go.size / 2.0f;
		go.velocity = vec3(0, 0, 0);
		go.color = color;
		nonPlayerObjects.push_back(go);
	}

	void CreateDeadly(vec3 pos, vec3 size, vec3 color)
	{
		gameObject go;
		go.xyz = pos;
		go.size = size;
		go.bottomleft = go.xyz - go.size / 2.0f;
		go.velocity = vec3(0, 0, 0);
		go.color = color;
		deadlyObjects.push_back(go);
	}
	
	void CreateWinZone(vec3 pos, vec3 size, vec3 color)
	{
		gameObject go;
		go.xyz = pos;
		go.size = size;
		go.bottomleft = go.xyz - go.size / 2.0f;
		go.velocity = vec3(0, 0, 0);
		go.color = color;
		winObjects.push_back(go);
	}

	double startTime;

	double GetGameTime()
	{
		return (glfwGetTime() - startTime);
	}

	float startPoint = 0;
	bool debug = false;
	void InitGame()
	{
		if (debug)
		{
			startPoint = 100;
		}
		mycam.pos = glm::vec3(-startPoint, -13, -30);
		startTime = glfwGetTime();
		float mapLength = 2000;

		CreatePlayer(vec3(startPoint, 2, 3), vec3(1, 1, 1), vec3(0.8f, 0.8f, 0.8f));

		CreateObject(vec3(0, 0, 0), vec3(mapLength, 1, 10), vec3(0));
		CreateObject(vec3(0, 0.6f, 5), vec3(mapLength, 0.4f, 0.2f), vec3(0, 0, 0.3f));
		CreateObject(vec3(0, 0.6f, -5), vec3(mapLength, 0.4f, 0.2f), vec3(0, 0, 0.3f));
		CreateObject(vec3(5, 1.5f, 1), vec3(2, 2, 6), vec3(0.5f, 0, 0.125f));

		CreateDeadly(vec3(45, 0.30f, 0.3f), vec3(64, 0.5f, 3), vec3(0, 0.4f, 0));
		CreateObject(vec3(20, 2, -3.3), vec3(3, 4, 3.7f), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(28, 2, 3.8f), vec3(3, 4, 2.8f), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(36, 2, -3.3), vec3(3, 4, 3.7f), vec3(0.5f, 0, 0.125f));


		CreateObject(vec3(48, 2, 3.8f), vec3(3, 4, 2.0f), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(48, 2, -3.3), vec3(3, 4, 3.0f), vec3(0.5f, 0, 0.125f));

		CreateObject(vec3(56, 2, -3.3), vec3(3, 4, 3.7f), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(64, 2, 3.8f), vec3(3, 4, 2.8f), vec3(0.5f, 0, 0.125f));

		CreateObject(vec3(80, 1.5f, 0), vec3(2, 2, 7), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(82, 1.5f, 0), vec3(2, 4, 7), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(84, 2.2f, -0.3f), vec3(2, 4.5f, 10), vec3(0.5f, 0, 0.125f));

		CreateDeadly(vec3(90, 0.30f, -0.5f), vec3(13, 0.5f, 9.5f), vec3(0, 0.4f, 0));

		CreateObject(vec3(104, 1.5f, 0), vec3(2, 2, 7), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(106, 1.5f, 0), vec3(2, 4, 7), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(108, 2.2f, -0.3f), vec3(2, 4.5f, 10), vec3(0.5f, 0, 0.125f));

		CreateDeadly(vec3(120, 0.30f, -0.5f), vec3(25, 0.5f, 9.5f), vec3(0, 0.4f, 0));

		CreateObject(vec3(153, 4, -0.3f), vec3(2, 8.0f, 12), vec3(0.5f, 0, 0.125f));

		CreateObject(vec3(135, 1.5f, 1), vec3(2, 2, 2), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(137, 3, 0), vec3(2, 2, 2), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(133, 4.5f, 0), vec3(2, 2, 2), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(139, 6, 2), vec3(2, 2, 2), vec3(0.5f, 0, 0.125f));
		CreateObject(vec3(145, 6, 1), vec3(4, 2, 2), vec3(0.5f, 0, 0.125f));

		CreateWinZone(vec3(166, 0.30f, -0.5f), vec3(17, 0.5f, 9.5f), vec3(1.0f, 1.0f, 1.0f));
	}

	void ResetGame()
	{
		nonPlayerObjects.clear();
		deadlyObjects.clear();
	}

	void MoveCharacter()
	{
		if (player.bottomleft.y < -20)
		{
			GameOver();
		}

		if (player.bottomleft.x > 70 && player.bottomleft.x < 72)
		{
			startPoint = 70;
		}

		if (player.bottomleft.x > 99 && player.bottomleft.x < 102)
		{
			startPoint = 100;
		}

		double ftime = get_last_elapsed_time();
		float speed = 0.7f;
		float maxSpeed = 0.2f;
		if (player.velocity.x > FRICTION* ftime)
		{
			player.velocity.x = player.velocity.x - FRICTION * ftime;
			if (player.velocity.x < FRICTION * ftime)
			{
				player.velocity.x = 0;
			}
		}
		else if (player.velocity.x < -FRICTION * ftime)
		{
			player.velocity.x = player.velocity.x + FRICTION * ftime;
			if (player.velocity.x > -FRICTION * ftime)
			{
				player.velocity.x = 0;
			}
		}

		if (player.velocity.z > FRICTION* ftime)
		{
			player.velocity.z = player.velocity.z - FRICTION * ftime;
			if (player.velocity.z < FRICTION * ftime)
			{
				player.velocity.z = 0;
			}
		}
		else if (player.velocity.z < -FRICTION * ftime)
		{
			player.velocity.z = player.velocity.z + FRICTION * ftime;
			if (player.velocity.z > -FRICTION * ftime)
			{
				player.velocity.z = 0;
			}
		}



		if (a == 1)
		{
			if (player.velocity.x < maxSpeed)
			{
				player.velocity.x = player.velocity.x - speed * ftime;
			}
		}
		else if (d == 1)
		{
			if (player.velocity.x < maxSpeed)
			{
				player.velocity.x = player.velocity.x + speed * ftime;
			}
		}
		if (w == 1)
		{
			if (player.velocity.z < maxSpeed)
			{
				player.velocity.z = player.velocity.z - speed * ftime;
			}
		}
		else if (s == 1)
		{
			if (player.velocity.z < maxSpeed)
			{
				player.velocity.z = player.velocity.z + speed * ftime;
			}
		}
		if (space == 1 && player.velocity.y == 0)
		{
			player.velocity.y = 0.22f;
		}
		player.velocity.y = player.velocity.y - (GRAVITY_PULL * ftime);

		player.Move(vec3(player.velocity.x, 0, 0));
		Collision c = CheckCollisions(vec3(player.velocity.x, 0, 0));

		if (get<0>(c))
		{
			if (get<1>(c) == LEFT || get<1>(c) == RIGHT)
			{
				player.Move(vec3(-player.velocity.x, 0, 0));
				player.velocity.x = 0;
			}
		}
		player.Move(vec3(0, player.velocity.y, 0));
		c = CheckCollisions(vec3(0, player.velocity.y, 0));
		if (get<0>(c))
		{
			if (get<1>(c) == UP || get<1>(c) == DOWN)
			{
				player.Move(vec3(0, -player.velocity.y, 0));
				player.velocity.y = 0;
			}
		}
		player.Move(vec3(0, 0, player.velocity.z));
		c = CheckCollisions(vec3(0, 0, player.velocity.z));
		if (get<0>(c))
		{
			if (get<1>(c) == Z_IN || get<1>(c) == Z_OUT)
			{
				player.Move(vec3(0, 0, -player.velocity.z));
				player.velocity.z = 0;
			}
		}
	}

	Collision CheckCollisions(vec3 velocity)
	{
		for (gameObject g : winObjects)
		{
			Collision c = CheckCollision(player, g, velocity);
			if (get<0>(c))
			{
				WinGame();
			}
		}
		for (gameObject g : deadlyObjects)
		{
			Collision c = CheckCollision(player, g, velocity);
			if (get<0>(c))
			{
				GameOver();
			}
		}
		for (gameObject g : nonPlayerObjects)
		{
			Collision c = CheckCollision(player, g, velocity);
			if (get<0>(c))
			{
				return c;
			}
		}
		return std::make_tuple(GL_FALSE, UP);
	}

	Collision CheckCollision(gameObject& dynamic, gameObject& terrain, vec3 velocity) // AABB - AABB collision
	{

		bool collisionX = (dynamic.bottomleft.x < (terrain.bottomleft.x + terrain.size.x)) &&
			((dynamic.bottomleft.x + dynamic.size.x) > (terrain.bottomleft.x));
		bool collisionY = (dynamic.bottomleft.y < (terrain.bottomleft.y + terrain.size.y)) &&
			((dynamic.bottomleft.y + dynamic.size.y) > (terrain.bottomleft.y));
		bool collisionZ = (dynamic.bottomleft.z < (terrain.bottomleft.z + terrain.size.z)) &&
			((dynamic.bottomleft.z + dynamic.size.z) > (terrain.bottomleft.z));

		if (collisionX && collisionY && collisionZ)
		{
			return std::make_tuple(GL_TRUE, CollisionDirection(velocity));
		}
		return std::make_tuple(GL_FALSE, UP);
	}

	Direction CollisionDirection(glm::vec3 target)
	{
		glm::vec3 compass[] = {
			glm::vec3(0.0f, 1.0f, 0.0f),	// up
			glm::vec3(1.0f, 0.0f, 0.0f),	// right
			glm::vec3(0.0f, -1.0f, 0.0f),	// down
			glm::vec3(-1.0f, 0.0f, 0.0f),	// left
			glm::vec3(0.0f, 0.0f, 1.0f),	// in
			glm::vec3(0.0f, 0.0f, -1.0f)	// out
		};
		GLfloat max = 0.0f;
		GLuint best_match = -1;
		for (GLuint i = 0; i < 6; i++)
		{
			GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
			if (dot_product > max)
			{
				max = dot_product;
				best_match = i;
			}
		}
		return (Direction)best_match;
	}

	void MoveCamera()
	{

		if (player.xyz.x + 24 < -mycam.pos.x)
		{
			GameOver();
		}

		float speed = GetGameTime() / 100 + 0.5f;
		if (player.xyz.x - 10 > -mycam.pos.x)
		{
			//cout << "(player.xyz.x - 10 > -mycam.pos.x) == true: (time) " << GetGameTime() << endl;
			mycam.pos = mycam.pos - vec3(0.20f, 0, 0) * speed;
		}
		else if (player.xyz.x + 10 < -mycam.pos.x)
		{
			mycam.pos = mycam.pos - vec3(0.02f, 0, 0) * speed;
		}
		else if (player.xyz.x > -mycam.pos.x)
		{
			mycam.pos = mycam.pos - vec3(0.15f, 0, 0) * speed;
		}
		else
		{
			mycam.pos = mycam.pos - vec3(0.05f, 0, 0) * speed;
		}

		if (player.bottomleft.y - 5 > -mycam.pos.y)
		{
			if (!mycam.transitioning)
			{
				mycam.prev = mycam.pos;
				mycam.time = glfwGetTime();
			}
			SmoothStep(-mycam.prev.y, -mycam.pos.y - 10, glfwGetTime() - mycam.time);
		}
	}

	void GameOver()
	{
		ResetGame();
		InitGame();
	}

	void WinGame()
	{
		gameWon = true;
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{

		//INIT RENDER
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 V, M, P;
		V = glm::mat4(1);
		M = glm::mat4(1);
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f);

		if (gameWon)
		{
			pwin->bind();
			glUniformMatrix4fv(pwin->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pwin->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniform3fv(pwin->getUniform("colour"), 1, &vec3(0.8f)[0]);

			glBindVertexArray(VertexArrayIDGameWin);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxGameWin);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture);

			mat4 T = translate(mat4(1), vec3(0, 0, -3));
			mat4 S = glm::scale(glm::mat4(1.0f), vec3(1.9f, 1, 1));
			M = T * S;
			glUniformMatrix4fv(pwin->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
			pwin->unbind();
			return;

		}

		MoveCharacter();
		MoveCamera();


		mycam.rot = vec3(M_PI / 8, 0, 0);
		V = mycam.process(get_last_elapsed_time());
		//V = V * rotate(mat4(1), M_PI / 8, vec3(1, 0, 0));

		vec3 campos = -mycam.pos;
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 10));

		M = TransSky * RotateXSky * SSky;

		psky->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &campos[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, FALSE);
		glEnable(GL_DEPTH_TEST);

		psky->unbind();

		prog->bind();

		for (gameObject g : nonPlayerObjects)
		{
			

			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniform3fv(prog->getUniform("colour"), 1, &g.color[0]);
			glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);

			glBindVertexArray(VertexArrayID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);

			mat4 pT = translate(mat4(1), g.xyz);
			mat4 S = scale(mat4(1), g.size / 2.0f);
			M = pT * S;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		}

		for (gameObject g : deadlyObjects)
		{
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniform3fv(prog->getUniform("colour"), 1, &g.color[0]);
			glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);

			glBindVertexArray(VertexArrayID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);

			mat4 pT = translate(mat4(1), g.xyz);
			mat4 S = scale(mat4(1), g.size / 2.0f);
			M = pT * S;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		}

		for (gameObject g : winObjects)
		{
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniform3fv(prog->getUniform("colour"), 1, &g.color[0]);
			glUniform3fv(prog->getUniform("campos"), 1, &campos[0]);

			glBindVertexArray(VertexArrayID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);

			mat4 pT = translate(mat4(1), g.xyz);
			mat4 S = scale(mat4(1), g.size / 2.0f);
			M = pT * S;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		}


		static float w;
		w += 0.001f;

		mat4 T = glm::translate(glm::mat4(1.0f), player.xyz);
		mat4 S = glm::scale(glm::mat4(1.0f), player.size / 2.0f);

		

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		M = T * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("colour"), 1, &player.color[0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		/*T = glm::translate(glm::mat4(1.0f), player.bottomleft);
		S = glm::scale(glm::mat4(1.0f), player.size);



		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		M = T * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		T = glm::translate(glm::mat4(1.0f), player.bottomleft + player.size + w);
		S = glm::scale(glm::mat4(1.0f), player.size);



		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		M = T * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);*/
		
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
	application->InitGame();

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
