#pragma once

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
    namespace Core
    {
        class Window
        {

        public:

            void Create(const String& title, const Vector2i& dimensions)
            {
                lastTime = Clock::now();

                WNDCLASSEX windowClass = { 0 };

                windowClass.cbSize = sizeof(WNDCLASSEX);
                windowClass.style = CS_HREDRAW | CS_VREDRAW;
                windowClass.lpfnWndProc = Settings::GetInstance()->Get<WNDPROC>("defaultWindowProceadure");
                windowClass.hInstance = GetModuleHandle(nullptr);
                windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
                windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
                windowClass.lpszClassName = "RenderStar";

                if (!RegisterClassEx(&windowClass))
                    Logger_ThrowError("nullptr", "Failed to register window class.", true);

                handle = CreateWindowEx(0, windowClass.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, dimensions.x, dimensions.y, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
            }

            void Show() const
            {
                ShowWindow(handle, SW_SHOW);
            }

            void SetTitle(const String& title) const
            {
                SetWindowText(handle, title.c_str());
            }

            void SetDimensions(const Vector2i& dimensions) const
            {
                RECT rect = { 0, 0, dimensions.x, dimensions.y };

                AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

                SetWindowPos(handle, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
            }

            void SetPosition(const Vector2i& position) const
            {
                SetWindowPos(handle, nullptr, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }

            String GetTitle() const
            {
                int length = GetWindowTextLength(handle);

                String title;
                title.resize(length);

                GetWindowText(handle, &title[0], length);

                return title;
            }

            Vector2i GetClientDimensions() const
            {
                RECT rect;

                GetClientRect(handle, &rect);

                return { rect.right - rect.left, rect.bottom - rect.top };
            }

            float GetDeltaTime()
            {
                TimePoint currentTime = Clock::now();

                Duration deltaTime = currentTime - lastTime;
                lastTime = currentTime;

                return deltaTime.count();
            }

            float GetAspectRatio() const
            {
                Vector2i dimensions = GetClientDimensions();

                return static_cast<float>(dimensions.x) / static_cast<float>(dimensions.y);
            }

            Vector2i GetPosition() const
            {
                RECT rect = { 0 };

                GetWindowRect(handle, &rect);

                return { rect.left, rect.top };
            }

            HWND GetHandle() const
            {
                return handle;
            }

            void AddUpdateFunction(Function<void()> function)
            {
                LockGuard<Mutex> lock(updateMutex);

                updateFunctions.push_back(function);
            }

            void Run()
            {
                running = true;
                updateThread = Thread([this] { UpdateLoop(); });

                MSG message = { 0 };

                while (GetMessage(&message, nullptr, 0, 0) > 0)
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                running = false;
                updateThread.join();
            }

            void CleanUp() const
            {
                UnregisterClass("RenderStar", GetModuleHandle(nullptr));

                if (handle)
                    DestroyWindow(handle);
            }

            static Shared<Window> GetInstance()
            {
                static Shared<Window> instance(new Window());

                return instance;
            }

        private:

            void UpdateLoop()
            {
                while (running)
                {
                    {
                        LockGuard<Mutex> lock(updateMutex);

                        for (auto& function : updateFunctions)
                            function();
                    }

                    std::this_thread::sleep_for(Milliseconds(16));
                }
            }

            Vector<Function<void()>> updateFunctions;

            Thread updateThread;
            Mutex updateMutex;

            HWND handle = nullptr;

            HighResolutionTimePoint lastTime;

            volatile bool running = false;
        };
    }
}