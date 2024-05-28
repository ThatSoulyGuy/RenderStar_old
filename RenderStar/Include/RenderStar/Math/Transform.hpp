#pragma once

#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::ECS;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Math
	{
		class Transform : public Component
		{
            
        public:

            void SetLocalPosition(const XMVector& position) 
            {
                localPosition = position;
                MarkDirty();
            }

            void SetLocalPosition(const Vector3f& position)
            {
                localPosition = DirectX::XMLoadFloat3(&position);
                MarkDirty();
            }

            void SetLocalRotation(const XMVector& rotation) 
            {
                localRotation = rotation;
                MarkDirty();
            }

            void SetLocalRotation(const Vector3f& rotation)
			{
				localRotation = DirectX::XMLoadFloat3(&rotation);
				MarkDirty();
			}

            void SetLocalScale(const XMVector& scale) 
            {
                localScale = scale;
                MarkDirty();
            }

            void SetLocalScale(const Vector3f& scale)
            {
				localScale = DirectX::XMLoadFloat3(&scale);
				MarkDirty();
            }

            XMVector GetLocalPosition() const
			{
				return localPosition;
			}

            XMVector GetLocalRotation() const
            {
				return localRotation;
            }

            XMVector GetLocalScale() const
            {
				return localScale;
            }

            XMVector GetWorldPosition()
            {
                UpdateWorldMatrixIfNeeded();
                return worldMatrix.r[3];
            }

            XMVector GetForwardVector() const
            {
                return DirectX::XMVector3Normalize(XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), worldMatrix));
            }

            XMVector GetRightVector() const
            {
                return DirectX::XMVector3Normalize(XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), worldMatrix));
            }

            XMVector GetUpVector() const
            {
                return DirectX::XMVector3Normalize(XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), worldMatrix));
            }

            Matrix4f GetWorldMatrix()
            {
                UpdateWorldMatrixIfNeeded();

                return worldMatrix;
            }

            void Translate(const XMVector& translation) 
            {
                SetLocalPosition(DirectX::XMVectorAdd(localPosition, translation));
            }

            void Rotate(const XMVector& rotation) 
            {
                SetLocalRotation(DirectX::XMVectorAdd(localRotation, rotation));
                WrapAngles();
            }

            void ScaleBy(const XMVector& scale) 
            {
                SetLocalScale(DirectX::XMVectorMultiply(localScale, scale));
            }

            void SetParent(Shared<Transform> parent)
            {
				parentTransform = parent;

				MarkDirty();
			}

			static Shared<Transform> Create()
			{
				return std::make_shared<Transform>();
			}

        private:

            void MarkDirty() 
            {
                if (!isDirty) 
                    isDirty = true;
            }

            void UpdateWorldMatrixIfNeeded() 
            {
                if (isDirty) 
                {
                    RecalculateWorldMatrix();
                    isDirty = false;
                }
            }

            void RecalculateWorldMatrix()
            {
                Matrix4f scaleMatrix = DirectX::XMMatrixScalingFromVector(localScale);
                Matrix4f rotationMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(localRotation);
                Matrix4f translationMatrix = DirectX::XMMatrixTranslationFromVector(localPosition);

                worldMatrix = DirectX::XMMatrixMultiply(scaleMatrix, rotationMatrix);
                worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, translationMatrix);

                if (parentTransform)
                    worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, parentTransform->GetWorldMatrix());
            }

            void WrapAngles() 
            {
                localRotation = DirectX::XMVectorModAngles(localRotation);
            }

            XMVector localPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            XMVector localRotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
            XMVector localScale = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
            Matrix4f worldMatrix = DirectX::XMMatrixIdentity();
            bool isDirty = true;

            Shared<Transform> parentTransform = nullptr;
		};
	}
}