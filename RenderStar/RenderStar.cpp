#include "RenderStar/RenderStar.hpp"
#include "RenderStar/Core/Window.hpp"

int main()
{
	RenderStar::Core::Settings::GetInstance().Set<String>("defaultDomain", "RenderStar");
	
	RenderStar::RenderStarEngine::PreInitialize();

	RenderStar::Core::Window::GetInstance()->Create(RenderStar::Core::Settings::GetInstance().Get<String>("defaultApplicationName") + " " + RenderStar::Core::Settings::GetInstance().Get<RenderStar::Util::CommonVersionFormat>("defaultApplicationVersion").GetVersionString(), RenderStar::Core::Settings::GetInstance().Get<Vector2i>("defaultWindowDimensions"));

	RenderStar::Core::Window::GetInstance()->Show();

	RenderStar::RenderStarEngine::Initialize();

	RenderStar::Core::Window::GetInstance()->AddUpdateFunction(RenderStar::RenderStarEngine::Update);
	RenderStar::Core::Window::GetInstance()->AddUpdateFunction(RenderStar::RenderStarEngine::Render);

	RenderStar::Core::Window::GetInstance()->Run();

	RenderStar::RenderStarEngine::CleanUp();

	return 0;
}