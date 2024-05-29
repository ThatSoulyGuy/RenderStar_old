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
        enum class RootSignatureParameterType
		{
			CONSTANT_BUFFER_VIEW,
			SHADER_RESOURCE_VIEW,
            SAMPLER
		};

        struct RootSignatureParameter
		{
			RootSignatureParameterType type;
			
            UINT slot;

            static RootSignatureParameter Create(RootSignatureParameterType type, UINT slot)
            {
                return { type, slot };
            }
		};

        class RootSignature
        {

        public:

            ComPtr<ID3D12RootSignature> Generate()
            {
                for (const auto& parameter : rootParameters)
                {
                    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange;

                    switch (parameter.type)
                    {

                    case RootSignatureParameterType::CONSTANT_BUFFER_VIEW:
                        descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, parameter.slot);
                        break;

                    case RootSignatureParameterType::SHADER_RESOURCE_VIEW:
                        descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, parameter.slot);
                        break;

                    case RootSignatureParameterType::SAMPLER:
                        descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, parameter.slot);
                        break;
                    }

                    descriptorRanges.push_back(descriptorRange);

                    rootParametersDescriptors.emplace_back();
                    rootParametersDescriptors.back().InitAsDescriptorTable(1, &descriptorRanges.back());
                }

                for (int r = 0; r < rootParameters.size(); ++r)
                    rootParametersDescriptors[r].InitAsDescriptorTable(1, &descriptorRanges[r]);

                CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescription;

                if (staticSamplers.empty())
                    versionedRootSignatureDescription.Init_1_1(static_cast<UINT>(rootParametersDescriptors.size()), rootParametersDescriptors.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
                else
                    versionedRootSignatureDescription.Init_1_1(static_cast<UINT>(rootParametersDescriptors.size()), rootParametersDescriptors.data(), static_cast<UINT>(staticSamplers.size()), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                ComPtr<ID3DBlob> serializedRootSignature;
                ComPtr<ID3DBlob> errorBlob;

                HRESULT result = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDescription, &serializedRootSignature, &errorBlob);

                if (FAILED(result))
                {
                    if (errorBlob)
                    {
                        String errorMessage = static_cast<char*>(errorBlob->GetBufferPointer());
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

            static Shared<RootSignature> Create(const Vector<RootSignatureParameter>& parameters, const Vector<CD3DX12_STATIC_SAMPLER_DESC>& samplers = {})
            {
                Shared<RootSignature> out = std::make_shared<RootSignature>();

                out->Initialize(parameters, samplers);

                return out;
            }

        private:

            void Initialize(const Vector<RootSignatureParameter>& parameters, const Vector<CD3DX12_STATIC_SAMPLER_DESC>& samplers)
            {
                rootParameters = parameters;
                staticSamplers = samplers;
            }

            Vector<RootSignatureParameter> rootParameters;
            Vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges;
            Vector<CD3DX12_ROOT_PARAMETER1> rootParametersDescriptors;
            Vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
        };
	}
}