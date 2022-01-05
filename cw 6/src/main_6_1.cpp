#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

#include <iostream>
using namespace std;

GLuint programColor;
GLuint programTexture;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext bottomContext;
Core::RenderContext fishContext;
Core::RenderContext fishContext2;
Core::RenderContext sharkContext;
Core::RenderContext duckContext;


glm::vec3 fishPositions[100];
float fishSpreadFactor = 50.0;

float old_x, old_y = -1;
float delta_x, delta_y = 0;
glm::quat rotationCamera = glm::quat(1, 0, 0, 0);
glm::quat rotation_y = glm::normalize(glm::angleAxis(209 * 0.03f, glm::vec3(1, 0, 0)));
glm::quat rotation_x = glm::normalize(glm::angleAxis(307 * 0.03f, glm::vec3(0, 1, 0)));
float dy = 0;
float dx = 0;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);

GLuint textureReef;
GLuint textureFish;
GLuint textureFish2;
GLuint textureShark;
GLuint textureDuck;

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	if (old_x >= 0) {
		delta_x = x - old_x;
		delta_y = y - old_y;
	}
	old_x = x;
	old_y = y;
}


glm::mat4 createCameraMatrix()
{
	auto rot_y = glm::angleAxis(delta_y * 0.03f, glm::vec3(1, 0, 0));
	auto rot_x = glm::angleAxis(delta_x * 0.03f, glm::vec3(0, 1, 0));

	dy += delta_y;
	dx += delta_x;
	delta_x = 0;
	delta_y = 0;

	rotation_x = glm::normalize(rot_x * rotation_x);
	rotation_y = glm::normalize(rot_y * rotation_y);

	rotationCamera = glm::normalize(rotation_y * rotation_x);

	auto inverse_rot = glm::inverse(rotationCamera);

	cameraDir = inverse_rot * glm::vec3(0, 0, -1);
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = inverse_rot * glm::vec3(1, 0, 0);

	glm::mat4 cameraTranslation;
	cameraTranslation[3] = glm::vec4(-cameraPos, 1.0f);

	return glm::mat4_cast(rotationCamera) * cameraTranslation;
}

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext context, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}
void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();
	float timee = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0,-0.25f,0)) * glm::rotate(glm::radians(90.0f), glm::vec3(1,0,0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.002f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * shipInitialTransformation;
	drawObjectTexture(shipContext, shipModelMatrix,textureDuck);

	drawObjectTexture(bottomContext, glm::translate(glm::vec3(0,-5,0)) * glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)) * glm::scale(glm::vec3(0.25f) * glm::vec3(0.25f)), textureReef);


	for (int i = 0; i < 30; i++) {
		if (i % 10 == 0) {
			drawObjectTexture(fishContext2, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.7f) * glm::vec3(0.7f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, -1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-70, -80, -50)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish2);

		} else if (i % 7 == 0) {
			drawObjectTexture(fishContext, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.5f) * glm::vec3(0.5f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-50 - 3 * i, -50, -70)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish);

		} else if (i % 3 == 0) {
			drawObjectTexture(fishContext, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.5f) * glm::vec3(0.5f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, -1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-50 - 3 * i, -50, -85)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish);
		} else {
			drawObjectTexture(fishContext2, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.25f) * glm::vec3(0.25f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-50, -50, -fishPositions[i].z)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish2);
		}	
	}

	for (int j = 0; j < 2; j++) drawObjectTexture(sharkContext, glm::translate(glm::vec3( 70 * j, 100 + 20 * j, 20 + 50 * j)) *
									glm::rotate(glm::radians((timee / 12) * 90.f), glm::vec3(0, 1, 0)) *
									glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(-95, -95, -100)) *
									glm::rotate(glm::radians((timee / 12) * 0.f), glm::vec3(0, 1, 0)) * glm::eulerAngleY(sin(timee * 2) / 4) * 
									glm::scale(glm::vec3(0.3f) * glm::vec3(0.3f)) *
									glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
									glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)),
									textureShark);

	for (int j = 0; j < 2; j++) drawObjectTexture(sharkContext, glm::translate(glm::vec3(70 * j, 100 + 20 * j, 20 + 50 * j)) *
		glm::rotate(glm::radians((timee / 12) * 90.f), glm::vec3(0, -1, 0)) *
		glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(-95, -95, -100)) *
		glm::rotate(glm::radians((timee / 12) * 0.f), glm::vec3(0, 1, 0)) * glm::eulerAngleY(sin(timee * 2) / 4) *
		glm::scale(glm::vec3(0.3f) * glm::vec3(0.3f)) *
		glm::rotate(glm::radians(120.0f), glm::vec3(0, -1, 0)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)),
		textureShark);


	drawObjectTexture(sharkContext, glm::translate(glm::vec3(100, 100, 100)) *
		glm::rotate(glm::radians((timee / 12) * 90.f), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(-95, -95, -100)) *
		glm::rotate(glm::radians((timee / 12) * 0.f), glm::vec3(0, 1, 0)) * glm::eulerAngleY(sin(timee * 2) / 4) *
		glm::scale(glm::vec3(0.4f) * glm::vec3(0.4f)) *
		glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)),
		textureShark);

	glutSwapBuffers();
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	loadModelToContext("models/boat.obj", shipContext);
	loadModelToContext("models/bottom.obj", bottomContext);
	loadModelToContext("models/nemo.obj", fishContext);
	loadModelToContext("models/fish.obj", fishContext2);
	loadModelToContext("models/shark.obj", sharkContext);
	loadModelToContext("models/duck.obj", duckContext);

	textureFish = Core::LoadTexture("textures/b.jpg");
	textureReef = Core::LoadTexture("textures/reef.jpg");
	textureFish2 = Core::LoadTexture("textures/fish.png");
	textureShark = Core::LoadTexture("textures/shark.jpg");
	textureDuck = Core::LoadTexture("textures/boat.jpg");
	//textureFish = Core::LoadTexture("textures/xd.jpg");
	for (int i = 0; i < 100; i++) fishPositions[i] = glm::ballRand(fishSpreadFactor);
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
