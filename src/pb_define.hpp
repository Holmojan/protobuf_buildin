
#if !defined(__PB_BUILDIN__PB_DEFINE_HPP__)
#define __PB_BUILDIN__PB_DEFINE_HPP__

namespace pb_buildin {

	enum
	{
		PB_BUILDIN_FLAG_REPEATED = 0x00000001,
		PB_BUILDIN_FLAG_PACKED = 0x00000002,
	};

	enum proto_type {
		bool_PB_TYPE = 1,
		float_PB_TYPE,
		double_PB_TYPE,
		int32_PB_TYPE,
		int64_PB_TYPE,
		uint32_PB_TYPE,
		uint64_PB_TYPE,
		sint32_PB_TYPE,
		sint64_PB_TYPE,
		fixed32_PB_TYPE,
		fixed64_PB_TYPE,
		sfixed32_PB_TYPE,
		sfixed64_PB_TYPE,
		string_PB_TYPE,
		bytes_PB_TYPE
	};

	typedef int32_t int32;
	typedef int64_t int64;
	typedef uint32_t uint32;
	typedef uint64_t uint64;
	typedef int32_t sint32;
	typedef int64_t sint64;
	typedef uint32_t fixed32;
	typedef uint64_t fixed64;
	typedef int32_t sfixed32;
	typedef int64_t sfixed64;
	typedef std::string string;
	typedef std::string bytes;


	inline int32_t get_wire_type(proto_type type)
	{
		switch (type)
		{
		case bool_PB_TYPE:
		case int32_PB_TYPE:
		case int64_PB_TYPE:
		case uint32_PB_TYPE:
		case uint64_PB_TYPE:
		case sint32_PB_TYPE:
		case sint64_PB_TYPE:
			return 0;
		case fixed64_PB_TYPE:
		case sfixed64_PB_TYPE:
		case double_PB_TYPE:
			return 1;
		case fixed32_PB_TYPE:
		case sfixed32_PB_TYPE:
		case float_PB_TYPE:
			return 5;
		case string_PB_TYPE:
		case bytes_PB_TYPE:
			return 2;
		}

		return 2;
	}
}
#endif
