#include "stdafx.h"

#include "InputDevice.h"

namespace Core
{
    namespace Input
    {
        void InputDevice::AddInputObserver(IInputObserver* observer)
        {
            _observers.push_back(observer);
        }

        void InputDevice::RemoveInputObserver(IInputObserver* observer)
        {
            std::remove(_observers.begin(), _observers.end(), observer);
        }

        InputDevice::InputDevice()
        {
            Init();
            CreateKeyboardDevice();
            CreateMouseDevice();
        }

        InputDevice::~InputDevice()
        {
            if (_keyboardDevice)
            {
                _keyboardDevice->Unacquire();
                _keyboardDevice->Release();
                _keyboardDevice = nullptr;
            }

            if (_mouseDevice)
            {
                _mouseDevice->Unacquire();
                _mouseDevice->Release();
                _mouseDevice = nullptr;
            }
        }

        void InputDevice::Init()
        {
            // Create a DirectInput device
            HRESULT result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&_directInput, NULL);

            if (FAILED(result))
                Logger::Log(LogType::Error, "Failed to create DirectInput device");
        }

        void InputDevice::CreateKeyboardDevice()
        {
            HRESULT result = _directInput->CreateDevice(GUID_SysKeyboard, &_keyboardDevice, NULL);

            if (FAILED(result))
                Logger::Log(LogType::Error, "Failed to create keyboard input device device");

            _keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
            _keyboardDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _keyboardDevice->Acquire();
        }

        void InputDevice::CreateMouseDevice()
        {
            HRESULT result = _directInput->CreateDevice(GUID_SysMouse, &_mouseDevice, NULL);

            if (FAILED(result))
                Logger::Log(LogType::Error, "Failed to create mouse input device device");

            _mouseDevice->SetDataFormat(&c_dfDIMouse2);
            _mouseDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _mouseDevice->Acquire();
        }

        void InputDevice::Notify()
        {
            _mouseDevice->GetDeviceState(sizeof(_mouseState), (LPVOID)_mouseState);
            _keyboardDevice->GetDeviceState(sizeof(_keyboardState), (LPVOID)_keyboardState);

            _mouseDevice->Poll();
            for (auto observer : _observers)
            {
                observer->OnKeyboardEvent(_keyboardState);
                observer->OnMouseEvent(*_mouseState);
            }
        }
    } // namespace Input
} // namespace Core
