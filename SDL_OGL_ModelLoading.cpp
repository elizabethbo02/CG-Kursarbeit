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

#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

bool init();
bool initGL();
void render();
//GLuint CreateCube(float, GLuint&, GLuint&);
//void DrawCube(GLuint id);
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

// camera
Camera camera(glm::vec3(3.0f, 2.0f, 6.0f));
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

glm::vec3 deCasteljau(std::vector<glm::vec3> points, float t);

//event handlers
void HandleKeyDown(const SDL_KeyboardEvent& key);
void HandleMouseMotion(const SDL_MouseMotionEvent& motion);
void HandleMouseWheel(const SDL_MouseWheelEvent& wheel);

Uint32 startTime;

std::vector<glm:: vec3> animation = {
glm:: vec3(5.0f, 5.0f, -10.0f) ,
glm::vec3(10.0f, 10.0f, -15.0f),
// we can add however many control points as we like (main points where the tear drop should go through)
};

glm::vec3 deCasteljau(std::vector<glm::vec3> points, float t) {
	if (points.size() == 1) {
		return points[0];
	}
	std::vector<glm::vec3> newPoints;
	for (int i = 0; i < points.size() - 1; i++) {
		newPoints.push_back((1 - t) * points[i] + t * points[i + 1]);
	}
	return deCasteljau(newPoints, t);
}

float t = 0;
float anotherT = 3;

void CreateScene()
{
	gRoot = new GroupNode("root");

	TransformNode* trGolden = new TransformNode("Gold Fish");
	trGolden->SetTranslation(glm::vec3(0.0f, 0.0f, -15.0f));
	trGolden->SetScale(glm::vec3{ 0.7f,0.7f,0.7f });
	trGolden->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	GeometryNode* golden = new GeometryNode("Gold Fish");
	golden->LoadFromFile("./models/Goldfish/13001_Ryukin_Goldfish_v1_L3.obj");
	golden->SetShader(&gShader);
	trGolden->AddChild(golden);
	gRoot->AddChild(trGolden);

	TransformNode* trFish = new TransformNode("Other Fish");
	trFish->SetTranslation(glm::vec3(12.0f, 8.0f, -30.0f));
	trFish->SetScale(glm::vec3{ 0.4f,0.4f,0.4f });
	trFish->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	GeometryNode* SecFish = new GeometryNode("Other Fish");
	SecFish->LoadFromFile("./models/Fish_2/12265_Fish_v1_L2.obj");
	SecFish->SetShader(&gShader);
	trFish->AddChild(SecFish);
	gRoot->AddChild(trFish);

	TransformNode* trSand = new TransformNode("Sand");
	trSand->SetTranslation(glm::vec3(4.0f, 17.0f, -2.0f));
	trSand->SetScale(glm::vec3{ 2.5f,2.5f,2.5f });
	trSand->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

	GeometryNode* sandfield = new GeometryNode("Sand");
	sandfield->LoadFromFile("./models/Sand/model.obj");
	sandfield->SetShader(&gShader);
	trSand->AddChild(sandfield);
	gRoot->AddChild(trSand);

	
	
	// Update fish translation based on de Casteljau algorithm
	if (t <= 1) {
		t += 0.001f;
		trFish->SetTranslation(deCasteljau({ animation }, t));
		if (anotherT <= 1)
		{
			anotherT += 0.002f;
			trFish->SetTranslation(deCasteljau({ animation }, anotherT));
		}
		else
		{
			trFish->SetTranslation(deCasteljau({ animation }, t));
		}
	}
	else {
		t = 0;
	}

	
	// Update fish translation based on time for animation
	float time = SDL_GetTicks() * 0.001f;
	float x = sin(time) * 5.0f; // Adjust the amplitude and frequency for desired movement
	float y = cos(time) * 5.0f;
	//trFish->SetTranslation(glm::vec3(x, y, -30.0f)); // Adjust the depth as needed
	trFish->SetTranslation(deCasteljau({ animation }, t));

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

		/*
		float time = SDL_GetTicks() * 0.001f;
		float x = sin(time) * 5.0f; // Adjust the amplitude and frequency for desired movement
		float y = cos(time) * 5.0f;
		//TransformNode* trFish = dynamic_cast<TransformNode*>(gRoot->FindNodeByName("Other Fish"));
		//trFish->SetTranslation(glm::vec3(x, y, -30.0f)); // Adjust the depth as needed
*/

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
				HandleMouseMotion(e.motion);
				break;
			case SDL_MOUSEWHEEL:
				HandleMouseWheel(e.wheel);
				break;
			}
		}

		//Render
		render();

		//Update screen
		SDL_GL_SwapWindow(gWindow);
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


	//gVAO = CreateCube(1.0f, gVBO, gEBO);

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

	//glm::mat4 model = glm::mat4(1.0f);
	//model = glm::rotate(model, glm::radians(250.0f), glm::vec3(1, 0, 0));
	//model = glm::translate(model, glm::vec3(-0.5f, 1.0f, 1.5f));

	//depending on the model size, the model may have to be scaled up or down to be visible
    //model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
	//model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 4.0f / 3.0f, 0.1f, 100.0f);

	//glm::mat3 normalMat = glm::transpose(glm::inverse(model));

	glUseProgram(gShader.ID);
	//gShader.setMat4("model", model);
	gShader.setMat4("view", view);
	gShader.setMat4("proj", proj);
	//gShader.setMat3("normalMat", normalMat);

	//lighting
	gShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	gShader.setVec3("light.position", lightPos);
	gShader.setVec3("viewPos", camera.Position);

	//gModel.Draw(gShader);
	gRoot->Traverse();
	//	DrawCube(gVAO);
}

//GLuint CreateCube(float width, GLuint& VBO, GLuint& EBO)
//{
//	GLfloat vertices[] = {
//		//vertex position
//		0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // top right
//		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
//		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // bottom left
//		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f   // top left 
//	};
//	//indexed drawing - we will be using the indices to point to a vertex in the vertices array
//	GLuint indices[] = {
//		0, 1, 3,   // first triangle
//		1, 2, 3    // second triangle
//	};
//
//
//	GLuint VAO;
//	glGenBuffers(1, &VBO);
//	glGenBuffers(1, &EBO);
//	glGenVertexArrays(1, &VAO);
//
//	glBindVertexArray(VAO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//
//	//we have to change the stride to 6 floats, as each vertex now has 6 attribute values
//	//the last value (pointer) is still 0, as the position values start from the beginning
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //the data comes from the currently bound GL_ARRAY_BUFFER
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//	glEnableVertexAttribArray(1);
//
//	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//	glBindVertexArray(0);
//	//the elements buffer must be unbound after the vertex array otherwise the vertex array will not have an associated elements buffer array
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//	return VAO;
//}

//void DrawCube(GLuint vaoID)
//{
//	glBindVertexArray(vaoID);
//
//	//glDrawElements uses the indices in the EBO to get to the vertices in the VBO
//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//	glBindVertexArray(0);
//}


