#pragma once

#include "RenderStar/Util/DateTime.hpp"
#include "RenderStar/Util/Formatter.hpp"

#define Logger_WriteConsole(message, level) Logger::WriteConsole(message, __FUNCTION__, level)
#define Logger_ThrowError(unexpected, message, fatal) Logger::ThrowError(unexpected, message, __FUNCTION__, fatal)

using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Core
	{
		enum class LogLevel
		{
			INFORMATION,
			DEBUGGING,
			WARNING,
			ERROR,
			FATAL_ERROR
		};

		class Logger
		{

		public:
			
			static void WriteConsole(const String& message, const String& function, const LogLevel& level)
			{
				static String name = GetFunctionName(function);
				name[0] = toupper(name[0]);

				switch (level)
				{

				case LogLevel::INFORMATION:
					std::cout << Formatter::Format(Formatter::ColorFormat("&2&l[{}] [Thread/INFORMATION] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
					break;

				case LogLevel::DEBUGGING:
					std::cout << Formatter::Format(Formatter::ColorFormat("&1&l[{}] [Thread/DEBUGGING] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
					break;

				case LogLevel::WARNING:
					std::cout << Formatter::Format(Formatter::ColorFormat("&6[{}] [Thread/WARNING] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
					break;

				case LogLevel::ERROR:
					std::cout << Formatter::Format(Formatter::ColorFormat("&c[{}] [Thread/ERROR] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
					break;

				case LogLevel::FATAL_ERROR:
					std::cout << Formatter::Format(Formatter::ColorFormat("&4[{}] [Thread/FATAL ERROR] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
					break;

				default:
					break;
				}
			}

			static void ThrowError(const String& unexpected, const String& message, const String& function, bool fatal)
			{
				static String name = GetFunctionName(function);
				name[0] = toupper(name[0]);

				if (!fatal)
					std::cout << Formatter::Format(Formatter::ColorFormat("&c[{}] [Thread/ERROR] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
				else
				{
					std::cout << Formatter::Format(Formatter::ColorFormat("&4[{}] [Thread/FATAL ERROR] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
					MessageBeep(MB_ICONERROR);
					MessageBox(nullptr, message.c_str(), "Fatal Error", MB_ICONERROR);
					throw std::runtime_error(message);
				}
			}

		private:

			static String GetFunctionName(const String& input)
			{
				Size position = input.rfind("::");

				if (position != String::npos) 
					position += 2;
				else 
					position = 0;
				
				return input.substr(position);
			}

		};
	}
}