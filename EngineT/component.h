#pragma once

#include "engine_t.h"

namespace EngineT
{
    class Component
    {
    public:
        GameObject* gameObject;

        Component()
        {
            cout << GetType() << " constructor" << endl;
        }

        ~Component()
        {
            cout << GetType() << " destructor" << endl;
        }

        virtual void Update() = 0;

        virtual string GetType()
        {
            return typeid(Component).name();
        }
    };
}
