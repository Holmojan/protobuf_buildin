
#if !defined(__PB_BUILDIN__INTERFACE_HPP__)
#define __PB_BUILDIN__INTERFACE_HPP__

namespace pb_buildin {

	class member_register;

	class serialize_interface
	{
	public:
		virtual void clear(void* data) = 0;
#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
		virtual bool serialize(const void* data, binary_serializer::helper& helper, const member_register* info) = 0;
		virtual bool deserialize(void* data, const binary_serializer::helper& helper, const member_register* info) = 0;
#endif
#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
		virtual bool serialize(const void* data, json_serializer::helper& helper, const member_register* info) = 0;
		virtual bool deserialize(void* data, const json_serializer::helper& helper, const member_register* info) = 0;
#endif
	};

}

#endif