
#if !defined(__PB_BUILDIN__JSON_SERIALIZER_HPP__)
#define __PB_BUILDIN__JSON_SERIALIZER_HPP__

#if defined(PB_BUILDIN__USE_JSON_SERIALIZER)

#include <json/json.h>

#if !defined(JSON_HAS_INT64)
#	pragma message(__FILE__": json version unsupport int64, treate as int32!")
#endif

namespace pb_buildin {

	namespace json_serializer
	{
		//////////////////////////////////////////////////////////////////////////
		static bool serialize(const bool v, Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			root = v; 
			return true; 
		}


		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, bool>
		serialize(const T v, Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			root = (int32_t)v;
			return true;
		}

		static bool serialize(const int32_t v, Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			root = v; 
			return true;
		}
		static bool serialize(const int64_t v, Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			root = v; 
			return true;
		}

		static bool serialize(const uint32_t v, Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			root = v; 
			return true;
		}
		static bool serialize(const uint64_t v, Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			root = v;
			return true; 
		}

		static bool serialize(const float v, Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			root = v; 
			return true; 
		}
		static bool serialize(const double v, Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			root = v; 
			return true; 
		}

		static bool serialize(const std::string& v, Json::Value& root, const member_register* member)
		{
			switch (member->get_type())
			{
			case bytes_PB_TYPE:

			{
				std::string u;
				if (!en_base64(v, u)) {
					return false;
				}
				root = u;
				return true;
			}
			case string_PB_TYPE:

				root = v;
				return true;
			}
			return false;
		}
		
		template<typename T>
		static bool serialize(const pb_repeated<T>& v, Json::Value& root, const member_register* member)
		{
			Json::Value arr = Json::Value(Json::arrayValue);
			for (size_t i = 0; i < v.size(); i++) {
				if (!serialize(v[i], arr[i], member)) {
					return false;
				}
			}
			root[member->get_name()].swap(arr);
			return true;
		}

		template<typename T>
		static std::enable_if_t<std::is_integral<typename pb_map<T>::key_type>::value, bool>
		serialize(const pb_map<T>& v, Json::Value& root, const member_register* member)
		{
			typedef typename pb_map<T>::pair_type pair_type;

			auto table = pair_type::GetDescriptor()->get_member_table();

			Json::Value map = Json::Value(Json::objectValue);
			for (auto& p : v) {
				if (!serialize(p.second, map[std::to_string(p.first)], table[1])) {
					return false;
				}
			}
			root[member->get_name()].swap(map);
			return true;
		}

		template<typename T>
		static std::enable_if_t<std::is_same<typename pb_map<T>::key_type, std::string>::value, bool>
		serialize(const pb_map<T>& v, Json::Value& root, const member_register* member)
		{
			typedef typename pb_map<T>::pair_type pair_type;

			auto table = pair_type::GetDescriptor()->get_member_table();

			Json::Value map = Json::Value(Json::objectValue);
			for (auto& p : v) {
				if (!serialize(p.second, map[p.first], table[1])) {
					return false;
				}
			}
			root[member->get_name()].swap(map);
			return true;
		}

		template<typename T>
		static bool serialize(const pb_optional<T>& v, Json::Value& root, const member_register* member)
		{
			if (!v.has()) {
				return true;
			}

			const std::string name = member->get_name();
			return serialize(v.get(), root[name], member);
		}

		static bool	serialize(const pb_message_base& v, Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			
			root = Json::Value(Json::objectValue);

			auto table = v.GetDescriptor()->get_member_table();
			for (auto& item : table)
			{
				if (!item->serialize(&v, root)) {
					return false;
				}
			}

			Json::Value _unknown_fields = Json::Value(Json::arrayValue);
			for (auto& item : v.GetUnknownFields())
			{
				uint32_t tag = item.first;
				const std::string& data = item.second;

				std::string b64;
				if (!en_base64(data, b64)) {
					return false;
				}

				Json::Value _item = Json::Value(Json::objectValue);
				_item["tag"] = tag;
				_item["data"] = b64;
				_unknown_fields.append(_item);
			}
			if (!_unknown_fields.empty()) {
				root["::_unknown_fields"].swap(_unknown_fields);
			}
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		static bool deserialize(bool& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::booleanValue)) {
				return false;
			}
			v = root.asBool();
			return true; 
		}

		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, bool>
		deserialize(T& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::intValue)) {
				return false;
			}
			v = (T)root.asInt();
			return true;
		}

		static bool deserialize(int32_t& v, const Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::intValue)) {
				return false;
			}
			v = root.asInt();
			return true; 
		}

		static bool deserialize(int64_t& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::intValue)) {
				return false;
			}
#if defined(JSON_HAS_INT64)
			v = root.asInt64();
#else
			v = root.asInt();
#endif
			return true;
		}

		static bool deserialize(uint32_t& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::uintValue)) {
				return false;
			}
			v = root.asUInt();
			return true; 
		}
		static bool deserialize(uint64_t& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::uintValue)) {
				return false;
			}
#if defined(JSON_HAS_INT64)
			v = root.asUInt64();
#else
			v = root.asUInt();
