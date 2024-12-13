#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include <stdlib.h>



using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world = new GameWorld();
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);
	//std::vector<GameObject*> floatingCats;

	forceMagnitude = 10.0f;
	useGravity = true;
	physics->UseGravity(useGravity);
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();

	currentState = GameState::Menu;
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	catMesh = renderer->LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh = renderer->LoadMesh("Kitten.msh");

	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	//delete constraint;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdatePlayer(float dt) {
	Vector3 force(0.0f, 0.0f, 0.0f);
	float camYaw = world->GetMainCamera().GetYaw() * PI / 180.0f; //* PI / 180.0f is new
	float camPitch = world->GetMainCamera().GetPitch();

	Vector3 forward = Vector3(sinf(camYaw), 0, cosf(camYaw));
	Vector3 right = Vector3(cosf(camYaw), 0, -sinf(camYaw));

	//Vector3 forward = Vector3(sinf(camYaw * PI / 180.0f), 0, cosf(camYaw * PI / 180.0f));
	//Vector3 right = Vector3(cosf(camYaw * PI / 180.0f), 0, -sinf(camYaw * PI / 180.0f));

	if (controller.GetAxis(2) > 0.1f) {
		force -= (forward * controller.GetAxis(2)); //Forward (W)
	}
	if (controller.GetAxis(2) < -0.1f) {
		force += forward * fabs(controller.GetAxis(2)); //Backward (S)
	}
	if (controller.GetAxis(0) > 0.1f) {
		force += right * controller.GetAxis(0); //Right (D)
	}
	if (controller.GetAxis(0) < -0.1f) {
		force -= right * fabs(controller.GetAxis(0));
	}
	
	//playerPhysics->AddForce(Vector::Normalise(force) * 500.0f * dt);
	//float yawDelta = controller.GetAxis(3) * 0.1f;
	//float pitchDelta = -controller.GetAxis(4) * 0.1f;

	//world->GetMainCamera().SetYaw(camYaw + yawDelta);
	//world->GetMainCamera().SetPitch(std::clamp(camPitch + pitchDelta, -45.0f, 45.0f));

	if (Vector::Length(force) > 0.1f) {
		Vector3 normalizedForce = Vector::Normalise(force);
		force = normalizedForce * 1000.0f * dt; //New
		playerPhysics->AddForce(force);//New
		//float yaw = atan2(normalizedForce.x, normalizedForce.z) * 180.0f / PI;
		float yaw = atan2(force.x, force.z) * 180.0f / PI;

		Quaternion orientation = Quaternion::EulerAnglesToQuaternion(0, yaw, 0);
		playerObject->GetTransform().SetOrientation(orientation);
	}

	/*force.x = controller.GetAxis(0);
	force.z = controller.GetAxis(2);

	playerPhysics->AddForce((Vector::Normalise(force)) * 100.0f * dt);

	float yaw = controller.GetAxis(3) * 0.1f;
	float pitch = controller.GetAxis(4) * 0.1f;

	Quaternion orientation = playerObject->GetTransform().GetOrientation();
	orientation = orientation * Quaternion::EulerAnglesToQuaternion(pitch, yaw, 0);
	playerObject->GetTransform().SetOrientation(orientation);*/



}

