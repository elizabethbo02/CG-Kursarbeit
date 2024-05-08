//#include "stdafx.h"
//#define GLEW_STATIC
#include <GL\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <gl\GLU.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "GeometryNode.h"
#include "GroupNode.h"
#include "TransformNode.h"
#include "stb_image.h"

#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

bool init();
bool initGL();
void render();

void close();
void CreateScene();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

Shader gShader;
Model gModel;

GLuint gVAO, gVBO, gEBO;

GroupNode* gRoot;

bool sceneIsLoaded;

// camera
Camera camera(glm::vec3(2.0f, 4.0f, 14.0f));
float lastX = -1;
float lastY = -1;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//statics
unsigned int Node::genID;
glm::mat4 TransformNode::transformMatrix = glm::mat4(1.0f);

glm::vec3 deCasteljau(std::vector<glm::vec3> points, float t_forward);

//event handlers
void HandleKeyDown(const SDL_KeyboardEvent& key);
void HandleMouseMotion(const SDL_MouseMotionEvent& motion);
void HandleMouseWheel(const SDL_MouseWheelEvent& wheel);

Uint32 startTime;

//Initializes the nessesary variables for the animation
TransformNode* trYellow;
TransformNode* trGolden;

float t_forward = 0.0f;
float t_back = 0.0f;
float speedFactor = 1.0f;
float lastFrameTime = SDL_GetTicks();

//The forward animation control points
std::vector<glm:: vec3> animationYellow = {
glm:: vec3(30.0f, 5.0f, -18.0f) ,
glm::vec3(16.0f, 6.0f, -18.0f),
glm::vec3(9.0f, 4.0f, -18.0f),
glm::vec3(0.0f, 6.0f, -18.0f),
glm::vec3(-20.0f, 3.0f, -18.0f),
};

//The backward animation control points
std::vector<glm::vec3> animationYellowBackPoints = {
glm::vec3(-20.0f, 3.0f, -18.0f),
glm::vec3(0.0f, 6.0f, -18.0f),
glm::vec3(9.0f, 4.0f, -18.0f),
glm::vec3(16.0f, 6.0f, -18.0f),
glm::vec3(30.0f, 5.0f, -18.0f),
};

std::vector<glm::vec3> animationGolden = {
glm::vec3(-30.0f, 2.0f, -10.0f) ,
glm::vec3(-16.0f, 6.0f, -10.0f),
glm::vec3(-9.0f, 0.0f, -10.0f),
glm::vec3(0.0f, 4.0f, -10.0f),
glm::vec3(20.0f, 0.0f, -10.0f),
};

std::vector<glm::vec3> animationGoldenBackPoints = {
glm::vec3(20.0f, 0.0f, -10.0f),
glm::vec3(0.0f, 4.0f, -10.0f),
glm::vec3(-9.0f, 0.0f, -10.0f),
glm::vec3(-16.0f, 6.0f, -10.0f),
glm::vec3(-30.0f, 2.0f, -10.0f) ,
};

glm::vec3 deCasteljau(std::vector<glm::vec3> points, float t_forward) {
	if (points.size() == 1) {
		return points[0];
	}
	std::vector<glm::vec3> newPoints;
	for (int i = 0; i < points.size() - 1; i++) {
		newPoints.push_back((1 - t_forward) * points[i] + t_forward * points[i + 1]);
	}
	return deCasteljau(newPoints, t_forward);
}


void CreateScene()
{
	gRoot = new GroupNode("root");

	trGolden = new TransformNode("Gold Fish");
	trGolden->SetTranslation(glm::vec3(0.0f, 0.0f, -15.0f));
	trGolden->SetScale(glm::vec3{ 0.7f,0.7f,0.7f });
	trGolden->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	GeometryNode* golden = new GeometryNode("Gold Fish");
	golden->LoadFromFile("./models/Goldfish/13001_Ryukin_Goldfish_v1_L3.obj");
	golden->SetShader(&gShader);
	trGolden->AddChild(golden);
	gRoot->AddChild(trGolden);

	trYellow = new TransformNode("Other Fish");
	trYellow->SetTranslation(glm::vec3(12.0f, 8.0f, -30.0f));
	trYellow->SetScale(glm::vec3{ 0.4f,0.4f,0.4f });
	trYellow->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	GeometryNode* SecFish = new GeometryNode("Other Fish");
	SecFish->LoadFromFile("./models/Fish_2/12265_Fish_v1_L2.obj");
	SecFish->SetShader(&gShader);
	trYellow->AddChild(SecFish);
	gRoot->AddChild(trYellow);

	TransformNode* trSand = new TransformNode("Sand");
	trSand->SetTranslation(glm::vec3(5.0f, 17.0f, -2.0f));
	trSand->SetScale(glm::vec3{ 2.9f,2.5f,2.5f });
	trSand->SetRotation(glm::vec3(85.0f, 0.0f, 0.0f));

	GeometryNode* sandfield = new GeometryNode("Sand");
	sandfield->LoadFromFile("./models/Sand/model.obj");
	sandfield->SetShader(&gShader);
	trSand->AddChild(sandfield);
	gRoot->AddChild(trSand);	


	float time = SDL_GetTicks() * 0.001f;
	float x = sin(time) * 5.0f; 
	float t_back = cos(time) * 5.0f;
}

