#pragma once

#include "mat.h"
#include "wavefront.h"
#include "texture.h"
#include "Text.h"

#include "orbiter.h"
#include "draw.h"
#include "app.h"

#include "GameObject.h"
#include "MeshRenderer.h"
#include "FlyCamera.h"
#include "RotateObjectMouse.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Skybox.h"

#include <chrono>

class Engine : public App
{
private:
	Uint64 lastTime;
	Uint64 newTime;
	Uint64 beginFrame;
	Uint64 endFrame;
	Text console;
	unsigned int oldFPSTimer = 0;
	int frameWidth = 1280;
	int frameHeight = 720;
	float avgFrametime = 0.0f;
	int frametimeCounter = 0;

	// Scene setup
	GameObject* rootObject;
	vector<GameObject*> gameObjects;
	Camera* mainCamera;
	DirectionalLight* mainLight;
	Color ambientLight = Color(0.1f, 0.1f, 0.1f, 0.1f);
	Skybox* skybox;

	// PARAMETRES UTILISATEUR
	//string shaderToUse = "m2tp/Shaders/deferred.glsl";
	//string shaderToUse = "m2tp/Shaders/deferred_SSAO.glsl";
	//string shaderToUse = "m2tp/Shaders/deferred_SSR.glsl";
	string shaderToUse = "m2tp/Shaders/deferred_SSR_SSAO.glsl";
	//string shaderToUse = "m2tp/Shaders/deferred_UltraSSR_SSAO.glsl";
	bool useFlyCamera = false;

public:
	Engine() : App(1280, 720) {}

	int init()
	{
		// Create scene root object
		rootObject = new GameObject();
		gameObjects.push_back(rootObject);

		// Init GranPourrismo game
		InitScene();
		console = create_text();

		// On Start 
		for (int i = 0; i < gameObjects.size(); i++)
		{
			vector<Component*> components = gameObjects[i]->GetAllComponents();
			for (int j = 0; j < components.size(); j++)
				components[j]->Start();
		}

		// etat openGL par defaut
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);       // couleur par defaut de la fenetre

		glClearDepth(1.0f);                         // profondeur par defaut
		glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
		glEnable(GL_DEPTH_TEST);                    // activer le ztest
		glFrontFace(GL_CCW);

