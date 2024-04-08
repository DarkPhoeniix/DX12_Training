#pragma once

#include "IInputObserver.h"

#include <dinput.h>

namespace Core
{
    namespace Input
    {
        class InputDevice
        {
        public:
            InputDevice(const InputDevice& copy) = delete;
            InputDevice& operator=(const InputDevice& copy) = delete;

            void AddInputObserver(IInputObserver* observer);
            void RemoveInputObserver(IInputObserver* observer);

        private:
            InputDevice();
            ~InputDevice();

            void Init();

            void CreateKeyboardDevice();
            void CreateMouseDevice();

            void Notify();

            LPDIRECTINPUT8 _directInput;
            LPDIRECTINPUTDEVICE8 _keyboardDevice;
            LPDIRECTINPUTDEVICE8 _mouseDevice;

            LPDIMOUSESTATE2 _mouseState;
            BYTE _keyboardState[256];

            std::vector<IInputObserver*> _observers;
        };
    } // namespace Input
} // namespace Core
