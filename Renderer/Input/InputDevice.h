#pragma once

#include "Window/IWindowEventListener.h"

#include <dinput.h>

namespace core
{
    namespace events
    {
        class inputDevice
        {
        public:
            inputDevice(const inputDevice& copy) = delete;
            inputDevice& operator=(const inputDevice& copy) = delete;

            void PollEvents();

            void AddinputObserver(IWindowEventListener* observer);
            void RemoveinputObserver(IWindowEventListener* observer);

            static inputDevice& Instance();

        private:
            inputDevice();
            ~inputDevice();

            void _CreateKeyboardDevice();
            void _CreateMouseDevice();

            void _NotifyKeyPressed(DIKeyCode keyCode);
            void _NotifyKeyReleased(DIKeyCode keyCode);

            void _NotifyMouseButtonPressed();
            void _NotifyMouseButtonReleased();
            void _NotifyMouseMoved(int relativeX, int relativeY);
            void _NotifyMouseScrolled(int relativeZ);

            LPDIRECTINPUT8 _directinput;
            LPDIRECTINPUTDEVICE8 _keyboardDevice;
            LPDIRECTINPUTDEVICE8 _mouseDevice;

            DIMOUSESTATE _prevMouseState;
            DIMOUSESTATE _currentMouseState;

            BYTE _prevKeyboardState[256];
            BYTE _currentKeyboardState[256];

            std::vector<IWindowEventListener*> _inputListeners;
        };
    } // namespace events
} // namespace core
