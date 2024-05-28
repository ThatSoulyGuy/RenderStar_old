#pragma once

#include "RenderStar/Render/Shader.hpp"
#include "RenderStar/Util/Manager.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		class ShaderManager : public Manager<Shader, ShaderManager> {};
	}
}