#pragma once

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/ECS/GameObjectManager.hpp"
#include "RenderStar/Render/Mesh.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/ShaderManager.hpp"
#include "RenderStar/Render/TextureManager.hpp"
#include "RenderStar/Util/CommonVersionFormat.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Render;
using namespace RenderStar::Util;

namespace RenderStar
{
	class RenderStarEngine
	{

	public:

		static void PreInitialize()
		{
			Settings::GetInstance()->Set<String>("defaultApplicationName", "RenderStar*");
			Settings::GetInstance()->Set<CommonVersionFormat>("defaultApplicationVersion", CommonVersionFormat::Create(0, 0, 9));
			Settings::GetInstance()->Set<Vector2i>("defaultWindowDimensions", { 750, 450 });
			Settings::GetInstance()->Set<WNDPROC>("defaultWindowProceadure", [](HWND handle, UINT message, WPARAM wParam, LPARAM  lParam) -> LRESULT
			{
				switch (message)
				{

				case WM_SIZE:

					if (wParam != SIZE_MINIMIZED)
					{
						if (!Renderer::GetInstance()->IsInitialized())
							return 0;
						
						Renderer::GetInstance()->Resize({ LOWORD(lParam), HIWORD(lParam) });

						Render();
					}

					return 0;

				case WM_CLOSE:
					DestroyWindow(handle);

					return 0;

				case WM_DESTROY:
					PostQuitMessage(0);

					return 0;

				default:

					return DefWindowProc(handle, message, wParam, lParam);
				}
			});

			Logger_WriteConsole("RenderStar Engine Pre-Initialized.", LogLevel::INFORMATION);
		}

		static void Initialize()                
		{
			Logger_WriteConsole("RenderStar Engine Initialized.", LogLevel::INFORMATION);

			Renderer::GetInstance()->Initialize();

			Renderer::GetInstance()->AddRenderFunction([]{GameObjectManager::GetInstance()->Render(); });

			ShaderManager::GetInstance()->Register(Shader::Create("default", "Shader/Default", RootSignature::Create({ RootSignatureParameter::Create(RootSignatureParameterType::SHADER_RESOURCE_VIEW, 0), RootSignatureParameter::Create(RootSignatureParameterType::SAMPLER, 0) })));
			TextureManager::GetInstance()->Register(Texture::Create("test", "Texture/Test.dds"));

			Shared<GameObject> square = Mesh::CreateGameObject("square", "default", "test",
			{
				{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
				{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
				{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }
			}, { 2, 1, 0, 3, 2, 0 });

			square->GetComponent<Mesh>()->Generate();
		}

		static void Update()
		{
			GameObjectManager::GetInstance()->Update();
		}

		static void Render()
		{
			Renderer::GetInstance()->Render();
		}

		static void CleanUp()
		{
			Logger_WriteConsole("RenderStar Engine Cleaned Up.", LogLevel::INFORMATION);

			GameObjectManager::GetInstance()->CleanUp();
			
			
			Renderer::GetInstance()->CleanUp();
		}
	};
}