#endif
			return true; 
		}

		static bool deserialize(float& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::realValue)) {
				return false;
			}
			v = root.asFloat();
			return true; 
		}
		static bool deserialize(double& v, const Json::Value& root, const member_register* member) 
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::realValue)) {
				return false;
			}
			v = root.asDouble();
			return true;
		}

		static bool deserialize(std::string& v, const Json::Value& root, const member_register* member)
		{
			if (!root.isConvertibleTo(Json::ValueType::stringValue)) {
				return false;
			}
			switch (member->get_type())
			{
			case bytes_PB_TYPE:
			{
				std::string u = root.asString();
				if (!de_base64(u, v)) {
					return false;
				}
				return true;
			}
			case string_PB_TYPE:

				v = root.asString();
				return true;
			}
			return false;
		}
		
		
		template<typename T>
		static bool deserialize(pb_repeated<T>& v, const Json::Value& root, const member_register* member)
		{
			if (!root.isConvertibleTo(Json::ValueType::arrayValue)) {
				return false;
			}

			v.resize(root.size());
			for (size_t i = 0; i < v.size(); i++){
				if (!deserialize(v[i], root[i], member)) {
					return false;
				}
			}
			return true;
		}

		template<typename T>
		static std::enable_if_t<std::is_integral<typename pb_map<T>::key_type>::value, bool>
		deserialize(pb_map<T>& v, const Json::Value& root, const member_register* member)
		{
			if (!root.isConvertibleTo(Json::ValueType::objectValue)) {
				return false;
			}

			typedef typename pb_map<T>::key_type key_type;
			typedef typename pb_map<T>::pair_type pair_type;

			auto table = pair_type::GetDescriptor()->get_member_table();

			for (auto i = root.begin(); i != root.end(); i++) {
				if (!deserialize(v[(key_type)atoll(i.name().c_str())], *i, table[1])) {
					return false;
				}
			}
			return true;
		}
		template<typename T>
		static std::enable_if_t<std::is_same<typename pb_map<T>::key_type, std::string>::value, bool>
		deserialize(pb_map<T>& v, const Json::Value& root, const member_register* member)
		{
			if (!root.isConvertibleTo(Json::ValueType::objectValue)) {
				return false;
			}

			typedef typename pb_map<T>::pair_type pair_type;

			auto table = pair_type::GetDescriptor()->get_member_table();

			for (auto i = root.begin(); i != root.end(); i++){
				if (!deserialize(v[i.name()], *i, table[1])) {
					return false;
				}
			}
			return true;
		}

		template<typename T>
		static bool deserialize(pb_optional<T>& v, const Json::Value& root, const member_register* member)
		{
			if (root.isNull()) {
				return true;
			}
			return deserialize(*v.mutable_get(), root, member);
		}

		static bool deserialize(pb_message_base& v, const Json::Value& root, const member_register* member)
		{
			ignore_unused(member);
			if (!root.isConvertibleTo(Json::ValueType::objectValue)) {
				return false;
			}

			auto table = v.GetDescriptor()->get_member_table();

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)
			{
				binary_serializer::binary_stream bs2;

				for (auto& _item : root["::_unknown_fields"])
				{
					if (!_item.isMember("tag") || !_item["tag"].isUInt()) {
						return false;
					}
					uint32_t tag = _item["tag"].asUInt();

					if (!_item.isMember("data") || !_item["data"].isString()) {
						return false;
					}
					std::string data;
					if (!de_base64(_item["data"].asString(), data)) {
						return false;
					}

					if (!binary_serializer::serialize(tag, data, bs2)) {
						return false;
					}
				}

				if (bs2.size() > 0)
				{
					binary_serializer::binary_stream bs;
					if (!bs.write_stream(bs2)) {
						return false;
					}

					bs.set_pos(0);
					if (!binary_serializer::deserialize(v, bs, member)) {
						return false;
					}
				}
			}
#else
			for (auto& item : table) {
				item.clear(&v);
			}
#endif

			for (auto& item : table)
			{
				const std::string name = item->get_name();
				if (root.isMember(name)) {

					if (!item->deserialize(&v, root[name])) {
						return false;
					}
				}
			}

			return true;
		}
	}

	static bool	serialize_to_json(const pb_message_base& pb, Json::Value& root) {
		return json_serializer::serialize(pb, root, nullptr);
	}
	static bool	serialize_to_json(const pb_message_base& pb, std::string& json, bool multiline = true)
	{
		Json::Value root;
		if (!serialize_to_json(pb, root)) {
			return false;
		}
		if (multiline) {
			json = root.toStyledString();
		}
		else {
			Json::FastWriter writer;
			writer.omitEndingLineFeed();
			json = writer.write(root);
		}
		return true;
	}

	static bool deserialize_from_json(const Json::Value& root, pb_message_base& pb) {
		return json_serializer::deserialize(pb, root, nullptr);
	}
	static bool deserialize_from_json(const std::string& json, pb_message_base& pb)
	{
		Json::Value root;
		Json::Reader reader;
		if (!reader.parse(json, root)) {
			return false;
		}
		return json_serializer::deserialize(pb, root, nullptr);
	}

}

#endif

#endif
