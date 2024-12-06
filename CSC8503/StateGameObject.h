#pragma once
#include "..\CSC8503CoreClasses\GameObject.h" //Original import was just GameObject.h
                                              //I just added the ..\CSC8503CoreClasses

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            StateMachine* stateMachine;
            float counter;

            StateGameObject* AddStateObjectToWorld(const Vector3& position);
            StateGameObject* testStateObject;
        };
    }
}
