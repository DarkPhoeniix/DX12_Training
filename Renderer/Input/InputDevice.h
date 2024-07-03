#pragma once

#include "Window/IWindowEventListener.h"

#include <dinput.h>

namespace Core
{
    namespace Events
    {
        class InputDevice
        {
        public:
            InputDevice(const InputDevice& copy) = delete;
            InputDevice& operator=(const InputDevice& copy) = delete;

            void PollEvents();

            void AddInputObserver(IWindowEventListener* observer);
            void RemoveInputObserver(IWindowEventListener* observer);

            static InputDevice& Instance();

        private:
            InputDevice();
            ~InputDevice();

            void _CreateKeyboardDevice();
            void _CreateMouseDevice();

            void _NotifyKeyPressed(DIKeyCode keyCode);
            void _NotifyKeyReleased(DIKeyCode keyCode);

            void _NotifyMouseButtonPressed();
            void _NotifyMouseButtonReleased();
            void _NotifyMouseMoved(int relativeX, int relativeY);
            void _NotifyMouseScrolled(int relativeZ);

            LPDIRECTINPUT8 _directInput;
            LPDIRECTINPUTDEVICE8 _keyboardDevice;
            LPDIRECTINPUTDEVICE8 _mouseDevice;

            DIMOUSESTATE _prevMouseState;
            DIMOUSESTATE _currentMouseState;

            BYTE _prevKeyboardState[256];
            BYTE _currentKeyboardState[256];

            std::vector<IWindowEventListener*> _inputListeners;
        };
    } // namespace Events
} // namespace Core
