#pragma once

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Util/CommonVersionFormat.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Render;
using namespace RenderStar::Util;

namespace RenderStar
{
	class RenderStarEngine
	{

	public:

		static void PreInitialize()
		{
			Settings::GetInstance().Set<String>("defaultApplicationName", "RenderStar*");
			Settings::GetInstance().Set<CommonVersionFormat>("defaultApplicationVersion", CommonVersionFormat::Create(0, 0, 1));
			Settings::GetInstance().Set<Vector2i>("defaultWindowDimensions", { 750, 450 });
			Settings::GetInstance().Set<WNDPROC>("defaultWindowProceadure", [](HWND handle, UINT message, WPARAM wParam, LPARAM  lParam) -> LRESULT
			{
				switch (message)
				{

				case WM_SIZE:

					if (wParam != SIZE_MINIMIZED)
					{
						if (!Renderer::GetInstance()->IsInitialized())
							break;

						//Render();

						//Renderer::GetInstance()->Resize({ LOWORD(lParam), HIWORD(lParam) });
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
		}

		static void Update()
		{

		}

		static void Render()
		{
			Renderer::GetInstance()->PreRender();
			Renderer::GetInstance()->PostRender();
		}

		static void CleanUp()
		{
			Logger_WriteConsole("RenderStar Engine Cleaned Up.", LogLevel::INFORMATION);

			Renderer::GetInstance()->CleanUp();
		}
	};
}