void TutorialGame::UpdateGame(float dt) {
	//if (!inSelectionMode) {
		//world->GetMainCamera().UpdateCamera(dt);
	//}
	if (lockedObject == nullptr) {
		Debug::Print("Press P to play!", Vector2(5, 95), Debug::RED);
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
			lockedObject = playerObject;
			elapsedTime = 0.0f;
			timerActive = true;
		}
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = Matrix::Inverse(temp);

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::V)) {
			lockedObject = nullptr;
		}
	}

	if (timerActive) {
		elapsedTime += dt;
		int seconds = static_cast<int>(elapsedTime);
		int milliseconds = static_cast<int>((elapsedTime - seconds) * 1000);
		std::string timerText = std::to_string(seconds) + "." + (milliseconds < 100 ? "0" : "") + std::to_string(milliseconds);
		Debug::Print(timerText, Vector2(40, 5));
	}

	//UpdateKeys();

	/*if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::MAGENTA);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}*/
	//This year we can draw debug textures as well!
	//Debug::DrawTex(*basicTex, Vector2(10, 10), Vector2(5, 5), Debug::MAGENTA);

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();

	//world->UpdateWorld(dt);
	//renderer->Update(dt);
	//physics->Update(dt);
	//UpdatePlayer(dt);
	//CheckForKeyCollection();
	//UpdateAI(dt);

	if (playerObject->GetKeyCollected()) {
		//InitCamera();
		//InitWorld();
		Debug::Print("Key collected!", Vector2(1, 80));
	}

	if (playerObject->GetTransform().GetPosition().y < -26.0f) {
		Debug::Print("You fell off! Press R to play again or ESC to quit", Vector2(1, 50));
		floatingCats.clear();
		timerActive = false;
		int seconds = static_cast<int>(elapsedTime);
		int milliseconds = static_cast<int>((elapsedTime - seconds) * 1000);
		std::string timerText = std::to_string(seconds) + "." + (milliseconds < 100 ? "0" : "") + std::to_string(milliseconds);
		Debug::Print(timerText, Vector2(40, 5));

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
			InitCamera();
			InitWorld();
		}
	}

	if (floatingCats.size() == 4) {
		Debug::Print("All kittens collected!", Vector2(60, 5));
	}
	if (floatingCats.size() < 4) {
		std::string kittensText = "Kittens collected: " + std::to_string(floatingCats.size()) + "/4";
		Debug::Print(kittensText, Vector2(5, 85));
	}
	if (enemySpawn == true) {
		//AddEnemyToWorld(Vector3(-30, -15, -263));
		//AddEnemyToWorld(Vector3(30, -15, -265));
		//AddEnemyToWorld(Vector3(-30, -15, -363));
		//AddEnemyToWorld(Vector3(30, -15, -365));
		//InitCamera();
		SecondEnemySpawn();
		enemySpawn = false;
	}

	if (caught == true) {
		InitCamera();
		Debug::Print("You got caught! Press R to play again or ESC to quit", Vector2(1, 50));
		floatingCats.clear();
		timerActive = false;
		int seconds = static_cast<int>(elapsedTime);
		int milliseconds = static_cast<int>((elapsedTime - seconds) * 1000);
		std::string timerText = std::to_string(seconds) + "." + (milliseconds < 100 ? "0" : "") + std::to_string(milliseconds);
		Debug::Print(timerText, Vector2(40, 5));

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
			caught = false;
			InitCamera();
			InitWorld();
		}
	}

	if (victory == true) {
		if (floatingCats.size() == 4 && playerObject->GetKeyCollected() == true) {
			InitCamera();
			timerActive = false;
			int seconds = static_cast<int>(elapsedTime);
			int milliseconds = static_cast<int>((elapsedTime - seconds) * 1000);
			std::string timerText = std::to_string(seconds) + "." + (milliseconds < 100 ? "0" : "") + std::to_string(milliseconds);
			std::string victoryText = "Congratulations! You escaped in " + timerText + " seconds!";
			Debug::Print(victoryText, Vector2(1, 50));

			if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
				floatingCats.clear();
				victory = false;
				InitCamera();
				InitWorld();
			}
		}
		//InitCamera();
		//timerActive = false;
		//int seconds = static_cast<int>(elapsedTime);
		//int milliseconds = static_cast<int>((elapsedTime - seconds) * 1000);
		//std::string timerText = std::to_string(seconds) + "." + (milliseconds < 100 ? "0" : "") + std::to_string(milliseconds);
		//std::string victoryText = "Congratulations! You escaped in " + timerText + " seconds!";
		//Debug::Print(victoryText, Vector2(1, 50));

		//if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
			//victory = false;
			//InitCamera();
			//InitWorld();
		//}
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	UpdatePlayer(dt);
	CheckForKeyCollection();
	UpdateAI(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);

	if (testStateObject) {
		testStateObject->Update(dt);
	}
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(45.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 50, 0));

	/*if (currentState == GameState::Menu) {
		world->GetMainCamera().SetPitch(45.0f);
		world->GetMainCamera().SetYaw(0.0f);
		world->GetMainCamera().SetPosition(Vector3(0, 50, 0));
	}*/
	//world->GetMainCamera().SetPitch(45.0f);
	//world->GetMainCamera().SetYaw(0.0f);
	//world->GetMainCamera().SetPosition(Vector3(0, 50, 0));
	// 
	//world->GetMainCamera().SetPitch(-15.0f);
	//world->GetMainCamera().SetYaw(315.0f);
	//world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	InitGameExamples();
	AddPlayerToWorld(Vector3(0, -15, 0));
	InitDefaultFloor();
	//BridgeConstraintTest();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) { //This was const
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2.0f)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddSecondFloorToWorld() {
	GameObject* secondFloor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	secondFloor->SetBoundingVolume((CollisionVolume*)volume);
	secondFloor->GetTransform()
		.SetScale(floorSize * 2.0f)
		.SetPosition(Vector3(0, -20, -413));

	secondFloor->SetRenderObject(new RenderObject(&secondFloor->GetTransform(), cubeMesh, basicTex, basicShader));
	secondFloor->SetPhysicsObject(new PhysicsObject(&secondFloor->GetTransform(), secondFloor->GetBoundingVolume()));

	secondFloor->GetPhysicsObject()->SetInverseMass(0);
	secondFloor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(secondFloor);

	return secondFloor;
}


