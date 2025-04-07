
#if !defined(__PB_BUILDIN__PB_DEFINE_HPP__)
#define __PB_BUILDIN__PB_DEFINE_HPP__

namespace pb_buildin {

	enum
	{
		PB_BUILDIN_FLAG_REPEATED = 0x00000001,
		PB_BUILDIN_FLAG_PACKED = 0x00000002,
	};

#define PB_TYPE(_type)			_type##_PB_TYPE

	enum proto_type {
		PB_TYPE(bool) = 1,
		PB_TYPE(float),
		PB_TYPE(double),
		PB_TYPE(int32),
		PB_TYPE(int64),
		PB_TYPE(uint32),
		PB_TYPE(uint64),
		PB_TYPE(sint32),
		PB_TYPE(sint64),
		PB_TYPE(fixed32),
		PB_TYPE(fixed64),
		PB_TYPE(sfixed32),
		PB_TYPE(sfixed64),
		PB_TYPE(string),
		PB_TYPE(bytes)
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
	/*
	class pb_message_base;

	template<typename T>
	std::enable_if_t<std::is_base_of<pb_message_base, T>::value, int32_t>
	get_wire_type(T*, proto_type type) {
		return 2;
	}
	*/
	template<typename T>
	std::enable_if_t<std::is_enum<T>::value, uint32_t>
	get_wire_type(T*, proto_type type) {
		return 0;
	}

	template<typename T>
	std::enable_if_t<!std::is_enum<T>::value, uint32_t>
	get_wire_type(T*, proto_type type)
	{
		switch (type)
		{
		case PB_TYPE(bool):
		case PB_TYPE(int32):
		case PB_TYPE(int64):
		case PB_TYPE(uint32):
		case PB_TYPE(uint64):
		case PB_TYPE(sint32):
		case PB_TYPE(sint64):
			return 0;
		case PB_TYPE(fixed64):
		case PB_TYPE(sfixed64):
		case PB_TYPE(double):
			return 1;
		case PB_TYPE(fixed32):
		case PB_TYPE(sfixed32):
		case PB_TYPE(float):
			return 5;
		case PB_TYPE(string):
		case PB_TYPE(bytes):
			return 2;
		}

		return 2;
	}

	inline uint32_t get_tag(uint32_t num, uint32_t wire_type, uint32_t flag)
	{
		if (flag & PB_BUILDIN_FLAG_REPEATED) {
			if (flag & PB_BUILDIN_FLAG_PACKED) {
				return num << 3 | 2;
			}
		}
		return num << 3 | wire_type;
	}
}
#endif
