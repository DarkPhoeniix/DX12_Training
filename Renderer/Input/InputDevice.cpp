#include "RendererPCH.h"

#include "inputDevice.h"

namespace core
{
    namespace events
    {
        void inputDevice::PollEvents()
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

        void inputDevice::AddinputObserver(IWindowEventListener* observer)
        {
            _inputListeners.push_back(observer);
        }

        void inputDevice::RemoveinputObserver(IWindowEventListener* observer)
        {
            std::erase(_inputListeners, observer);
        }

        inputDevice& inputDevice::Instance()
        {
            static inputDevice device;
            return device;
        }

        inputDevice::inputDevice()
        {
            // Create a Directinput device
            HRESULT result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&_directinput, NULL);
            ASSERT(SUCCEEDED(result), "Failed to create Directinput device");

            _CreateKeyboardDevice();
            _CreateMouseDevice();
        }

        inputDevice::~inputDevice()
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

        void inputDevice::_CreateKeyboardDevice()
        {
            HRESULT result = _directinput->CreateDevice(GUID_SysKeyboard, &_keyboardDevice, NULL);

            ASSERT(SUCCEEDED(result), "Failed to create keyboard input device device");

            _keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
            _keyboardDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _keyboardDevice->Acquire();
        }

        void inputDevice::_CreateMouseDevice()
        {
            HRESULT result = _directinput->CreateDevice(GUID_SysMouse, &_mouseDevice, NULL);

            ASSERT(SUCCEEDED(result), "Failed to create mouse input device device");

            _mouseDevice->SetDataFormat(&c_dfDIMouse);
            _mouseDevice->SetCooperativeLevel(NULL, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
            _mouseDevice->Acquire();
        }

        void inputDevice::_NotifyKeyPressed(DIKeyCode keyCode)
        {
            KeyEvent keyEvent(keyCode);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnKeyPressed(keyEvent);
            }
        }

        void inputDevice::_NotifyKeyReleased(DIKeyCode keyCode)
        {
            KeyEvent keyEvent(keyCode);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnKeyReleased(keyEvent);
            }
        }

        void inputDevice::_NotifyMouseButtonPressed()
        {
            MouseButtonEvent mouseButtonEvent(_currentMouseState.rgbButtons[0], _currentMouseState.rgbButtons[2], _currentMouseState.rgbButtons[1], 0, 0);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseButtonPressed(mouseButtonEvent);
            }
        }

        void inputDevice::_NotifyMouseButtonReleased()
        {
            MouseButtonEvent mouseButtonEvent(_currentMouseState.rgbButtons[0], _currentMouseState.rgbButtons[2], _currentMouseState.rgbButtons[1], 0, 0);
            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseButtonReleased(mouseButtonEvent);
            }
        }

        void inputDevice::_NotifyMouseMoved(int relativeX, int relativeY)
        {
            MouseMoveEvent mouseMoveEvent;
            mouseMoveEvent.relativeX = relativeX;
            mouseMoveEvent.relativeY = relativeY;

            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseMoved(mouseMoveEvent);
            }
        }

        void inputDevice::_NotifyMouseScrolled(int relativeZ)
        {
            MouseScrollEvent mouseScrollEvent(relativeZ);

            for (IWindowEventListener* listener : _inputListeners)
            {
                listener->OnMouseScroll(mouseScrollEvent);
            }
        }
    } // namespace events
} // namespace core
