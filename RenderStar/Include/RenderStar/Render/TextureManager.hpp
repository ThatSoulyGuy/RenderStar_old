#pragma once

#include "RenderStar/Render/Texture.hpp"
#include "RenderStar/Util/Manager.hpp"

namespace RenderStar
{
	namespace Render
	{
		class TextureManager : public Manager<Texture, TextureManager> { };
	}
}