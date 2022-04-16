
#if !defined(__PB_BUILDIN__IMPLEMENT_HPP__)
#define __PB_BUILDIN__IMPLEMENT_HPP__

namespace pb_buildin {


	template<typename T>
	class serialize_implement :public serialize_interface {
	protected:
		serialize_implement(){}
	public:
		static serialize_implement* get_instance() {
			static serialize_implement instance;
			return &instance;
		}
		virtual void clear(void* data) {
			(*(T*)data).clear();
		}
#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
		virtual bool serialize(const void* data, binary_serializer::helper& helper, const member_register* info) {
			return binary_serializer::serialize(*(const T*)data, helper, info);
		}
		virtual bool deserialize(void* data, const binary_serializer::helper& helper, const member_register* info) {
			return binary_serializer::deserialize(*(T*)data, helper, info);
		}
#endif
#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
		virtual bool serialize(const void* data, json_serializer::helper& helper, const member_register* info) {
			return json_serializer::serialize(*(const T*)data, helper, info);
		}
		virtual bool deserialize(void* data, const json_serializer::helper& helper, const member_register* info) {
			return json_serializer::deserialize(*(T*)data, helper, info);
		}
#endif
	};


	class pb_message : public pb_message_base
	{
	public:
		pb_message(const class_register* p) :
			pb_message_base(p) {}
		pb_message(class_register* p) :
			pb_message_base(p) {}
		~pb_message() {}

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
	public:
		std::string SerializeAsString()const {
			std::string s; serialize_to_binary(*this, s); return s;
		}
		bool ParseFromString(const std::string& s) {
			return deserialize_from_binary(s, *this);
		}
		bool ParseFromArray(const void* data, uint32_t len) {
			return deserialize_from_binary(data, len, *this);
		}
#endif
#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
	public:
		std::string SerializeAsJson()const {
			std::string s; serialize_to_json(*this, s); return s;
		}
		Json::Value SerializeAsJsonValue()const {
			Json::Value root; serialize_to_json(*this, root); return root;
		}
		bool ParseFromJson(const std::string& s) {
			return deserialize_from_json(s, *this);
		}
		bool ParseFromJsonValue(const Json::Value& root) {
			return deserialize_from_json(root, *this);
		}
#endif
	};
}

#endif