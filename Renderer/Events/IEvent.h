#pragma once

namespace Core
{
    namespace Events
    {
        class IEvent
        {
        public:
            virtual ~IEvent()
            {   }
        };
    } // namespace Events
} // namespace Core