#pragma once

#include <DirectXTex.h>
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Util/Loader.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
        class Texture : public Component
        {
        public:

            void Bind()
            {
                auto commandList = Renderer::GetInstance()->GetCommandList();

                if (currentState != (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
                {
                    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), currentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                    commandList->ResourceBarrier(1, &barrier);

                    currentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                }
            }

            void CleanUp() override
            {
                texture.Reset();
                textureUploadHeap.Reset();
            }

            ComPtr<ID3D12Resource> GetRaw()
            {
                return texture;
            }

            String GetName()
            {
                return name;
            }

            static Shared<Texture> Create(const String& name, const String& localPath, const String& domain = Settings::GetInstance()->Get<String>("defaultDomain"))
            {
                Shared<Texture> out = std::make_shared<Texture>();

                out->name = name;
                out->localPath = localPath;
                out->path = "Assets/" + domain + "/" + localPath;
                out->Generate();

                return std::move(out);
            }

        private:

            void Generate()
            {
                HRESULT result = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to initialize COM library.", false);

                TexMetadata metadata;
                ScratchImage scratchImage;

                result = LoadFromDDSFile(WString(path.begin(), path.end()).c_str(), DDS_FLAGS_NONE, &metadata, scratchImage);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to load DDS file.", false);

                const Image* image = scratchImage.GetImage(0, 0, 0);

                auto device = Renderer::GetInstance()->GetDevice();

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

                result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &textureDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create texture resource.", false);

                const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, metadata.mipLevels);

                CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

                result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create texture upload heap.", false);

                D3D12_SUBRESOURCE_DATA textureData = {};

                textureData.pData = image->pixels;
                textureData.RowPitch = static_cast<LONG_PTR>(image->rowPitch);
                textureData.SlicePitch = textureData.RowPitch * static_cast<LONG_PTR>(image->height);

                auto commandList = Renderer::GetInstance()->GetCommandList();

                Renderer::GetInstance()->OpenCommandList();

                UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, metadata.mipLevels, &textureData);

                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                commandList->ResourceBarrier(1, &barrier);

                Renderer::GetInstance()->CloseCommandList();
                Renderer::GetInstance()->WaitForPreviousFrame();

                currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            }

            String name;
            String localPath;
            String path;

            ComPtr<ID3D12Resource> texture;
            ComPtr<ID3D12Resource> textureUploadHeap;

            D3D12_SUBRESOURCE_DATA subresourceData = {};

            D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_COPY_DEST;
        };
	}
}