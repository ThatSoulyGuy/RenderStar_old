#pragma once

#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Math/Transform.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Math;
using namespace RenderStar::Render;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace ECS
	{
        class GameObject : public EnableShared<GameObject>
        {

        public:

            String name;

            bool isActive = true;

            Shared<GameObject> parent;

            List<Shared<GameObject>> children;
            UnorderedMap<TypeIndex, Shared<Component>> components;

            template<typename T>
            Shared<T> AddComponent(Shared<T> component)
            {
                if (!component)
                    return nullptr;

                components[TypeIndex(typeid(T))] = component;
                component->gameObject = shared_from_this();
                component->Initialize();

                return component;
            }

            template<typename T>
            Shared<T> GetComponent()
            {
                auto iterator = components.find(TypeIndex(typeid(T)));

                if (iterator != components.end())
                    return std::dynamic_pointer_cast<T>(iterator->second);

                return nullptr;
            }

            template<typename T>
            bool HasComponent()
            {
                return components.count(TypeIndex(typeid(T))) > 0;
            }

            Shared<GameObject> AddChild(Shared<GameObject> child)
            {
                if (child && child->parent != shared_from_this())
                {
                    children.push_back(child);

                    child->parent = shared_from_this();
                }

                return child;
            }

            Shared<GameObject> GetChild(const String& name)
            {
                for (auto& child : children)
                    if (child->name == name)
                        return child;

                return nullptr;
            }

            void RemoveChild(Shared<GameObject> child)
            {
                if (child)
                {
                    child->parent.reset();

                    children.remove_if([&child](const Shared<GameObject>& gameObject)
                        {
                            return gameObject == child;
                        });
                }
            }

            void Update()
            {
                if (parent != nullptr)
                    GetComponent<Transform>()->SetParent(parent->GetComponent<Transform>());

                if (!isActive)
                    return;

                for (auto& component : components)
                    component.second->Update();

                for (auto& child : children)
                    child->Update();
            }

            void Render()
            {
                if (!isActive)
                    return;

                for (auto& component : components)
                    component.second->Render();

                for (auto& child : children)
                    child->Render();
            }

            void CleanUp()
            {
                for (auto& component : components)
                    component.second->CleanUp();

                components.clear();

                for (auto& child : children)
                    child->CleanUp();

                children.clear();
            }
        };
	}
}