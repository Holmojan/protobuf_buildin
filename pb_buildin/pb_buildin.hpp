
#if !defined(__PB_BUILDIN__PB_BUILDIN_HPP__)
#define __PB_BUILDIN__PB_BUILDIN_HPP__

#if defined(_MSC_VER)
#	if _MSVC_LANG >= 201703L
#		define PB_USE_CPP17
#	endif
#	if _MSC_VER <= 1800
#		include <assert.h>
#		include <atomic>
#		include <mutex>
#		define PB_STATIC(...)	\
			get_static_constructor_lock().lock();	\
			static __VA_ARGS__;						\
			get_static_constructor_lock().unlock()
#	else
#		define PB_STATIC(...)	static __VA_ARGS__
#	endif
#elif defined(__GNUC__)
#	pragma GCC diagnostic ignored "-Winvalid-offsetof"

//#	define sscanf_s				sscanf
//#	define localtime_s(_v,_t)	localtime_r((_t),(_v))
#	include <string.h>
#	if __cplusplus >= 201703L
#		define PB_USE_CPP17
#	endif
#	define PB_STATIC(...)	static __VA_ARGS__
#else
#	error __FILE__": unsupported ide, compile stoped!" 
#endif

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <typeinfo>
#include <functional>
#include <type_traits>

#if defined(PB_USE_CPP17)
#include <optional>
#endif

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
#	define PB_BUILDIN_FLAG_BINARY	0x00000001
#else
#	define PB_BUILDIN_FLAG_BINARY	0x00000000
#endif

#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
#	include <json/json.h>
#	define PB_BUILDIN_FLAG_JSON		0x00000002
#else
#	define PB_BUILDIN_FLAG_JSON		0x00000000
#endif


namespace pb_buildin {

	enum {
		PB_BUILDIN_BYTESIZE_SERIALIZE = 0x00000001,
		PB_BUILDIN_BYTESIZE_EMPTY = -1,
	};

	namespace binary_serializer {
		//template<bool CHECK>
		//class binary_stream;
		//typedef binary_stream<true> read_helper;
		//typedef binary_stream<false> write_helper;
		class read_helper;
		class write_helper;
	}

	namespace json_serializer {
		//typedef Json::Value read_helper;
		//typedef Json::Value write_helper;
		class read_helper;
		class write_helper;
	}
}


#pragma warning( push )
#pragma warning(disable:4250)

#include "src/pb_define.hpp"
#include "src/util.hpp"
#include "src/interface.hpp"
#include "src/registrar.hpp"
#include "src/pb_member.hpp"
#include "src/binary_serializer.hpp"
#include "src/json_serializer.hpp"
#include "src/implement.hpp"
#include "src/macro.hpp"

#pragma warning( pop )

#endif
