
#if !defined(__PB_BUILDIN__PB_BUILDIN_HPP__)
#define __PB_BUILDIN__PB_BUILDIN_HPP__

#if defined(_MSC_VER)
#	if _MSVC_LANG >= 201703L
#		define PB_USE_CPP17
#	endif
#elif defined(__GNUC__)
#	pragma GCC diagnostic ignored "-Winvalid-offsetof"

//#	define sscanf_s				sscanf
//#	define localtime_s(_v,_t)	localtime_r((_t),(_v))
#	include <string.h>
#	if __cplusplus >= 201703L
#		define PB_USE_CPP17
#	endif
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

#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)

#include <json/json.h>

namespace pb_buildin {
	namespace json_serializer {
		typedef Json::Value read_helper;
		typedef Json::Value write_helper;
	}
}

#endif

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)

namespace pb_buildin {

	enum
	{
		PB_BUILDIN_BYTESIZE_USE_CACHE = 0x00000001,
		PB_BUILDIN_BYTESIZE_EMPTY= -1,
	};

	namespace binary_serializer {
		template<bool CHECK>
		class binary_stream;
		typedef binary_stream<true> read_helper;
		typedef binary_stream<false> write_helper;
	}
}

#endif


#include "src/pb_define.hpp"
#include "src/util.hpp"
#include "src/interface.hpp"
#include "src/registrar.hpp"
#include "src/pb_member.hpp"
#include "src/binary_serializer.hpp"
#include "src/json_serializer.hpp"
#include "src/implement.hpp"
#include "src/macro.hpp"

#endif