void UpdateAnimationSpeed()
{
	Uint32 currentTime = SDL_GetTicks();
	float deltaTime = (currentTime - lastFrameTime) / 1000.0f; // Convert to seconds
	lastFrameTime = currentTime;

	float adjustedSpeedFactor = speedFactor * deltaTime;
	t_forward += 0.01f * adjustedSpeedFactor; // Adjust animation speed based on speed factor
	t_back += 0.01f * adjustedSpeedFactor;
}

int main(int argc, char* args[])
{
	init(); 

	
	SDL_Event e;
	//While application is running
	bool quit = false;
	CreateScene();
	while (!quit)
	{
		// per-frame time logic
		// --------------------
		startTime = SDL_GetTicks();
		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else
				{
					HandleKeyDown(e.key);
				}
				break;
			case SDL_MOUSEMOTION:
				if (sceneIsLoaded)
				{
					HandleMouseMotion(e.motion);
					break;
				}
			case SDL_MOUSEWHEEL:
				if (sceneIsLoaded)
				{
					HandleMouseWheel(e.wheel);
					break;
				}
			}

		}

		//Render
		render();

		//Update screen
		SDL_GL_SwapWindow(gWindow);
		sceneIsLoaded = true;
	}

	close();

	return 0;
}

void HandleKeyDown(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_w:
		camera.ProcessKeyboard(FORWARD, deltaTime);
		break;
	case SDLK_s:
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		break;
	case SDLK_a:
		camera.ProcessKeyboard(LEFT, deltaTime);
		break;
	case SDLK_d:
		camera.ProcessKeyboard(RIGHT, deltaTime);
		break;
	case SDLK_LEFT: // Decrease animation speed
		speedFactor -= 0.1f; // Decrease speed factor
		if (speedFactor < 0.1f) // Limit minimum speed factor
			speedFactor = 0.1f;
		printf("Speed decreaded\n");
		break;
	case SDLK_RIGHT: // Increase animation speed
		speedFactor += 0.1f;
		printf("Speed increased\n");
		break;
	}
}

void HandleMouseMotion(const SDL_MouseMotionEvent& motion)
{
	if (firstMouse)
	{
		lastX = motion.x;
		lastY = motion.y;
		firstMouse = false;
	}
	else
	{
		camera.ProcessMouseMovement(motion.x - lastX, lastY - motion.y);
		lastX = motion.x;
		lastY = motion.y;
	}
}

void HandleMouseWheel(const SDL_MouseWheelEvent& wheel)
{
	camera.ProcessMouseScroll(wheel.y);
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);


		//Create window
		gWindow = SDL_CreateWindow("Underwater World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	glewExperimental = true;

	glewInit();

	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		success = false;
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
	}

	glClearColor(0.5f, 0.9f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gShader.Load("./shaders/vertex.vert", "./shaders/fragment.frag");
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //other modes GL_FILL, GL_POINT

	return success;
}


void close()
{
	//delete GL programs, buffers and objects
	glDeleteProgram(gShader.ID);
	glDeleteVertexArrays(1, &gVAO);
	glDeleteBuffers(1, &gVBO);

	//Delete OGL context
	SDL_GL_DeleteContext(gContext);
	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (t_forward <= 1) {
		t_forward += 0.001f * speedFactor;;
		trYellow->SetTranslation(deCasteljau({ animationYellow }, t_forward));
		trGolden->SetTranslation(deCasteljau({ animationGolden }, t_forward));

		trYellow->SetRotation(glm::vec3(trYellow->GetRotation().x, trYellow->GetRotation().y, 0));
		trGolden->SetRotation(glm::vec3(trGolden->GetRotation().x, trGolden->GetRotation().y, 0));

		t_back = 0;
	}
	else {		
		t_back += 0.001f * speedFactor;;
		trYellow->SetTranslation(deCasteljau({ animationYellowBackPoints }, t_back));
		trGolden->SetTranslation(deCasteljau({ animationGoldenBackPoints }, t_back));

		trYellow->SetRotation(glm::vec3(trYellow->GetRotation().x, trYellow->GetRotation().y, 180));
		trGolden->SetRotation(glm::vec3(trGolden->GetRotation().x, trGolden->GetRotation().y, 180));
		
		if (t_back > 1) {
			t_forward = 0;
		}
	}
	
	UpdateAnimationSpeed();

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 4.0f / 3.0f, 0.1f, 100.0f);


	glUseProgram(gShader.ID);
	gShader.setMat4("view", view);
	gShader.setMat4("proj", proj);

	//lighting
	gShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	gShader.setVec3("light.position", lightPos);
	gShader.setVec3("viewPos", camera.Position);

	gRoot->Traverse();

	
}

