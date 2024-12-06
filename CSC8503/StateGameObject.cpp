#include "StateGameObject.h"
//#include "StateTransition.h"
//#include "StateMachine.h"
//#include "State.h"
//#include "PhysicsObject.h"

#include "..\CSC8503CoreClasses\StateTransition.h"
#include "..\CSC8503CoreClasses\StateMachine.h"
#include "..\CSC8503CoreClasses\State.h"
#include "..\CSC8503CoreClasses\PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)-> void {
		this->MoveLeft(dt);
		}
	);

	State* stateB = new State([&](float dt)-> void {
		this->MoveRight(dt);
		}
	);

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()-> bool {
		return this->counter > 3.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()-> bool {
		return this->counter < 0.0f;
		}
	));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -100,0,0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 100,0,0 });
	counter -= dt;
}


/*StateGameObject* StateGameObject::AddStateObjectToWorld(const Vector3& position) {
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
}*/