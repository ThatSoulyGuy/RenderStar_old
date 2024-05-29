#pragma once

#include <d3d12.h>
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		struct Vertex
		{
			Vector3f position;
			Vector3f color;
			Vector3f normal;
			Vector2f textureCoordinates;

			static Array<D3D12_INPUT_ELEMENT_DESC, 4> GetInputLayout()
			{
				static Array<D3D12_INPUT_ELEMENT_DESC, 4> out =
				{
					D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				return out;
			}

			static Vertex Create(const Vector3f& position, const Vector3f& color, const Vector3f& normal, const Vector2f& textureCoordinates)
			{
				return { position, color, normal, textureCoordinates };
			}
		};
	}
}