#pragma once

#include <d3dx12.h>
#include <d3d12.h>
#include <dxc/dxcapi.h>
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/Vertex.hpp"
#include "RenderStar/Util/Loader.hpp"
#include "RenderStar/Util/RootSignature.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Render;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
        class Shader : public Component
        {

        public:

            void Bind()
            {
                auto commandList = Renderer::GetInstance()->GetCommandList();

                if (rootSignature)
                    commandList->SetGraphicsRootSignature(rootSignature.Get());

                if (pipelineState)
                    commandList->SetPipelineState(pipelineState.Get());

                ID3D12DescriptorHeap* descriptorHeaps[] = { cbvSrvUavHeap.Get(), samplerHeap.Get() };
                commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                if (!constantBuffers.empty())
                {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHeapHandle(cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                    commandList->SetGraphicsRootDescriptorTable(0, cbvSrvUavHeapHandle);
                }

                if (!textures.empty())
                {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE shaderResourceViewHandle(cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), maxConstantBuffers, cbvSrvUavDescriptorSize);
                    commandList->SetGraphicsRootDescriptorTable(0, shaderResourceViewHandle);
                }

                if (!samplers.empty())
                {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE samplerHandle(samplerHeap->GetGPUDescriptorHandleForHeapStart());
                    commandList->SetGraphicsRootDescriptorTable(1, samplerHandle);
                }
            }

            void CreateConstantBuffer(UINT bufferSize, UINT bufferIndex)
            {
                if (bufferIndex >= constantBuffers.size())
                    constantBuffers.resize(bufferIndex + 1);

                CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

                HRESULT result = Renderer::GetInstance()->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffers[bufferIndex]));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create constant buffer", false);

                constantBuffers[bufferIndex]->Map(0, nullptr, reinterpret_cast<void**>(&constantBufferDatas[bufferIndex]));

                D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDescription = {};

                constantBufferViewDescription.BufferLocation = constantBuffers[bufferIndex]->GetGPUVirtualAddress();
                constantBufferViewDescription.SizeInBytes = (bufferSize + 255) & ~255;

                CD3DX12_CPU_DESCRIPTOR_HANDLE constantBufferViewHandle(cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), bufferIndex, cbvSrvUavDescriptorSize);
                Renderer::GetInstance()->GetDevice()->CreateConstantBufferView(&constantBufferViewDescription, constantBufferViewHandle);
            }

            void UpdateConstantBuffer(const void* data, Size dataSize, UINT bufferIndex)
            {
                memcpy(constantBufferDatas[bufferIndex], data, dataSize);
            }

            void CreateTexture(ComPtr<ID3D12Resource> resource, UINT textureIndex)
            {
                if (textureIndex >= textures.size())
                    textures.resize(textureIndex + 1);

                textures[textureIndex] = resource;

                D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription = {};

                shaderResourceViewDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                shaderResourceViewDescription.Format = resource->GetDesc().Format;
                shaderResourceViewDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                shaderResourceViewDescription.Texture2D.MipLevels = resource->GetDesc().MipLevels;

                CD3DX12_CPU_DESCRIPTOR_HANDLE shaderResourceViewHandle(cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), textureIndex + maxConstantBuffers, cbvSrvUavDescriptorSize);
                Renderer::GetInstance()->GetDevice()->CreateShaderResourceView(resource.Get(), &shaderResourceViewDescription, shaderResourceViewHandle);

                Logger_WriteConsole("Created texture at index: " + std::to_string(textureIndex), LogLevel::INFORMATION);
                assert(textures[textureIndex] != nullptr);
            }

            void CreateSampler(UINT samplerIndex)
            {
                if (samplerIndex >= samplers.size())
                    samplers.resize(samplerIndex + 1);

                D3D12_SAMPLER_DESC samplerDescription = {};

                samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                samplerDescription.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                samplerDescription.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                samplerDescription.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                samplerDescription.MinLOD = 0;
                samplerDescription.MaxLOD = D3D12_FLOAT32_MAX;
                samplerDescription.MipLODBias = 0.0f;
                samplerDescription.MaxAnisotropy = 1;
                samplerDescription.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

                CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(samplerHeap->GetCPUDescriptorHandleForHeapStart(), samplerIndex, samplerDescriptorSize);

                Renderer::GetInstance()->GetDevice()->CreateSampler(&samplerDescription, samplerHandle);

                samplers[samplerIndex] = samplerHandle;
            }

            String GetName() const
            {
                return name;
            }

            String GetLocalPath() const
            {
                return localPath;
            }

            String GetDomain() const
            {
                return domain;
            }

            UnorderedMap<String, String> GetPaths() const
            {
                return
                {
                    { "vertex", vertexPath },
                    { "pixel", pixelPath },
                    { "compute", computePath },
                    { "geometry", geometryPath },
                    { "hull", hullPath },
                    { "domain", domainPath }
                };
            }

            ComPtr<ID3D12RootSignature> GetRootSignature() const
            {
                return rootSignature;
            }

            ComPtr<ID3D12PipelineState> GetPipelineState() const
            {
                return pipelineState;
            }

            void CleanUp()
            {
                for (auto& buffer : constantBuffers)
                {
                    if (buffer)
                        buffer->Unmap(0, nullptr);
                }
            }

            static Shared<Shader> Create(const String& name, const String& localPath, Shared<RootSignature> rootSignature, const String& domain = Settings::GetInstance()->Get<String>("defaultDomain"))
            {
                Shared<Shader> out = std::make_shared<Shader>();

                out->name = name;
                out->localPath = localPath;
                out->domain = domain;
                out->vertexPath = "Assets/" + domain + "/" + localPath + "Vertex.hlsl";
                out->pixelPath = "Assets/" + domain + "/" + localPath + "Pixel.hlsl";
                out->computePath = "Assets/" + domain + "/" + localPath + "Compute.hlsl";
                out->geometryPath = "Assets/" + domain + "/" + localPath + "Geometry.hlsl";
                out->hullPath = "Assets/" + domain + "/" + localPath + "Hull.hlsl";
                out->domainPath = "Assets/" + domain + "/" + localPath + "Domain.hlsl";
                out->rootSignature = rootSignature->Generate();

                out->Generate();

                return std::move(out);
            }

        private:

            void Generate()
            {
                ComPtr<IDxcUtils> utils;
                ComPtr<IDxcCompiler3> compiler;
                ComPtr<IDxcIncludeHandler> includeHandler;

                HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create DxcUtils instance", false);

                result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create DxcCompiler instance", false);

                result = utils->CreateDefaultIncludeHandler(&includeHandler);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create default include handler", false);

                CreateDescriptorHeaps();

                ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(utils, compiler, includeHandler, vertexPath, L"vs_6_6");
                ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(utils, compiler, includeHandler, pixelPath, L"ps_6_6");
                ComPtr<IDxcBlob> computeShaderBlob = CompileShader(utils, compiler, includeHandler, computePath, L"cs_6_6");
                ComPtr<IDxcBlob> geometryShaderBlob = CompileShader(utils, compiler, includeHandler, geometryPath, L"gs_6_6");
                ComPtr<IDxcBlob> hullShaderBlob = CompileShader(utils, compiler, includeHandler, hullPath, L"hs_6_6");
                ComPtr<IDxcBlob> domainShaderBlob = CompileShader(utils, compiler, includeHandler, domainPath, L"ds_6_6");

                if (!vertexShaderBlob || !pixelShaderBlob)
                    Logger_ThrowError("FAILED", "Failed to compile shader", false);

                CreateGraphicsPipelineState(vertexShaderBlob, pixelShaderBlob, geometryShaderBlob, hullShaderBlob, domainShaderBlob);
            }

            void CreateDescriptorHeaps()
            {
                D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDescription = {};

                cbvSrvUavHeapDescription.NumDescriptors = maxConstantBuffers + maxTextures;
                cbvSrvUavHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                cbvSrvUavHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                HRESULT result = Renderer::GetInstance()->GetDevice()->CreateDescriptorHeap(&cbvSrvUavHeapDescription, IID_PPV_ARGS(&cbvSrvUavHeap));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create CBV/SRV/UAV descriptor heap", false);

                cbvSrvUavDescriptorSize = Renderer::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDescription = {};

                samplerHeapDescription.NumDescriptors = maxTextures;
                samplerHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                samplerHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                result = Renderer::GetInstance()->GetDevice()->CreateDescriptorHeap(&samplerHeapDescription, IID_PPV_ARGS(&samplerHeap));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create sampler descriptor heap", false);

                samplerDescriptorSize = Renderer::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            }

            ComPtr<IDxcBlob> CompileShader(ComPtr<IDxcUtils>& utils, ComPtr<IDxcCompiler3>& compiler, ComPtr<IDxcIncludeHandler>& includeHandler, const String& path, const wchar_t* target)
            {
                InputFileStream shaderFile(path);

                if (!shaderFile.good())
                    return nullptr;

                StringStream shaderStream;

                shaderStream << shaderFile.rdbuf();

                String shaderCode = shaderStream.str();

                DxcBuffer sourceBuffer = {};

                sourceBuffer.Ptr = shaderCode.data();
                sourceBuffer.Size = shaderCode.size();
                sourceBuffer.Encoding = DXC_CP_UTF8;

                const wchar_t* arguments[] =
                {
                    L"-E", L"Main",
                    L"-T", target,
                    L"-Zi",
                    L"-Qstrip_reflect"
                };

                ComPtr<IDxcResult> results;

                HRESULT result = compiler->Compile(&sourceBuffer, arguments, _countof(arguments), includeHandler.Get(), IID_PPV_ARGS(&results));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to compile shader: " + path, false);

                ComPtr<IDxcBlobUtf8> errors;

                result = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

                if (errors && errors->GetStringLength() > 0)
                    Logger_ThrowError("FAILED", errors->GetStringPointer(), false);

                ComPtr<IDxcBlob> shaderBlob;

                result = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to retrieve shader blob: " + path, false);

                return shaderBlob;
            }

            void CreateGraphicsPipelineState(ComPtr<IDxcBlob>& vertexShaderBlob, ComPtr<IDxcBlob>& pixelShaderBlob, ComPtr<IDxcBlob>& geometryShaderBlob, ComPtr<IDxcBlob>& hullShaderBlob, ComPtr<IDxcBlob>& domainShaderBlob)
            {
                D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};

                pipelineStateDescription.VS = vertexShaderBlob ? D3D12_SHADER_BYTECODE{ vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
                pipelineStateDescription.PS = pixelShaderBlob ? D3D12_SHADER_BYTECODE{ pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
                pipelineStateDescription.GS = geometryShaderBlob ? D3D12_SHADER_BYTECODE{ geometryShaderBlob->GetBufferPointer(), geometryShaderBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
                pipelineStateDescription.HS = hullShaderBlob ? D3D12_SHADER_BYTECODE{ hullShaderBlob->GetBufferPointer(), hullShaderBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
                pipelineStateDescription.DS = domainShaderBlob ? D3D12_SHADER_BYTECODE{ domainShaderBlob->GetBufferPointer(), domainShaderBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};

                pipelineStateDescription.pRootSignature = rootSignature.Get();
                pipelineStateDescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
                pipelineStateDescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                pipelineStateDescription.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                pipelineStateDescription.InputLayout = { Vertex::GetInputLayout().data(), (UINT)Vertex::GetInputLayout().size() };
                pipelineStateDescription.SampleMask = UINT_MAX;
                pipelineStateDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                pipelineStateDescription.NumRenderTargets = 1;
                pipelineStateDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
                pipelineStateDescription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
                pipelineStateDescription.SampleDesc.Count = 1;

                HRESULT result = Renderer::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState));

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to create graphics pipeline state", true);
            }

            String name;
            String localPath;
            String domain;

            String vertexPath, pixelPath;
            String computePath, geometryPath;
            String hullPath, domainPath;

            ComPtr<ID3D12PipelineState> pipelineState;
            ComPtr<ID3D12RootSignature> rootSignature;

            Vector<ComPtr<ID3D12Resource>> constantBuffers;
            Vector<UINT8*> constantBufferDatas;
            Vector<ComPtr<ID3D12Resource>> textures;
            Vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> samplers;

            ComPtr<ID3D12DescriptorHeap> cbvSrvUavHeap;
            ComPtr<ID3D12DescriptorHeap> samplerHeap;

            UINT cbvSrvUavDescriptorSize = 0;
            UINT samplerDescriptorSize = 0;

            static const UINT maxConstantBuffers = 8;
            static const UINT maxTextures = 8;
        };
	}
}