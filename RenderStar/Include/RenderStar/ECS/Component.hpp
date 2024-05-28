#pragma once

#include "RenderStar/Util/Typedefs.hpp"

namespace RenderStar
{
    namespace Render
    {
        class Camera;
    }
}

using namespace RenderStar::Render;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace ECS
	{
		class GameObject;
		
        class Component
        {

        public:

            virtual ~Component() { }

            virtual void Initialize() { }

            virtual void Update() { }
            virtual void Render() { }

            virtual void CleanUp() { }

            Shared<GameObject> gameObject;
        };
	}
}