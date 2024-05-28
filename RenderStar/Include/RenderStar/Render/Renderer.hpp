#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/Core/Window.hpp"

using namespace RenderStar::Core;

namespace RenderStar
{
	namespace Render
	{
        class Renderer
        {
        public:
            void Initialize()
            {
                EnableDebugLayer();
                CreateDevice();
                CreateCommandQueue();
                CreateSwapChain();
                CreateRenderTargetView();
                CreateCommandAllocatorsAndLists();
                CreateFenceAndEvent();

                isInitialized = true;
            }

            void PreRender()
            {
                PopulateCommandList();

                ID3D12CommandList* ppCommandLists[] = { commandLists[frameIndex].Get()};

                commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
            }

            void PostRender()
            {
                swapChain->Present(1, 0);

                WaitForPreviousFrame();
            }

            bool IsInitialized() const
            {
                return isInitialized;
            }

            void Resize(const Vector2i& dimensions)
            {
                WaitForPreviousFrame();

                for (UINT f = 0; f < frameCount; f++)
                    renderTargets[f].Reset();

                HRESULT result = swapChain->ResizeBuffers(frameCount, dimensions.x, dimensions.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to resize swap chain buffers.", true);

                CreateRenderTargetView();
            }

            ComPtr<ID3D12Device2> GetDevice() const
			{
				return device;
			}

            ComPtr<ID3D12CommandQueue> GetCommandQueue() const
            {
                return commandQueue;
			}

            ComPtr<IDXGISwapChain4> GetSwapChain() const
			{
				return swapChain;
			}

            ComPtr<ID3D12DescriptorHeap> GetRenderTargetViewHeap() const
            {
                return renderTargetViewHeap;
            }

            ComPtr<ID3D12Resource> GetRenderTarget() const
			{
				return renderTargets[frameIndex];
			}

            ComPtr<ID3D12Resource> GetRenderTarget(UINT index) const
			{
				return renderTargets[index];
			}

            ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const
            {
                return commandAllocators[frameIndex];
            }

            ComPtr<ID3D12CommandAllocator> GetCommandAllocator(UINT index) const
            {
				return commandAllocators[index];
			}

            ComPtr<ID3D12GraphicsCommandList> GetCommandList() const
            {
                return commandLists[frameIndex];
            }

            ComPtr<ID3D12GraphicsCommandList> GetCommandList(UINT index) const
            {
                return commandLists[index];
            }

            ComPtr<ID3D12Fence> GetFence() const
			{
				return fence;
			}

            HANDLE GetFenceEvent() const
			{
				return fenceEvent;
			}

            UINT GetRenderTargetHeapDescriptorSize() const
            {
                return renderTargetHeapDescriptorSize;
            }

            UINT GetFrameIndex() const
			{
				return frameIndex;
			}

            void CleanUp()
            {
                WaitForPreviousFrame();

                CloseHandle(fenceEvent);
            }

            static Shared<Renderer> GetInstance()
            {
                static Shared<Renderer> instance = std::make_shared<Renderer>();

                return instance;
            }

        private:

            void EnableDebugLayer()
            {
#ifdef _DEBUG
                ComPtr<ID3D12Debug> debugController;

                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                    debugController->EnableDebugLayer();
#endif
            }

            void CreateDevice()
            {
                ComPtr<IDXGIFactory4> factory;

                UINT createFactoryFlags = 0;

#if defined(_DEBUG)
                createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

                HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory));

                if (FAILED(result))
                {
                    Logger_ThrowError("FAILED", "Failed to create DXGIFactory.\n", true);
                    return;
                }

                ComPtr<IDXGIAdapter1> hardwareAdapter1;
                ComPtr<IDXGIAdapter4> hardwareAdapter;

                for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &hardwareAdapter1); ++adapterIndex)
                {
                    hardwareAdapter1.As(&hardwareAdapter);

                    DXGI_ADAPTER_DESC1 adapterDescription;
                    hardwareAdapter->GetDesc1(&adapterDescription);

                    if (adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                        continue;

                    if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                        break;
                }

                if (!hardwareAdapter)
                {
                    result = factory->EnumWarpAdapter(IID_PPV_ARGS(&hardwareAdapter));

                    if (FAILED(result))
                    {
                        Logger_ThrowError("FAILED", "Failed to enumerate WARP adapter.\n", true);
                        return;
                    }
                }

                result = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

                if (FAILED(result))
                {
                    Logger_ThrowError("FAILED", "Failed to create D3D12 device.\n", true);
                    return;
                }
            }

            void CreateCommandQueue()
            {
                D3D12_COMMAND_QUEUE_DESC commandQueueDescription = {};

                commandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                commandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

                HRESULT result = device->CreateCommandQueue(&commandQueueDescription, IID_PPV_ARGS(&commandQueue));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create command queue.", true);
            }

