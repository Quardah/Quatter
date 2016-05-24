#include "quattercam.h"

QuatterCam::QuatterCam():
    Object(MC->GetContext()),
    distance_{23.0f},
    dollyRotation_{Vector2::ZERO},
    targetPosition_{Vector3::ZERO},
    lastTargetPosition_{Vector3::ZERO}

{
    rootNode_ = MC->world.scene->CreateChild("Camera");
    camera_ = rootNode_->CreateComponent<Camera>();
    camera_->SetFarClip(1024.0f);
    rootNode_->SetPosition(Vector3(3.0f, 5.0f, -11.0f));
    rootNode_->LookAt(Vector3::ZERO);

    Zone* zone{rootNode_->CreateComponent<Zone>()};
    zone->SetFogStart(23.0f);
    zone->SetFogEnd(42.0f);

    SetupViewport();
}

void QuatterCam::SetupViewport()
{
    //Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, MC->world.scene, camera_));
    viewport_ = viewport;

    //Add anti-asliasing and bloom
    effectRenderPath_ = viewport_->GetRenderPath()->Clone();
    effectRenderPath_->Append(MC->cache_->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
    effectRenderPath_->SetEnabled("FXAA3", true);
    effectRenderPath_->Append(MC->cache_->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    effectRenderPath_->SetShaderParameter("BloomThreshold", 0.23f);
    effectRenderPath_->SetShaderParameter("BloomMix", Vector2(1.75f, 1.25f));
    effectRenderPath_->SetEnabled("Bloom", true);

    Renderer* renderer = GetSubsystem<Renderer>();
    viewport_->SetRenderPath(effectRenderPath_);
    renderer->SetViewport(0, viewport_);
}

void QuatterCam::Rotate(Vector2 rotation)
{
    rootNode_->LookAt(targetPosition_);
    rootNode_->RotateAround(targetPosition_,
                            Quaternion(rotation.x_, Vector3::UP) * Quaternion(rotation.y_, rootNode_->GetRight()), TS_WORLD);

    //Clamp pitch
    float pitch{rootNode_->GetRotation().EulerAngles().x_};
    if (pitch > 80.0f)
        rootNode_->RotateAround(targetPosition_,
                                Quaternion(-(pitch - 80.0f), rootNode_->GetRight()), TS_WORLD);
    else if (pitch < -0.0f)
        rootNode_->RotateAround(targetPosition_,
                                Quaternion(-pitch, rootNode_->GetRight()), TS_WORLD);
}

//void QuatterCam::Focus(Vector3 targetPosition, float distance, float duration)
//{
/////Focus on targetposition for duration from distance
//}

//Vector3 QuatterCam::GetWorldPosition()
//{
//    return rootNode_->GetWorldPosition();
//}

Quaternion QuatterCam::GetRotation()
{
    return rootNode_->GetRotation();
}