/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddYarnToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	sphere->SetName("Yarn");
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddZoneToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	cube->SetName("Zone");
	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();
	SphereVolume* volume = new SphereVolume(1.0f);
	character->SetName("Player");

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	playerObject = character;
	playerPhysics = character->GetPhysicsObject();
	//lockedObject = playerObject;

	return character;
}

GameObject* TutorialGame::AddFloatingCat(const Vector3& position) {
	GameObject* cat = new GameObject();
	cat->SetName("FloatingCat");

	SphereVolume* volume = new SphereVolume(1.0f);
	cat->SetBoundingVolume((CollisionVolume*)volume);

	cat->GetTransform().SetScale(Vector3(0.6, 0.6, 0.6)).SetPosition(position);

	cat->SetRenderObject(new RenderObject(&cat->GetTransform(), catMesh, nullptr, basicShader));
	cat->SetPhysicsObject(new PhysicsObject(&cat->GetTransform(), cat->GetBoundingVolume()));
	cat->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	cat->GetPhysicsObject()->SetInverseMass(0);

	world->AddGameObject(cat);
	return cat;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	character->SetName("Enemy");
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->SetSpeed(3.0f);

	world->AddGameObject(character);

	return character;
}

/*void TutorialGame::UpdateAI(float dt) {
	for (GameObject* obj : world->GetGameObjects()) {
		if (obj->GetName() == "EnemyOne" || obj->GetName() == "EnemyTwo") {
			Vector3 position = obj->GetTransform().GetPosition();
			//float speed = 3.0f;
			Vector3 playerPos = playerObject->GetTransform().GetPosition();
			Vector3 direction = Vector::Normalise((playerPos - position));

			//position.x += speed * dt;
			position.x += obj->GetSpeed() * dt;

			if (position.x > 30.0f || position.x < -30.0f) {
				//speed = -speed;
				obj->SetSpeed(-(obj->GetSpeed()));
			}
			//obj->GetTransform().SetPosition(position);

			Vector3 rayDirection = Vector3(obj->GetSpeed() > 0 ? 1 : -1, 0, 0);
			Ray ray(position, rayDirection);
			RayCollision collision;

			if (world->Raycast(ray, collision, true)) {
				GameObject* hitObject = (GameObject*)collision.node;

				if (hitObject == playerObject) {
					Vector3 chaseForce = direction * 5.0f;
					obj->GetPhysicsObject()->AddForce(chaseForce);
				}
			}

			obj->GetTransform().SetPosition(position);
		}
	}
}*/

