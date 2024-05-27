#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <random>
#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <any>
#include <exception>
#include <array>
#include <format>
#include <list>
#include <regex>
#include <typeindex>
#include <filesystem>
#include <typeinfo>
#include <type_traits>
#include <wrl.h> 
#include <DirectXMath.h>

#ifdef DLL_MODE
#define DLL_API __declspec(dllimport)
#else
#define DLL_API __declspec(dllexport)
#endif

#undef WriteConsole
#undef ERROR

namespace RenderStar
{
	namespace Util
	{
		typedef signed char schar;
		typedef signed short sshort;
		typedef signed int sint;
		typedef signed long slong;
		typedef signed long long int slongint;

		typedef unsigned char uchar;
		typedef unsigned short ushort;
		typedef unsigned int uint;
		typedef unsigned long ulong;
		typedef unsigned long long ullong;
		typedef unsigned long long int ullint;

		typedef std::string String;
		typedef std::wstring WString;

		typedef DirectX::XMVECTOR XMVector;

		typedef DirectX::XMINT2 Vector2i;
		typedef DirectX::XMINT3 Vector3i;
		typedef DirectX::XMINT4 Vector4i;

		typedef DirectX::XMFLOAT2 Vector2f;
		typedef DirectX::XMFLOAT3 Vector3f;
		typedef DirectX::XMFLOAT4 Vector4f;

		typedef DirectX::XMFLOAT2A Vector2fA;
		typedef DirectX::XMFLOAT3A Vector3fA;
		typedef DirectX::XMFLOAT4A Vector4fA;

		typedef DirectX::XMMATRIX Matrix4f;

		typedef DirectX::XMFLOAT4X4 Matrix4x4f;

		typedef std::any Any;
		typedef std::exception RuntimeError;

		typedef std::type_info TypeInformation;
		typedef std::type_index TypeIndex;

		typedef std::filesystem::path Path;

		typedef std::chrono::steady_clock SteadyClock;
		typedef std::chrono::high_resolution_clock Clock;
		typedef std::chrono::time_point<Clock> TimePoint;
		typedef std::chrono::high_resolution_clock::time_point HighResolutionTimePoint;
		typedef std::chrono::duration<float> Duration;
		typedef std::chrono::milliseconds Milliseconds;

		typedef std::thread Thread;
		typedef std::mutex Mutex;
		typedef std::condition_variable ConditionVariable;
		typedef std::unique_lock<Mutex> UniqueLock;

		typedef std::atomic<int> AtomicInt;
		typedef std::atomic<bool> AtomicBool;
		typedef std::future<void> Future;

		typedef std::time_t Time;
		typedef std::tm TimeInformation;

		typedef std::filesystem::directory_iterator DirectoryIterator;
		typedef std::filesystem::directory_entry DirectoryEntry;
		typedef std::filesystem::file_status FileStatus;
		typedef std::filesystem::file_type FileType;

		typedef std::random_device RandomDevice;
		typedef std::mt19937 RandomEngine;

		typedef std::ostream OutputStream;
		typedef std::istream InputStream;

		typedef std::stringstream StringStream;
		typedef std::ostringstream OutputStringStream;

		typedef size_t Size;
		typedef char Character;

		typedef std::ofstream OutputFileStream;
		typedef std::ifstream InputFileStream;

		typedef std::streamsize StreamSize;

		typedef std::regex Regex;

		template<typename T>
		using Shared = std::shared_ptr<T>;

		template<typename T>
		using Unique = std::unique_ptr<T>;

		template<typename T>
		using Weak = std::weak_ptr<T>;

		template<typename T>
		using Vector = std::vector<T>;

		template<typename T, typename A>
		using Map = std::map<T, A>;

		template<typename T, typename A>
		using UnorderedMap = std::unordered_map<T, A>;

		template<typename T>
		using Function = std::function<T>;

		template<typename T, size_t S>
		using Array = std::array<T, S>;

		template<class... _Args>
		using FormatString = std::basic_format_string<char, std::type_identity_t<_Args>...>;

		template<typename T>
		using EnableShared = std::enable_shared_from_this<T>;

		template<typename T>
		using List = std::list<T>;

		template<typename T, typename A>
		using Pair = std::pair<T, A>;

		template<typename T>
		using LockGuard = std::lock_guard<T>;

		template<typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
	}
}