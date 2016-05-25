#include "inputmaster.h"
#include "quattercam.h"
#include "piece.h"

InputMaster::InputMaster() : Master(),
    input_{GetSubsystem<Input>()},
    idle_{false}
{
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleMouseButtonDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(InputMaster, HandleMouseButtonUp));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(InputMaster, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(InputMaster, HandleKeyUp));
    SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleJoystickButtonDown));
    SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(InputMaster, HandleJoystickButtonUp));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(InputMaster, HandleUpdate));
}

void InputMaster::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    int key{eventData[KeyDown::P_KEY].GetInt()};
    pressedKeys_.Insert(key);

    float volumeStep{0.1f};

    switch (key){
        //Exit when ESC is pressed
    case KEY_ESC:{
        MC->Exit();
    } break;
        //Take screenshot when 9 is pressed
    case KEY_9:{
        Graphics* graphics = GetSubsystem<Graphics>();
        Image screenshot(context_);
        graphics->TakeScreenShot(screenshot);
        //Here we save in the Screenshots folder with date and time appended
        String fileName = GetSubsystem<FileSystem>()->GetProgramDir() + "Screenshots/Screenshot_" +
                Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_')+".png";
        Log::Write(1, fileName);
        screenshot.SavePNG(fileName);
    } break;
    case KEY_M: {
        MC->ToggleMusic();
    } break;
    case KEY_KP_PLUS: {
        MC->MusicGainUp(volumeStep);
    } break;
    case KEY_KP_MINUS: {
        MC->MusicGainDown(volumeStep);
    } break;
    default: break;
    }
}
void InputMaster::HandleKeyUp(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyUp;
    int key = eventData[P_KEY].GetInt();
    if (pressedKeys_.Contains(key)) pressedKeys_.Erase(key);
}

void InputMaster::HandleMouseButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonDown;
    int button = eventData[P_BUTTON].GetInt();
    pressedMouseButtons_.Insert(button);
}


void InputMaster::HandleMouseButtonUp(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonUp;
    int button = eventData[P_BUTTON].GetInt();
    if (pressedMouseButtons_.Contains(button)) pressedMouseButtons_.Erase(button);
}

void InputMaster::HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace JoystickButtonDown;
    int joystickId = eventData[P_JOYSTICKID].GetInt();
    int button = eventData[P_BUTTON].GetInt();
    pressedJoystickButtons_.Insert(button);
}

void InputMaster::HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData)
{
    using namespace JoystickButtonUp;
    int joystickId = eventData[P_JOYSTICKID].GetInt();
    int button = eventData[P_BUTTON].GetInt();
    if (pressedJoystickButtons_.Contains(button)) pressedJoystickButtons_.Erase(button);
}

void InputMaster::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    float t{eventData[Update::P_TIMESTEP].GetFloat()};
    idleTime_ += t;

    Vector2 camRot{};
    float camZoom{};

    float keyRotMultiplier{0.5f};
    float keyZoomSpeed{0.1f};

    if (pressedKeys_.Size()) idleTime_ = 0.0f;
    for (int key : pressedKeys_){
        switch (key){
        case KEY_A:{ camRot += keyRotMultiplier * Vector2::RIGHT;
        } break;
        case KEY_D:{ camRot += keyRotMultiplier * Vector2::LEFT;
        } break;
        case KEY_W:{ camRot += keyRotMultiplier * Vector2::UP;
        } break;
        case KEY_S:{ camRot += keyRotMultiplier * Vector2::DOWN;
        } break;
        case KEY_Q:{ camZoom += keyZoomSpeed;
        } break;
        case KEY_E:{ camZoom += -keyZoomSpeed;
        } break;
        default: break;
        }
    }

    if (input_->GetMouseButtonDown(MOUSEB_RIGHT)){
        idleTime_ = 0.0f;
        IntVector2 mouseMove = input_->GetMouseMove();
        camRot += Vector2(mouseMove.x_, mouseMove.y_) * 0.1f;
    }
    //Should check whose turn it is
    JoystickState* joy0 = input_->GetJoystickByIndex(0);
    if (joy0){
        Vector2 rotation{-Vector2(joy0->GetAxisPosition(2), joy0->GetAxisPosition(3))};
        if (rotation.Length()){
            idleTime_ = 0.0f;
            camRot += rotation * t * 42.0f;
        }
        camZoom += t * (joy0->GetAxisPosition(12) - joy0->GetAxisPosition(13));
    }

    float idleThreshold{5.0f};
    float idleStartup{Min(0.5f * (idleTime_ - idleThreshold), 1.0f)};
    if (idleTime_ > idleThreshold){
        if (!idle_) {
            idle_ = true;
            for (Piece* p: MC->world.pieces_){
                p->Deselect();
            }
        }
        camRot += Vector2(t * idleStartup * -0.5f,
                          t * idleStartup * MC->Sine(0.23f, -0.042f, 0.042f));
    } else {
        if (idle_) idle_ = false;
    }

    smoothCamRotate_ = 0.1f * (camRot  + smoothCamRotate_ * 9.0f);
    smoothCamZoom_   = 0.1f * (camZoom + smoothCamZoom_   * 9.0f);

    CAMERA->Rotate(smoothCamRotate_);
    CAMERA->Zoom(smoothCamZoom_);
}