void TutorialGame::UpdateAI(float dt) {
	for (GameObject* obj : world->GetGameObjects()) {
		if (obj->GetName() == "Enemy") {
			Vector3 position = obj->GetTransform().GetPosition();
			//float speed = 3.0f;
			Vector3 playerPos = playerObject->GetTransform().GetPosition();
			Vector3 direction = Vector::Normalise((playerPos - position));

			if (obj->GetChase() == false) {
				position.x += obj->GetSpeed() * dt;

				if (position.x > 30.0f || position.x < -30.0f) {
					//speed = -speed;
					obj->SetSpeed(-(obj->GetSpeed()));
				}

				Vector3 rayDirection = Vector3(obj->GetSpeed() > 0 ? 1 : -1, 0, 0);
				Ray ray(position, (rayDirection * 10.0f));
				RayCollision collision;

				if (world->Raycast(ray, collision, true)) {
					GameObject* hitObject = (GameObject*)collision.node;

					if (hitObject == playerObject) {
						obj->setChase(true);
					}
				}
				//obj->GetTransform().SetPosition(position);
			else {
					Vector3 chaseForce = direction * 5.0f;
					obj->GetPhysicsObject()->AddForce(chaseForce);

					Ray ray(position, (direction * 15.0f));
					RayCollision collision;

					if (!world->Raycast(ray, collision, true) || collision.node != playerObject) {
						obj->setChase(false);
					}
				}

				obj->GetTransform().SetPosition(position);
		    }
		}
		if (obj->GetName() == "floatingCatTwo" && obj->GetFollow() == true) {
			Vector3 playerPos = playerObject->GetTransform().GetPosition();
			Quaternion playerRotation = playerObject->GetTransform().GetOrientation();

			Vector3 offset = Vector3(0.5f, 0, -2.0f); // Offset behind the player
			Vector3 followPosition = playerPos + playerRotation * offset;

			obj->GetTransform().SetPosition(followPosition);
			obj->GetTransform().SetOrientation(playerRotation);
		}
		if (obj->GetName() == "floatingCatOne" && obj->GetFollow() == true) {
			Vector3 playerPos = playerObject->GetTransform().GetPosition();
			Quaternion playerRotation = playerObject->GetTransform().GetOrientation();

			Vector3 offset = Vector3(2.5f, 0, -2.0f); // Offset behind the player
			Vector3 followPosition = playerPos + playerRotation * offset;

			obj->GetTransform().SetPosition(followPosition);
			obj->GetTransform().SetOrientation(playerRotation);
		}
		if (obj->GetName() == "floatingCatThree" && obj->GetFollow() == true) {
			Vector3 playerPos = playerObject->GetTransform().GetPosition();
			Quaternion playerRotation = playerObject->GetTransform().GetOrientation();

			Vector3 offset = Vector3(-1.5f, 0, -2.0f); // Offset behind the player
			Vector3 followPosition = playerPos + playerRotation * offset;

			obj->GetTransform().SetPosition(followPosition);
			obj->GetTransform().SetOrientation(playerRotation);
		}
		if (obj->GetName() == "floatingCatFour" && obj->GetFollow() == true) {
			Vector3 playerPos = playerObject->GetTransform().GetPosition();
			Quaternion playerRotation = playerObject->GetTransform().GetOrientation();

			Vector3 offset = Vector3(4.5f, 0, -2.0f); // Offset behind the player
			Vector3 followPosition = playerPos + playerRotation * offset;

			obj->GetTransform().SetPosition(followPosition);
			obj->GetTransform().SetOrientation(playerRotation);
		}
	}
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
	//AddFloorToWorld(Vector3(0, -20, -213));
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	//AddEnemyToWorld(Vector3(5, -15, 0));
	AddEnemyToWorld(Vector3(-30, -15, -50));
	AddEnemyToWorld(Vector3(30, -15, -52));
	AddEnemyToWorld(Vector3(-30, -15, -150));
	AddEnemyToWorld(Vector3(30, -15, -152));
	//AddEnemyToWorld(Vector3(-30, -15, -263));
	//AddEnemyToWorld(Vector3(30, -15, -265));
	//AddEnemyToWorld(Vector3(-30, -15, -363));
	//AddEnemyToWorld(Vector3(30, -15, -365));
	//AddBonusToWorld(Vector3(10, -15, 0));
	GameObject* yarn = AddYarnToWorld(Vector3(5, -17, -280), 0.8f);
	yarn->SetName("yarn");
	GameObject* keyBlock = AddZoneToWorld(Vector3(5, -17, -300), Vector3(1, 1, 1)); //z = -200 is the very edge of the floor
	keyBlock->SetName("keyBlock");
	GameObject* endZone = AddZoneToWorld(Vector3(5, -17, -350), Vector3(1, 1, 1)); //z = -200 is the very edge of the floor
	endZone->SetName("endZone");
	AddZigZagToWorld();
	AddSecondFloorToWorld();
	GameObject* floatingCatOne = AddFloatingCat(Vector3(1, -17, -206));
	floatingCatOne->SetName("floatingCatOne");
	GameObject* floatingCatTwo = AddFloatingCat(Vector3(-20, -17, -75));
	floatingCatTwo->SetName("floatingCatTwo");
	GameObject* floatingCatThree = AddFloatingCat(Vector3(5, -17, -226));
	floatingCatThree->SetName("floatingCatThree");
	GameObject* floatingCatFour = AddFloatingCat(Vector3(1, -17, -256));
	floatingCatFour->SetName("floatingCatFour");
}

