
#if !defined(__PB_BUILDIN__INTERFACE_HPP__)
#define __PB_BUILDIN__INTERFACE_HPP__

namespace pb_buildin {

	class member_register;

	class serialize_interface
	{
	public:
		virtual void clear(void* data) = 0;
		virtual void swap(void* data, void* data2) = 0;
//#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
		virtual bool serialize(const void* data, const binary_serializer::write_helper& helper, const member_register* info) = 0;
		virtual bool deserialize(void* data, const binary_serializer::read_helper& helper, const member_register* info) = 0;
		virtual size_t bytesize(const void* data, uint32_t flags, const member_register* info) = 0;
//#endif
//#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
		virtual bool serialize(const void* data, const json_serializer::write_helper& helper, const member_register* info) = 0;
		virtual bool deserialize(void* data, const json_serializer::read_helper& helper, const member_register* info) = 0;
//#endif
	};

}

#endif