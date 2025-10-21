
#if !defined(__PB_BUILDIN__IMPLEMENT_HPP__)
#define __PB_BUILDIN__IMPLEMENT_HPP__

namespace pb_buildin {

	template<typename T>
	class serialize_implement_base : public virtual serialize_interface
	{
	public:
		virtual void clear(void* data) {
			(*static_cast<T*>(data)).clear();
		}

		virtual void swap(void* data, void* data2) {
			(*static_cast<T*>(data)).swap(*static_cast<T*>(data2));
		}
	};

	template<typename T, uint32_t F>
	class serialize_implement_binary;

	template<typename T>
	class serialize_implement_binary<T, 0> : public virtual serialize_interface
	{
	public:
		virtual bool serialize(const void* data, const binary_serializer::write_helper& helper, const member_register* info) {
			return false;
		}
		virtual bool deserialize(void* data, const binary_serializer::read_helper& helper, const member_register* info) {
			return false;
		}
		virtual size_t bytesize(const void* data, uint32_t flags, const member_register* info) {
			return 0;
		}
	};

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
	template<typename T>
	class serialize_implement_binary<T, PB_BUILDIN_FLAG_BINARY> : public virtual serialize_interface
	{
	public:
		virtual bool serialize(const void* data, const binary_serializer::write_helper& helper, const member_register* info) {
			return binary_serializer::serialize(*static_cast<const T*>(data), helper.ref, info);
		}
		virtual bool deserialize(void* data, const binary_serializer::read_helper& helper, const member_register* info) {
			return binary_serializer::deserialize(*static_cast<T*>(data), helper.ref, info);
		}
		virtual size_t bytesize(const void* data, uint32_t flags, const member_register* info) {
			return binary_serializer::bytesize(*static_cast<const T*>(data), flags, info);
		}
	};
#endif

	template<typename T, uint32_t F>
	class serialize_implement_json;

	template<typename T>
	class serialize_implement_json<T, 0> : public virtual serialize_interface
	{
	public:
		virtual bool serialize(const void* data, const json_serializer::write_helper& helper, const member_register* info) {
			return false;
		}
		virtual bool deserialize(void* data, const json_serializer::read_helper& helper, const member_register* info) {
			return false;
		}
	};

#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
	template<typename T>
	class serialize_implement_json<T, PB_BUILDIN_FLAG_JSON> : public virtual serialize_interface
	{
	public:
		virtual bool serialize(const void* data, const json_serializer::write_helper& helper, const member_register* info) {
			return json_serializer::serialize(*static_cast<const T*>(data), helper.ref, info);
		}
		virtual bool deserialize(void* data, const json_serializer::read_helper& helper, const member_register* info) {
			return json_serializer::deserialize(*static_cast<T*>(data), helper.ref, info);
		}
	};
#endif

	template<typename T, uint32_t F>
	class serialize_implement :
		public serialize_implement_base  <T>,
		public serialize_implement_binary<T, (F & PB_BUILDIN_FLAG_BINARY)>,
		public serialize_implement_json  <T, (F & PB_BUILDIN_FLAG_JSON)>
	{
	protected:
		serialize_implement() {}
	public:
		static serialize_implement* get_instance() {
			PB_STATIC(serialize_implement instance);
			return &instance;
		}
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
		std::unique_ptr<uint8_t[]> SerializeAsString(size_t& len)const {
			return serialize_to_binary(*this, len);
		}
		bool ParseFromString(const std::string& s) {
			Clear(); return deserialize_from_binary(s, *this);
		}
		bool ParseFromArray(const void* data, uint32_t len) {
			Clear(); return deserialize_from_binary(data, len, *this);
		}
		void CopyFrom(const pb_message& from) {
			ParseFromString(from.SerializeAsString());
		}
		void MergeFrom(const pb_message& from) {
			deserialize_from_binary(from.SerializeAsString(), *this);
		}
		size_t ByteSize() const{
			return bytesize_to_binary(*this);
		}
#endif
#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)
	public:
		std::string SerializeAsJson(bool multiline = false)const {
			std::string s; serialize_to_json(*this, s, multiline); return s;
		}
		Json::Value SerializeAsJsonValue()const {
			Json::Value root; serialize_to_json(*this, root); return root;
		}
		bool ParseFromJson(const std::string& s) {
			Clear(); return deserialize_from_json(s, *this);
		}
		bool ParseFromJsonValue(const std::string& s) = delete;
		bool ParseFromJsonValue(const Json::Value& root) {
			Clear(); return deserialize_from_json(root, *this);
		}
#endif
	};

}

#endif