#pragma once

#include <d3dx12.h>
#include <d3d12.h>
#include <dxc/dxcapi.h>
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/Vertex.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Render;

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
            }

            void CreateConstantBuffer(UINT bufferSize) 
            {
                CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

                HRESULT result = Renderer::GetInstance()->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer));

                if (FAILED(result)) 
                    Logger_ThrowError("D3D12", "Failed to create constant buffer", true);
                
                constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&constantBufferData));
            }

            void UpdateConstantBuffer(const void* data, Size dataSize) 
            {
                memcpy(constantBufferData, data, dataSize);
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
				if (constantBuffer) 
					constantBuffer->Unmap(0, nullptr);
			}

            static Shared<Shader> Create(const String& name, const String& localPath, const String& domain = Settings::GetInstance()->Get<String>("defaultDomain")) 
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
                    Logger_ThrowError("FAILED", "Failed to create DxcUtils instance", true);
                
                result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

                if (FAILED(result)) 
                    Logger_ThrowError("FAILED", "Failed to create DxcCompiler instance", true);
                
                result = utils->CreateDefaultIncludeHandler(&includeHandler);

                if (FAILED(result)) 
                    Logger_ThrowError("FAILED", "Failed to create default include handler", true);
                
                CreateRootSignature();

                ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(utils, compiler, includeHandler, vertexPath, L"vs_6_6");
                ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(utils, compiler, includeHandler, pixelPath, L"ps_6_6");
                ComPtr<IDxcBlob> computeShaderBlob = CompileShader(utils, compiler, includeHandler, computePath, L"cs_6_6");
                ComPtr<IDxcBlob> geometryShaderBlob = CompileShader(utils, compiler, includeHandler, geometryPath, L"gs_6_6");
                ComPtr<IDxcBlob> hullShaderBlob = CompileShader(utils, compiler, includeHandler, hullPath, L"hs_6_6");
                ComPtr<IDxcBlob> domainShaderBlob = CompileShader(utils, compiler, includeHandler, domainPath, L"ds_6_6");

                if(!vertexShaderBlob || !pixelShaderBlob)
                    Logger_ThrowError("nullptr", "Failed to compile shader", true);

                CreateGraphicsPipelineState(vertexShaderBlob, pixelShaderBlob, geometryShaderBlob, hullShaderBlob, domainShaderBlob);
            }

            void CreateRootSignature() 
            {
                CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};

                rootParameters[0].InitAsConstantBufferView(0);

                CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription = {};

                rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                ComPtr<ID3DBlob> signature;
                ComPtr<ID3DBlob> error;

                HRESULT result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &signature, &error);

                if (FAILED(result)) 
                    Logger_ThrowError("D3D12", "Failed to serialize root signature", true);
                
                result = Renderer::GetInstance()->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

                if (FAILED(result)) 
                    Logger_ThrowError("D3D12", "Failed to create root signature", true);
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
                    Logger_ThrowError("FAILED", "Failed to compile shader: " + path, true);
                
                ComPtr<IDxcBlobUtf8> errors;
                result = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

                if (errors && errors->GetStringLength() > 0) 
                    Logger_ThrowError("FAILED", errors->GetStringPointer(), false);
                
                ComPtr<IDxcBlob> shaderBlob;
                result = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);

                if (FAILED(result))
                    Logger_ThrowError("FAILED", "Failed to retrieve shader blob: " + path, true);
                
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
                pipelineStateDescription.InputLayout = { Vertex::GetInputLayout().data(), (uint)Vertex::GetInputLayout().size() };
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

            ComPtr<ID3D12PipelineState> vertexShader;
            ComPtr<ID3D12PipelineState> pixelShader;
            ComPtr<ID3D12PipelineState> computeShader;
            ComPtr<ID3D12PipelineState> geometryShader;
            ComPtr<ID3D12PipelineState> hullShader;
            ComPtr<ID3D12PipelineState> domainShader;

            ComPtr<ID3D12PipelineState> pipelineState;
            ComPtr<ID3D12RootSignature> rootSignature;

            ComPtr<ID3D12Resource> constantBuffer;
            UINT8* constantBufferData = nullptr;
        };
	}
}