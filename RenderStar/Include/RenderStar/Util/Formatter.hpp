#pragma once

#include "RenderStar/Util/Typedefs.hpp"

namespace RenderStar
{
	namespace Util
	{
		class Formatter
		{

		public:

			template <class... Argument>
			static String Format(const String& format, Argument&&... arguments)
			{
				return std::vformat(format, std::make_format_args(arguments...));
			}

			static String ColorFormat(const String& message)
            {
                static const UnorderedMap<char, String> ansiMap = 
                {
                    {'0', "\033[30m"},
                    {'1', "\033[34m"},
                    {'2', "\033[32m"},
                    {'3', "\033[36m"},
                    {'4', "\033[31m"},
                    {'5', "\033[35m"},
                    {'6', "\033[33m"},
                    {'7', "\033[37m"},
                    {'8', "\033[90m"},
                    {'9', "\033[94m"},
                    {'a', "\033[92m"},
                    {'b', "\033[96m"},
                    {'c', "\033[91m"},
                    {'d', "\033[95m"},
                    {'e', "\033[93m"},
                    {'f', "\033[97m"},
                    {'k', "\033[5m"}, 
                    {'l', "\033[1m"},
                    {'m', "\033[9m"},
                    {'n', "\033[4m"},
                    {'o', "\033[3m"},
                    {'r', "\033[0m"}
                };

                String result;
                Size length = message.length();

                for (Size character = 0; character < length; ++character) 
                {
                    if (message[character] == '&' && character + 1 < length) 
                    {                    
                        Character code = message[character + 1];
                        auto iterator = ansiMap.find(code);

                        if (iterator != ansiMap.end()) 
                        {
                            result += iterator->second;
                            ++character;
                        }
                        else 
                            result += message[character];
                    }
                    else 
                        result += message[character];
                }

                result += "\033[0m";

                return result;
            }

            static String FormatFromColor(const String& formattedMessage)
            {
                static const Regex ansiRegex("\033\\[[0-9;]*m");

                String result = std::regex_replace(formattedMessage, ansiRegex, "");

                Size length = result.length();
                String cleanMessage;

                for (Size i = 0; i < length; ++i) 
                {
                    if (result[i] == '&' && i + 1 < length) 
                    {
                        Character code = result[i + 1];

                        if ((code >= '0' && code <= '9') || (code >= 'a' && code <= 'f') || (code >= 'k' && code <= 'o') || code == 'r') 
                            ++i;
                        else 
                            cleanMessage += result[i];
                    }
                    else 
                        cleanMessage += result[i];
                }

                return cleanMessage;
            }
		};
	}
}