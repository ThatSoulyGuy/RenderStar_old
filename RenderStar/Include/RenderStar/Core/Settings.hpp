#pragma once

#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Core
	{
		class Settings
		{

		public:

			template <typename T>
			void Set(const String& key, const T& value)
			{
				if (!settings.contains(key))
					settings.emplace(key, value);
				else
					settings.at(key) = value;
			}

			template<typename T>
			T Get(const String& key)
			{
				if (settings.find(key) != settings.end())
				{
					try
					{
						return std::any_cast<T>(settings.at(key));
					}
					catch (const std::bad_any_cast&)
					{
						throw RuntimeError("Type mismatch or key does not exist.");
					}
				}

				return T();
			}

			static void SetInstance(Settings& instance)
			{
				Settings::instance = instance;
			}

			static Settings& GetInstance()
			{
				return instance;
			}

		private:

			UnorderedMap<String, Any> settings;

			static Settings instance;
		};

		Settings Settings::instance;
	}
}