#pragma once

#include "RenderStar/ECS/GameObjectManager.hpp"
#include "RenderStar/Render/ShaderManager.hpp"
#include "RenderStar/Render/TextureManager.hpp"
#include "RenderStar/Render/Vertex.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::ECS;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
        class Mesh : public Component
        {

        public:

            void Initialize() override
            {
                shader = gameObject->GetComponent<Shader>();
                texture = gameObject->GetComponent<Texture>();

                if (!shader || !texture)
                {
                    Logger_ThrowError("Initialization Error", "Shader or Texture component missing from GameObject", true);
                    return;
                }

                if (!texture->GetRaw())
                {
                    Logger_ThrowError("Initialization Error", "Texture resource is invalid", true);
                    return;
                }

                shader->CreateTexture(texture->GetRaw(), 0);
                shader->CreateSampler(0);
            }

            void Generate()
            {
                CreateVertexBuffer();
                CreateIndexBuffer();
            }

            void Render() override
            {
                if (!shader)
                    return;

                texture->Bind();
                shader->Bind();

                UINT stride = sizeof(Vertex);
                UINT offset = 0;

                ComPtr<ID3D12GraphicsCommandList> commandList = Renderer::GetInstance()->GetCommandList();

                commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
                commandList->IASetIndexBuffer(&indexBufferView);
                commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                commandList->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
            }

            String GetName() const
            {
                return name;
            }

            Pair<Vector<Vertex>, Vector<uint>> GetMeshData() const
            {
                return { vertices, indices };
            }

            void CleanUp() override
            {
                vertices.clear();
                indices.clear();
            }

            static Shared<Mesh> Create(const String& name, const Vector<Vertex>& vertices, const Vector<uint>& indices)
            {
                Shared<Mesh> out = std::make_shared<Mesh>();

                out->name = name;
                out->vertices = vertices;
                out->indices = indices;

                return out;
            }

            static Shared<GameObject> CreateGameObject(const String& name, const String& shader, const String& texture, const Vector<Vertex>& vertices, const Vector<uint>& indices)
            {
                Shared<GameObject> out = GameObjectManager::GetInstance()->Create(name);

                out->AddComponent(ShaderManager::GetInstance()->Get(shader));
                out->AddComponent(TextureManager::GetInstance()->Get(texture));
                out->AddComponent(Mesh::Create(name, vertices, indices));

                return out;
            }

        private:

            void CreateVertexBuffer()
            {
                auto device = Renderer::GetInstance()->GetDevice();

                const UINT vertexBufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));

                CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

                HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create vertex buffer", true);

                void* vertexData = nullptr;

                CD3DX12_RANGE readRange(0, 0);
                vertexBuffer->Map(0, &readRange, &vertexData);
                memcpy(vertexData, vertices.data(), vertexBufferSize);
                vertexBuffer->Unmap(0, nullptr);

                vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
                vertexBufferView.StrideInBytes = sizeof(Vertex);
                vertexBufferView.SizeInBytes = vertexBufferSize;
            }

            void CreateIndexBuffer()
            {
                auto device = Renderer::GetInstance()->GetDevice();

                const UINT indexBufferSize = static_cast<UINT>(indices.size() * sizeof(uint));

                CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

                HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create index buffer", true);

                void* indexData = nullptr;
                CD3DX12_RANGE readRange(0, 0);
                indexBuffer->Map(0, &readRange, &indexData);
                memcpy(indexData, indices.data(), indexBufferSize);
                indexBuffer->Unmap(0, nullptr);

                indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
                indexBufferView.Format = DXGI_FORMAT_R32_UINT;
                indexBufferView.SizeInBytes = indexBufferSize;
            }

            String name;

            Shared<Shader> shader;
            Shared<Texture> texture;

            Vector<Vertex> vertices;
            Vector<uint> indices;

            ComPtr<ID3D12Resource> vertexBuffer;
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

            ComPtr<ID3D12Resource> indexBuffer;
            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
        };
	}
}