void TutorialGame::SecondEnemySpawn() {
	GameObject* enemy1 = AddEnemyToWorld(Vector3(-30, -15, -263));
	enemy1->SetName("Enemy");
	GameObject* enemy2 = AddEnemyToWorld(Vector3(30, -15, -265));
	enemy2->SetName("Enemy");
	GameObject* enemy3 = AddEnemyToWorld(Vector3(-30, -15, -363));
	enemy3->SetName("Enemy");
	GameObject* enemy4 = AddEnemyToWorld(Vector3(30, -15, -365));
	enemy4->SetName("Enemy");
}

void TutorialGame::CheckForKeyCollection() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				if ((*i)->GetName() == "yarn" && (*j)->GetName() == "keyBlock") {
					//playerObject->SetKeyCollected(true);
					(*j)->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
					playerObject->SetKeyCollected(true);
					//Debug::Print("Key collected!", Vector2(5, 95));
				}
				//if ((*i)->GetName() == "Player" && (*j)->GetName() == "floatingCatTwo") {
					//(*j)->GetPhysicsObject()->ClearForces();
					//(*j)->SetPhysicsObject(nullptr);
					//floatingCats.push_back((*j));
					//(*j)->setFollow(true);
				//}
				if ((*i)->GetName() == "floatingCatTwo" && (*j)->GetName() == "Player") {
					(*i)->GetPhysicsObject()->ClearForces();
					//(*i)->SetPhysicsObject(nullptr);
					floatingCats.push_back((*i));
					(*i)->setFollow(true);
					(*i)->SetBoundingVolume(nullptr);
				}
				//if ((*i)->GetName() == "Player" && (*j)->GetName() == "floatingCatOne") {
					//(*j)->GetPhysicsObject()->ClearForces();
					//floatingCats.push_back((*j));
					//(*j)->setFollow(true);
					//enemySpawn = true;
				//}
				if ((*i)->GetName() == "floatingCatOne" && (*j)->GetName() == "Player") {
					(*i)->GetPhysicsObject()->ClearForces();
					floatingCats.push_back((*i));
					(*i)->setFollow(true);
					(*i)->SetBoundingVolume(nullptr);
					enemySpawn = true;
				}
				if ((*i)->GetName() == "Enemy" && (*j)->GetName() == "Player") {
					caught = true;
				}
				if ((*i)->GetName() == "Player" && (*j)->GetName() == "Enemy") {
					caught = true;
				}
				if ((*i)->GetName() == "floatingCatThree" && (*j)->GetName() == "Player") {
					(*i)->GetPhysicsObject()->ClearForces();
					//(*i)->SetPhysicsObject(nullptr);
					floatingCats.push_back((*i));
					(*i)->SetBoundingVolume(nullptr);
					(*i)->setFollow(true);
				}
				if ((*i)->GetName() == "floatingCatFour" && (*j)->GetName() == "Player") {
					(*i)->GetPhysicsObject()->ClearForces();
					//(*i)->SetPhysicsObject(nullptr);
					floatingCats.push_back((*i));
					(*i)->SetBoundingVolume(nullptr);
					(*i)->setFollow(true);
				}
				if ((*i)->GetName() == "endZone" && (*j)->GetName() == "Player") {
					victory = true;
				}
				if ((*i)->GetName() == "Player" && (*j)->GetName() == "endZone") {
					victory = true;
				}
			}
		}
	}
}

void TutorialGame::AddZigZagToWorld() {
	AddZoneToWorld(Vector3(5, -19, -201), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -202), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -203), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -204), Vector3(1, 1, 1)); //End of first straight
	AddZoneToWorld(Vector3(4, -19, -204), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(3, -19, -204), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(2, -19, -204), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(1, -19, -204), Vector3(1, 1, 1)); //End of first left
	AddZoneToWorld(Vector3(1, -19, -205), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(1, -19, -206), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(1, -19, -207), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(1, -19, -208), Vector3(1, 1, 1)); //End of second straight
	AddZoneToWorld(Vector3(2, -19, -208), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(3, -19, -208), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(4, -19, -208), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -208), Vector3(1, 1, 1)); //End of first right
	AddZoneToWorld(Vector3(5, -19, -209), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -210), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -211), Vector3(1, 1, 1));
	AddZoneToWorld(Vector3(5, -19, -212), Vector3(1, 1, 1)); //End of final straight
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	/*for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));*/
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	/*float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}*/
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	/*for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}*/
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		//Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		//Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	//Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	/*Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; //How heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; //Constraint distance
	float cubeDistance = 20; //Distance between links

	Vector3 startPos = Vector3(500, 500, 500);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);

		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);*/
}