#pragma once

#include <wincodec.h>
#include <DirectXTex.h>
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace DirectX;
using namespace RenderStar::Render;

namespace RenderStar
{
	namespace Util
	{
		class Loader
		{

		public:

            static HRESULT LoadTexture(const String& path, ID3D12Resource** texture)
            {
                ComPtr<ID3D12Resource> resource = LoadTexture(path);

                texture = &resource;

                return S_OK;    
            }

            static ComPtr<ID3D12Resource> LoadTexture(const String& path)
            {
                ScratchImage rawImage;
                HRESULT result = LoadFromDDSFile(WString(path.begin(), path.end()).c_str(), DDS_FLAGS_NONE, nullptr, rawImage);

                if (FAILED(result))
                    throw std::runtime_error("Failed to load DDS image.");

                auto device = Renderer::GetInstance()->GetDevice();
                const TexMetadata& metadata = rawImage.GetMetadata();

                D3D12_RESOURCE_DESC textureDescription = {};

                textureDescription.MipLevels = static_cast<UINT16>(metadata.mipLevels);
                textureDescription.Format = metadata.format;
                textureDescription.Width = static_cast<UINT>(metadata.width);
                textureDescription.Height = static_cast<UINT>(metadata.height);
                textureDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
                textureDescription.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
                textureDescription.SampleDesc.Count = 1;
                textureDescription.SampleDesc.Quality = 0;
                textureDescription.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

                CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
                ComPtr<ID3D12Resource> texture;

                result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &textureDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

                if (FAILED(result))
                    throw std::runtime_error("Failed to create texture resource.");

                const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, metadata.mipLevels);

                CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

                ComPtr<ID3D12Resource> textureUploadHeap;

                result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap));

                if (FAILED(result))
                    throw std::runtime_error("Failed to create texture upload heap.");

                Vector<D3D12_SUBRESOURCE_DATA> subresources(metadata.mipLevels);

                for (Size l = 0; l < metadata.mipLevels; ++l)
                {
                    const Image* image = rawImage.GetImage(l, 0, 0);

                    subresources[l].pData = image->pixels;
                    subresources[l].RowPitch = image->rowPitch;
                    subresources[l].SlicePitch = image->slicePitch;
                }

                auto renderer = Renderer::GetInstance();
                auto commandList = renderer->GetCommandList();
                auto commandAllocator = renderer->GetCommandAllocator();

                result = commandAllocator->Reset();

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to reset command allocator.", true);

                result = commandList->Reset(commandAllocator.Get(), nullptr);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to reset command list.", true);

                UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                commandList->ResourceBarrier(1, &barrier);

                result = commandList->Close();

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to close command list.", true);

                ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
                renderer->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

                renderer->WaitForPreviousFrame();

                return texture;
            }
		};
	}
}