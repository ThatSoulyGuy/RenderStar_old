#pragma once

#include "RenderStar/ECS/GameObject.hpp"
#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Math/Transform.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Math;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace ECS
	{
        class GameObjectManager 
        {

        public:

            Shared<GameObject> Create(const String& name)
            {
                Shared<GameObject> out = std::make_shared<GameObject>();

                out->name = name;
                out->AddComponent(Transform::Create());
                registeredGameObjects[name] = out;

                return out;
            }

            Shared<GameObject> Get(const String& name)
            {
                auto iterator = registeredGameObjects.find(name);

                if (iterator != registeredGameObjects.end())
                    return iterator->second;

                return nullptr;
            }

            void Update()
            {
                for (auto& gameObject : registeredGameObjects)
                    gameObject.second->Update();
            }

            void Render()
            {
                for (auto& gameObject : registeredGameObjects)
                    gameObject.second->Render();
            }

            void CleanUp()
            {
                for (auto& gameObject : registeredGameObjects)
                    gameObject.second->CleanUp();
            }

            void Remove(const String& name)
            {
                auto iterator = registeredGameObjects.find(name);

                if (iterator != registeredGameObjects.end())
                {
                    iterator->second->CleanUp();
                    registeredGameObjects.erase(iterator);
                }
            }

            static Shared<GameObjectManager> GetInstance()
            {
                static Shared<GameObjectManager> instance = std::make_shared<GameObjectManager>() ;

                return instance;
            }

        private:

            UnorderedMap<String, Shared<GameObject>> registeredGameObjects;

            static GameObjectManager instance;

        };
	}
}