            void CreateSwapChain()
            {
                ComPtr<IDXGIFactory4> factory;

                HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create DXGIFactory.", true);

                DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};

                swapChainDescription.BufferCount = frameCount;
                swapChainDescription.Width = Window::GetInstance()->GetClientDimensions().x;
                swapChainDescription.Height = Window::GetInstance()->GetClientDimensions().y;
                swapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                swapChainDescription.SampleDesc.Count = 1;

                ComPtr<IDXGISwapChain1> swapChain;

                result = factory->CreateSwapChainForHwnd(commandQueue.Get(), Window::GetInstance()->GetHandle(), &swapChainDescription, nullptr, nullptr, &swapChain);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create swap chain.", true);

                swapChain.As(&this->swapChain);
                frameIndex = this->swapChain->GetCurrentBackBufferIndex();
            }

            void CreateRenderTargetView()
            {
                D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDescription = {};

                renderTargetViewHeapDescription.NumDescriptors = frameCount;
                renderTargetViewHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                renderTargetViewHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                HRESULT result = device->CreateDescriptorHeap(&renderTargetViewHeapDescription, IID_PPV_ARGS(&renderTargetViewHeap));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create descriptor heap.", true);

                renderTargetHeapDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

                CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle(renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());

                for (UINT f = 0; f < frameCount; f++)
                {
                    result = swapChain->GetBuffer(f, IID_PPV_ARGS(&renderTargets[f]));

                    if (FAILED(result))
                        Logger_ThrowError("FAILED", "Failed to get swap chain buffer.", true);

                    device->CreateRenderTargetView(renderTargets[f].Get(), nullptr, renderTargetViewHandle);
                    renderTargetViewHandle.Offset(1, renderTargetHeapDescriptorSize);
                }
            }

            void CreateCommandAllocatorsAndLists()
            {
                for (UINT f = 0; f < frameCount; f++)
                {
                    HRESULT result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[f]));

                    if (FAILED(result))
                        Logger_ThrowError("FAILED", "Failed to create command allocator.", true);

                    result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[f].Get(), nullptr, IID_PPV_ARGS(&commandLists[f]));

                    if (FAILED(result))
                        Logger_ThrowError("FAILED", "Failed to create command list.", true);

                    commandLists[f]->Close();
                }
            }

            void CreateFenceAndEvent()
            {
                HRESULT result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create fence.", true);

                fenceValue = 1;
                fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

                if (fenceEvent == nullptr)
                    Logger_ThrowError("FAILED", "Failed to create fence event.", true);
            }

            void PopulateCommandList()
            {
                UINT currentFrameIndex = frameIndex;
                HRESULT result = commandAllocators[currentFrameIndex]->Reset();

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to reset command allocator.", true);

                result = commandLists[currentFrameIndex]->Reset(commandAllocators[currentFrameIndex].Get(), nullptr);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to reset command list.", true);

                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
                commandLists[currentFrameIndex]->ResourceBarrier(1, &barrier);

                CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle(renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIndex, renderTargetHeapDescriptorSize);
                commandLists[currentFrameIndex]->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, nullptr);

                const float clearColor[] = { 0.0f, 0.45f, 0.75f, 1.0f };
                commandLists[currentFrameIndex]->ClearRenderTargetView(renderTargetViewHandle, clearColor, 0, nullptr);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
                commandLists[currentFrameIndex]->ResourceBarrier(1, &barrier);

                result = commandLists[currentFrameIndex]->Close();

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to close command list.", true);
            }

            void WaitForPreviousFrame()
            {
                const UINT64 fence = fenceValue;

                HRESULT result = commandQueue->Signal(this->fence.Get(), fence);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to signal fence.", true);

                fenceValue++;

                if (this->fence->GetCompletedValue() < fence)
                {
                    result = this->fence->SetEventOnCompletion(fence, fenceEvent);

                    if (FAILED(result))
                        Logger_ThrowError("FAILED", "Failed to set event on completion.", true);

                    WaitForSingleObject(fenceEvent, INFINITE);
                }

                frameIndex = swapChain->GetCurrentBackBufferIndex();
            }

            static const UINT frameCount = 2;

            ComPtr<ID3D12Device2> device;
            ComPtr<ID3D12CommandQueue> commandQueue;
            ComPtr<IDXGISwapChain4> swapChain;
            ComPtr<ID3D12DescriptorHeap> renderTargetViewHeap;
            ComPtr<ID3D12Resource> renderTargets[frameCount];
            ComPtr<ID3D12CommandAllocator> commandAllocators[frameCount];
            ComPtr<ID3D12GraphicsCommandList> commandLists[frameCount];
            ComPtr<ID3D12Fence> fence;
            HANDLE fenceEvent = nullptr;
            UINT renderTargetHeapDescriptorSize = 0;
            UINT frameIndex = 0;
            UINT64 fenceValue = 0;

            bool isInitialized = false;
        };
	}
}