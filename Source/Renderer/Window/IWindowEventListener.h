#pragma once

#include "events/UpdateEvent.h"
#include "events/RenderEvent.h"
#include "events/KeyEvent.h"
#include "events/MouseMoveEvent.h"
#include "events/MouseButtonEvent.h"
#include "events/MouseScrollEvent.h"
#include "events/ResizeEvent.h"
#include "WindowEvent.h"

#define WM_PIPELINE_CHANGED (WM_USER + 1)
#define WM_LOAD_SCENE (WM_USER + 2)

class Frame;

namespace core
{
    namespace events
    {
        class IWindowEventListener
        {
        public:
            virtual void OnUpdate([[maybe_unused]] UpdateEvent& e) {}
            virtual void OnRender([[maybe_unused]] RenderEvent& e) {}
            virtual void OnKeyDown([[maybe_unused]] KeyEvent& e) {}
            virtual void OnKeyPressed([[maybe_unused]] KeyEvent& e) {}
            virtual void OnKeyReleased([[maybe_unused]] KeyEvent& e) {}
            virtual void OnMouseMoved([[maybe_unused]] MouseMoveEvent& e) {}
            virtual void OnMouseButtonPressed([[maybe_unused]] MouseButtonEvent& e) {}
            virtual void OnMouseButtonReleased([[maybe_unused]] MouseButtonEvent& e) {}
            virtual void OnMouseScroll([[maybe_unused]] MouseScrollEvent& e) {}
            virtual void OnResize([[maybe_unused]] ResizeEvent& e) {}
            virtual void OnPipelineChanged() {}
            virtual void OnLoadScene([[maybe_unused]] const std::string& filepath) {}
            virtual void OnWindowEvent([[maybe_unused]] const core::WindowEvent& windowEvent) {}
        };
    } // namespace events
} // namespace core
