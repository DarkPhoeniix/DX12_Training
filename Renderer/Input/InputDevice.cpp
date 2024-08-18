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
                for (int i = 0; i < 256; ++i)
                {
                    _prevKeyboardState[i] = _currentKeyboardState[i];
                }
                _keyboardDevice->GetDeviceState(sizeof(_currentKeyboardState), _currentKeyboardState);

                bool keyPressed = false;
                bool keyReleased = false;
                for (int i = 0; i < 256; ++i)
                {
                    keyPressed = _currentKeyboardState[i] || (_prevKeyboardState[i] != _currentKeyboardState[i]);
                    keyReleased = !_currentKeyboardState[i] && (_prevKeyboardState[i] != _currentKeyboardState[i]);

                    if (keyPressed)
                    {
                        _NotifyKeyPressed((DIKeyCode)i);
                    }
                    else if (keyReleased)
                    {
                        _NotifyKeyReleased((DIKeyCode)i);
                    }
                }
            }

            // Poll mouse state
            {
                _prevMouseState = _currentMouseState;
                HRESULT ok = _mouseDevice->GetDeviceState(sizeof(_currentMouseState), &_currentMouseState);

                bool LMBPressed = _currentMouseState.rgbButtons[0] && (_prevMouseState.rgbButtons[0] != _currentMouseState.rgbButtons[0]);
                bool LMBReleased = !_currentMouseState.rgbButtons[0] && (_prevMouseState.rgbButtons[0] != _currentMouseState.rgbButtons[0]);
                bool RMBPressed = _currentMouseState.rgbButtons[1] && (_prevMouseState.rgbButtons[1] != _currentMouseState.rgbButtons[1]);
                bool RMBReleased = !_currentMouseState.rgbButtons[1] && (_prevMouseState.rgbButtons[1] != _currentMouseState.rgbButtons[1]);
                bool MMBPressed = _currentMouseState.rgbButtons[2] && (_prevMouseState.rgbButtons[2] != _currentMouseState.rgbButtons[2]);
                bool MMBReleased = !_currentMouseState.rgbButtons[2] && (_prevMouseState.rgbButtons[2] != _currentMouseState.rgbButtons[2]);
                int x = _currentMouseState.lX;
                int y = _currentMouseState.lY;
                int z = _currentMouseState.lZ;

                if (LMBPressed || RMBPressed || MMBPressed)
                {
                    _NotifyMouseButtonPressed();
                }
                else if (LMBReleased || RMBReleased || MMBReleased)
                {
                    _NotifyMouseButtonReleased();
                }
                else if ((x != 0) || (y != 0))
                {
                    _NotifyMouseMoved(x, y);
                }
                else if (z != 0)
                {
                    _NotifyMouseScrolled(z);
                }
            }
        }

        void InputDevice::AddInputObserver(IWindowEventListener* observer)
        {
            _inputListeners.push_back(observer);
        }

        void InputDevice::RemoveInputObserver(IWindowEventListener* observer)
        {
            std::erase(_inputListeners, observer);
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
            ASSERT(SUCCEEDED(result), "Failed to create DirectInput device");

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

            ASSERT(SUCCEEDED(result), "Failed to create keyboard input device device");

            _keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
            _keyboardDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _keyboardDevice->Acquire();
        }

        void InputDevice::_CreateMouseDevice()
        {
            HRESULT result = _directInput->CreateDevice(GUID_SysMouse, &_mouseDevice, NULL);

            ASSERT(SUCCEEDED(result), "Failed to create mouse input device device");

            _mouseDevice->SetDataFormat(&c_dfDIMouse);
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

        void InputDevice::_NotifyMouseButtonPressed()
        {
            MouseButtonEvent mouseButtonEvent(_currentMouseState.rgbButtons[0], _currentMouseState.rgbButtons[2], _currentMouseState.rgbButtons[1], 0, 0);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseButtonPressed(mouseButtonEvent);
            }
        }

        void InputDevice::_NotifyMouseButtonReleased()
        {
            MouseButtonEvent mouseButtonEvent(_currentMouseState.rgbButtons[0], _currentMouseState.rgbButtons[2], _currentMouseState.rgbButtons[1], 0, 0);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseButtonReleased(mouseButtonEvent);
            }
        }

        void InputDevice::_NotifyMouseMoved(int relativeX, int relativeY)
        {
            MouseMoveEvent mouseMoveEvent;
            mouseMoveEvent.relativeX = relativeX;
            mouseMoveEvent.relativeY = relativeY;

            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseMoved(mouseMoveEvent);
            }
        }

        void InputDevice::_NotifyMouseScrolled(int relativeZ)
        {
            MouseScrollEvent mouseScrollEvent(relativeZ);

            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseScroll(mouseScrollEvent);
            }
        }
    } // namespace Events
} // namespace Core
