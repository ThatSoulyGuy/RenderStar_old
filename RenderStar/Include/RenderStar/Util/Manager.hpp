#pragma once

#include "RenderStar/Util/Typedefs.hpp"

#define IsBaseOf(base, derived) std::is_base_of<base, derived>::value

namespace RenderStar
{
    namespace Util
    {
        template<typename T, typename A>
        class Manager
        {

        public:

            virtual ~Manager()
            {
                CleanUp();
            }

            virtual void Register(Shared<T> value)
            {
                registeredObjects.emplace(value->GetName(), value);
            }

            virtual void Unregister(const String& name)
            {
                registeredObjects.erase(name);
            }

            virtual Shared<T> Get(const String& name)
            {
                auto iterator = registeredObjects.find(name);

                if (iterator != registeredObjects.end())
                    return iterator->second;

                return nullptr;
            }

            virtual void CleanUp()
            {
                for (auto& pair : registeredObjects)
                    pair.second->CleanUp();

                registeredObjects.clear();
            }

            static Shared<A> GetInstance()
            {
                static Shared<A> instance = std::make_shared<A>();

                return instance;
            }

        private:

            UnorderedMap<String, Shared<T>> registeredObjects;
        };
    }
}