		lastTime = SDL_GetPerformanceCounter();
		return 0;   // ras, pas d'erreur
	}

	int quit()
	{
		for (int i = 0; i < gameObjects.size(); i++)
		{
			vector<Component*> components = gameObjects[i]->GetAllComponents();
			for (int j = 0; j < components.size(); j++)
			{
				components[j]->OnDestroy();
				delete components[j];
			}
			delete gameObjects[i];
		}
		release_text(console);
		return 0;
	}

	int render()
	{
		beginFrame = SDL_GetPerformanceCounter();

		// Draw scene
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainCamera->GetFrameBuffer());
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mainCamera->GetColorBuffer(), 0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mainCamera->GetNormalBuffer(), 0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mainCamera->GetDepthBuffer(), 0);
		glViewport(0, 0, frameWidth, frameHeight);
		glClearColor(1, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox->Draw(mainCamera);
		for (int i = 0; i < gameObjects.size(); i++)
		{
			MeshRenderer* renderer = gameObjects[i]->GetComponent<MeshRenderer>();
			if (renderer != nullptr)
				renderer->Draw(mainCamera);
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glUseProgram(0);

		// Final deferred rendering pass (lighting)
		mainCamera->FinishDeferredRendering(mainLight, skybox);

		// Draw post effects
		//mainCamera->DrawPostEffects();

		// Blit to screen
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mainCamera->GetFrameBuffer());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, window_width(), window_height());
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(
			0, 0, frameWidth, frameHeight,
			0, 0, frameWidth, frameHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		mainCamera->UpdatePreviousColorBuffer();

		DisplayGUI();

		glFinish();
		endFrame = SDL_GetPerformanceCounter();
		float delta = (double)((endFrame - beginFrame) * 1000) / SDL_GetPerformanceFrequency();
		frametimeCounter++;
		if (frametimeCounter == 1)
			avgFrametime = delta;
		else
			avgFrametime = avgFrametime + (delta - avgFrametime) / frametimeCounter;
		//cout << delta << "  :  " << avgFrametime << endl;

		return 1;
	}

	int update(const float time, const float delta)
	{
		newTime = SDL_GetPerformanceCounter();
		float delta2 = (double)((newTime - lastTime) * 1000) / SDL_GetPerformanceFrequency();

		//cout << mainCamera->GetGameObject()->GetPosition() << endl;

		for (int i = 0; i < gameObjects.size(); i++)
		{
			gameObjects[i]->UpdateTransformIfNeeded();
			vector<Component*> components = gameObjects[i]->GetAllComponents();
			for (int j = 0; j < components.size(); j++)
				components[j]->Update(delta2);
		}

		lastTime = SDL_GetPerformanceCounter();
		return 0;
	}


	void InitScene()
	{
		// Set up light
		GameObject* lightObject = new GameObject();
		lightObject->SetName("lightObject");
		lightObject->SetPosition(0.0f, 0.0f, 0.0f);
		//lightObject->RotateAround(Vector(0, 1, 0), 192);
		lightObject->RotateAround(Vector(0, 1, 0), 0);
		lightObject->RotateAround(lightObject->GetRightVector(), 45);
		mainLight = new DirectionalLight(1.0f, White());
		lightObject->AddComponent(mainLight);
		rootObject->AddChild(lightObject);
		gameObjects.push_back(lightObject);

		// Set up camera
		GameObject* cameraObject = new GameObject();
		cameraObject->SetName("cameraObject");
		mainCamera = new Camera();
		cameraObject->AddComponent(mainCamera);
		gameObjects.push_back(cameraObject);
		rootObject->AddChild(cameraObject);
		cameraObject->SetPosition(-91.0f, 4.5f, 33.0f);
		cameraObject->RotateAround(Vector(0, 1, 0), 2.0);
		cameraObject->RotateAround(cameraObject->GetRightVector(), 17.0f);
		mainCamera->LoadDeferredShader(shaderToUse);
		mainCamera->SetupFrameBuffer(frameWidth, frameHeight);
		FlyCamera* flyCam = new FlyCamera();
		if(useFlyCamera == true)
			cameraObject->AddComponent(flyCam);

		// Set up skybox
		skybox = new Skybox();
		lightObject->AddComponent(skybox);
		skybox->CreateCubeMap(
			"m2tp/Scene/Skybox1/posz.tga",
			"m2tp/Scene/Skybox1/negz.tga",
			"m2tp/Scene/Skybox1/posy.tga",
			"m2tp/Scene/Skybox1/negy.tga",
			"m2tp/Scene/Skybox1/posx.tga",
			"m2tp/Scene/Skybox1/negx.tga");


		// SETUP SCENE 1
		GameObject* scene1 = new GameObject();
		scene1->SetName("scene1");
		gameObjects.push_back(scene1);
		rootObject->AddChild(scene1);

		GameObject* ball1 = new GameObject();
		ball1->SetName("ball1");
		MeshRenderer* renderer5 = new MeshRenderer();
		ball1->AddComponent(renderer5);
		renderer5->LoadMesh("data/shaderball.obj");
		renderer5->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer5->LoadPBRTextures("m2tp/Textures/gold-scuffed_basecolor.png", "m2tp/Textures/gold-scuffed_roughness.png", "m2tp/Textures/gold-scuffed_metallic.png");
		gameObjects.push_back(ball1);
		scene1->AddChild(ball1);
		ball1->SetPosition(0.0f, -2.0f, 0.0f);
		renderer5->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* cube1 = new GameObject();
		cube1->SetName("cube1");
		MeshRenderer* renderer2 = new MeshRenderer();
		cube1->AddComponent(renderer2);
		renderer2->LoadMesh("data/cube.obj");
		renderer2->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer2->LoadPBRTextures("m2tp/Textures/oakfloor_basecolor.png", "m2tp/Textures/oakfloor_roughness.png", "m2tp/Textures/black.jpg");
		gameObjects.push_back(cube1);
		scene1->AddChild(cube1);
		cube1->SetPosition(0.0f, -10.0f, 0.0f);
		cube1->SetScale(40.0f, 1.0f, 40.0f);
		renderer2->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* cube3 = new GameObject();
		cube3->SetName("cube3");
		MeshRenderer* renderer4 = new MeshRenderer();
		cube3->AddComponent(renderer4);
		renderer4->LoadMesh("data/cube.obj");
		renderer4->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer4->LoadPBRTextures("m2tp/Textures/rustediron_basecolor.png", "m2tp/Textures/rustediron_roughness.png", "m2tp/Textures/rustediron_metallic.png");
		gameObjects.push_back(cube3);
		scene1->AddChild(cube3);
		cube3->SetPosition(20.0f, 10.0f, 0.0f);
		cube3->SetScale(40.0f, 1.0f, 40.0f);
		cube3->RotateAround(Vector(1, 0, 0), -90.0f);
		cube3->RotateAround(Vector(0, 1, 0), -90.0f);
		renderer4->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* cube0 = new GameObject();
		cube0->SetName("cube");
		MeshRenderer* renderer1 = new MeshRenderer();
		cube0->AddComponent(renderer1);
		renderer1->LoadMesh("data/cube.obj");
		renderer1->LoadShader("m2tp/Shaders/basic_shader.glsl");
		renderer1->LoadTexture("m2tp/Textures/colors.jpg", 1.0f, 0.0f);
		gameObjects.push_back(cube0);
		scene1->AddChild(cube0);
		cube0->SetPosition(-20.0f, 10.0f, 0.0f);
		cube0->SetScale(40.0f, 1.0f, 40.0f);
		cube0->RotateAround(Vector(1, 0, 0), -90.0f);
		cube0->RotateAround(Vector(0, 1, 0), 90.0f);
		renderer1->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* cube2 = new GameObject();
		cube2->SetName("cube2");
		MeshRenderer* renderer3 = new MeshRenderer();
		cube2->AddComponent(renderer3);
		renderer3->LoadMesh("data/cube.obj");
		renderer3->LoadShader("m2tp/Shaders/basic_shader.glsl");
		renderer3->LoadTexture("data/debug2x2red.png", 1.0f, 0.0f);
		gameObjects.push_back(cube2);
		scene1->AddChild(cube2);
		cube2->SetPosition(0.0f, 10.0f, -20.0f);
		cube2->SetScale(40.0f, 1.0f, 40.0f);
		cube2->RotateAround(Vector(1, 0, 0), -90.0f);
		renderer3->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		// SETUP SCENE 2
		GameObject* scene2 = new GameObject();
		scene2->SetName("scene2");
		gameObjects.push_back(scene2);
		rootObject->AddChild(scene2);
		scene2->SetPosition(-90.0f, 0.0f, 0.0f);

		GameObject* ball2 = new GameObject();
		ball2->SetName("ball2");
		MeshRenderer* renderer6 = new MeshRenderer();
		ball2->AddComponent(renderer6);
		renderer6->LoadMesh("data/shaderball.obj");
		renderer6->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer6->LoadPBRTextures("m2tp/Textures/rustediron_basecolor.png", "m2tp/Textures/rustediron_roughness.png", "m2tp/Textures/rustediron_metallic.png");
		gameObjects.push_back(ball2);
		scene2->AddChild(ball2);
		ball2->SetPosition(0.0f, -2.0f, 0.0f);
		renderer6->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* ball3 = new GameObject();
		ball3->SetName("ball3");
		MeshRenderer* renderer8 = new MeshRenderer();
		ball3->AddComponent(renderer8);
		renderer8->LoadMesh("data/shaderball.obj");
		renderer8->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer8->LoadPBRTextures("m2tp/Textures/gold-scuffed_basecolor.png", "m2tp/Textures/gold-scuffed_roughness.png", "m2tp/Textures/gold-scuffed_metallic.png");
		gameObjects.push_back(ball3);
		scene2->AddChild(ball3);
		ball3->SetPosition(13.0f, -2.0f, 0.0f);
		renderer8->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* ball4 = new GameObject();
		ball4->SetName("ball4");
		MeshRenderer* renderer9 = new MeshRenderer();
		ball4->AddComponent(renderer9);
		renderer9->LoadMesh("data/shaderball.obj");
		renderer9->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer9->LoadPBRTextures("m2tp/Textures/aluminium_basecolor.png", "m2tp/Textures/aluminium_roughness.png", "m2tp/Textures/aluminium_metallic.png");
		gameObjects.push_back(ball4);
		scene2->AddChild(ball4);
		ball4->SetPosition(-13.0f, -2.0f, 0.0f);
		renderer9->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* ball5 = new GameObject();
		ball5->SetName("ball5");
		MeshRenderer* renderer10 = new MeshRenderer();
		ball5->AddComponent(renderer10);
		renderer10->LoadMesh("data/shaderball.obj");
		renderer10->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer10->LoadPBRTextures("m2tp/Textures/mahogfloor_basecolor.png", "m2tp/Textures/mahogfloor_roughness.png", "m2tp/Textures/black.jpg");
		gameObjects.push_back(ball5);
		scene2->AddChild(ball5);
		ball5->SetPosition(-26.0f, -2.0f, 0.0f);
		renderer10->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* ball6 = new GameObject();
		ball6->SetName("ball6");
		MeshRenderer* renderer11 = new MeshRenderer();
		ball6->AddComponent(renderer11);
		renderer11->LoadMesh("data/shaderball.obj");
		renderer11->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer11->LoadPBRTextures("m2tp/Textures/blocksrough_basecolor.png", "m2tp/Textures/blocksrough_roughness.png", "m2tp/Textures/blocksrough_metallic.jpg");
		gameObjects.push_back(ball6);
		scene2->AddChild(ball6);
		ball6->SetPosition(26.0f, -2.0f, 0.0f);
		renderer11->SetColor(Color(1.0, 1.0, 1.0, 1.0));

		GameObject* cube4 = new GameObject();
		cube4->SetName("cube4");
		MeshRenderer* renderer7 = new MeshRenderer();
		cube4->AddComponent(renderer7);
		renderer7->LoadMesh("data/cube.obj");
		renderer7->LoadShader("m2tp/Shaders/pbr_shader.glsl");
		renderer7->LoadPBRTextures("m2tp/Textures/aluminium_basecolor.png", "m2tp/Textures/aluminium_roughness.png", "m2tp/Textures/aluminium_metallic.png");
		gameObjects.push_back(cube4);
		scene2->AddChild(cube4);
		cube4->SetPosition(0.0f, -10.0f, 0.0f);
		cube4->SetScale(70.0f, 1.0f, 70.0f);
		renderer7->SetColor(Color(1.0, 1.0, 1.0, 1.0));
	}

	void DisplayGUI()
	{
		clear(console);
		unsigned int currentFPSTimer = SDL_GetTicks();
		printf(console, 0, 0, "FPS: %.1f", 1000.0f / (currentFPSTimer - oldFPSTimer));
		oldFPSTimer = currentFPSTimer;
		draw(console, window_width(), window_height());
	}
};