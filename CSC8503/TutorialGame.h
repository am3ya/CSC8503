#include "../NCLCoreClasses/KeyboardMouseController.h"
#include "../CSC8503CoreClasses/GameObject.h"
#include "../CSC8503CoreClasses/PhysicsObject.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"

namespace NCL {
	namespace CSC8503 {
		enum class GameState { Menu, Gameplay, End };


		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			void InitGameWorld();

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void ShowMenu();
			//void UpdatePlayer(float dt);

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void BridgeConstraintTest();
			void AddZigZagToWorld();

			void InitDefaultFloor();

			void UpdatePlayer(float dt);

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void CheckForKeyCollection();
			
			GameObject* AddFloorToWorld(const Vector3& position); //This was const
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddZoneToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddYarnToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* playerObject;
			PhysicsObject* playerPhysics;
			//KeyboardMouseController inputController;

#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;

			KeyboardMouseController controller;

			GameState currentState;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* catMesh = nullptr;
			Mesh* kittenMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 7, 17); //(0,7,20) was a good one
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
			float cameraDistance = 10.0f;

			GameObject* objClosest = nullptr;

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;
		};
	}
}
