#include "stdafx.h"

#include "InputDevice.h"

namespace Core
{
    namespace Events
    {
        void InputDevice::PollEvents()
        {
            // Poll keyboard state
            {
                _keyboardDevice->Poll();
                for (int i = 0; i < 256; ++i)
                {
                    _prevKeyboardState[i] = _currentKeyboardState[i];
                }
                _keyboardDevice->GetDeviceState(sizeof(_currentKeyboardState), _currentKeyboardState);

                for (int i = 0; i < 256; ++i)
                {
                    if (_currentKeyboardState[i] || (_prevKeyboardState[i] != _currentKeyboardState[i]))
                    {
                        _NotifyKeyPressed((DIKeyCode)(i));
                    }
                    else if (!_currentKeyboardState[i] && (_prevKeyboardState[i] != _currentKeyboardState[i]))
                    {
                        _NotifyKeyReleased((DIKeyCode)_currentKeyboardState[i]);
                    }
                }
            }

            // Poll mouse state
            {
                _mouseDevice->Poll();
                _prevMouseState = _currentMouseState;
                _mouseDevice->GetDeviceState(sizeof(_currentMouseState), _currentMouseState);
            }
        }

        void InputDevice::AddInputObserver(IWindowEventListener* observer)
        {
            _inputListeners.push_back(observer);
        }

        void InputDevice::RemoveInputObserver(IWindowEventListener* observer)
        {
            std::remove(_inputListeners.begin(), _inputListeners.end(), observer);
        }

        InputDevice& InputDevice::Instance()
        {
            static InputDevice device;
            return device;
        }

        InputDevice::InputDevice()
        {
            // Create a DirectInput device
            HRESULT result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&_directInput, NULL);
            if (FAILED(result))
            {
                Logger::Log(LogType::Error, "Failed to create DirectInput device");
            }

            _CreateKeyboardDevice();
            _CreateMouseDevice();
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

        void InputDevice::_CreateKeyboardDevice()
        {
            HRESULT result = _directInput->CreateDevice(GUID_SysKeyboard, &_keyboardDevice, NULL);

            if (FAILED(result))
            {
                Logger::Log(LogType::Error, "Failed to create keyboard input device device");
            }

            _keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
            _keyboardDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _keyboardDevice->Acquire();
        }

        void InputDevice::_CreateMouseDevice()
        {
            HRESULT result = _directInput->CreateDevice(GUID_SysMouse, &_mouseDevice, NULL);

            if (FAILED(result))
            {
                Logger::Log(LogType::Error, "Failed to create mouse input device device");
            }

            _mouseDevice->SetDataFormat(&c_dfDIMouse2);
            _mouseDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _mouseDevice->Acquire();
        }

        void InputDevice::_NotifyKeyPressed(DIKeyCode keyCode)
        {
            KeyEvent keyEvent(keyCode);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnKeyPressed(keyEvent);
            }
        }

        void InputDevice::_NotifyKeyReleased(DIKeyCode keyCode)
        {
            KeyEvent keyEvent(keyCode);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnKeyReleased(keyEvent);
            }
        }
    } // namespace Input
} // namespace Core
