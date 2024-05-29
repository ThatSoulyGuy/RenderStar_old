#pragma once

#include <d3dx12.h>
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace DirectX;
using namespace RenderStar::Render;

namespace RenderStar
{
	namespace Util
	{
        class RootSignature
        {

        public:

            ComPtr<ID3D12RootSignature> Generate()
            {
                for (size_t d = 0; d < descriptorRanges.size(); ++d)
                    rootParameters[d].InitAsDescriptorTable(1, &descriptorRanges[d]);

                CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescription;

                if (staticSamplers.empty())
                    versionedRootSignatureDescription.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
                else
                    versionedRootSignatureDescription.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), static_cast<UINT>(staticSamplers.size()), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                ComPtr<ID3DBlob> serializedRootSignature;
                ComPtr<ID3DBlob> errorBlob;

                HRESULT result = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDescription, &serializedRootSignature, &errorBlob);

                if (FAILED(result))
                {
                    if (errorBlob)
                    {
                        std::string errorMessage = static_cast<char*>(errorBlob->GetBufferPointer());
                        throw std::runtime_error("Failed to serialize root signature: " + errorMessage);
                    }

                    throw std::runtime_error("Failed to serialize root signature.");
                }

                ComPtr<ID3D12RootSignature> rootSignature;
                result = Renderer::GetInstance()->GetDevice()->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

                if (FAILED(result))
                    throw std::runtime_error("Failed to create root signature.");

                return rootSignature;
            }

            static Shared<RootSignature> Create(UINT constantBufferViews, UINT shaderResourceViews, UINT samplers)
            {
                Shared<RootSignature> out = std::make_shared<RootSignature>();

                out->Initialize(constantBufferViews, shaderResourceViews, samplers);

                return out;
            }

        private:
            void Initialize(UINT constantBufferViews, UINT shaderResourceViews, UINT samplers)
            {
                descriptorRanges.clear();
                rootParameters.clear();

                if (constantBufferViews > 0)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 cbvRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, constantBufferViews, 0);
                    descriptorRanges.push_back(cbvRange);

                    CD3DX12_ROOT_PARAMETER1 cbvRootParam;
                    cbvRootParam.InitAsDescriptorTable(1, &descriptorRanges.back());
                    rootParameters.push_back(cbvRootParam);
                }

                if (shaderResourceViews > 0)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shaderResourceViews, 0);
                    descriptorRanges.push_back(srvRange);

                    CD3DX12_ROOT_PARAMETER1 srvRootParam;
                    srvRootParam.InitAsDescriptorTable(1, &descriptorRanges.back());
                    rootParameters.push_back(srvRootParam);
                }

                if (samplers > 0)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 samplerRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, samplers, 0);
                    descriptorRanges.push_back(samplerRange);

                    CD3DX12_ROOT_PARAMETER1 samplerRootParam;
                    samplerRootParam.InitAsDescriptorTable(1, &descriptorRanges.back());
                    rootParameters.push_back(samplerRootParam);
                }
            }

            Vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges;
            Vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
            Vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
        };
	}
}