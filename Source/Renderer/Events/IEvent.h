#pragma once

namespace core
{
    namespace events
    {
        class IEvent
        {
        public:
            virtual ~IEvent()
            {
            }
        };
    } // namespace events
} // namespace core