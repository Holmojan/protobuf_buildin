#pragma once

#if !defined(__PB_BUILDIN__BINARY_SERIALIZER_HPP__)
#define __PB_BUILDIN__BINARY_SERIALIZER_HPP__

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)

namespace pb_buildin {

	namespace binary_serializer
	{
		class binary_stream
		{
		protected:
			std::vector<uint8_t> _data;
			mutable uint32_t _pos;
		public:
			binary_stream() :_pos(0) {
				_data.reserve(256);
			}

			const void* data() const {
				return _data.data();
			}

			uint32_t size() const {
				return _data.size();
			}

			uint32_t pos() const {
				return _pos;
			}

			void set_pos(uint32_t pos) const {
				_pos = pos;
			}

			bool read(void* data, uint32_t len) const
			{
				if (_pos + len > _data.size()) {
					return false;
				}
				memcpy(data, _data.data() + _pos, len);
				_pos += len;
				return true;
			}

			template<typename T>
			std::enable_if_t<std::is_pod<T>::value, bool> read(T& v) const {
				return read(&v, sizeof(v));
			}

			template<typename T>
			std::enable_if_t<std::is_unsigned<T>::value, bool> read_varints(T& v)const
			{
				enum {
					bit = sizeof(v) * 8,
					mask = ~((1 << (bit % 7)) - 1)
				};


				v = 0;

				for (int i = 0; i < 64/*bit*/; i += 7)
				{
					uint8_t t = 0;
					if (!read(t)) {
						return false;
					}

					uint8_t w = (t & 0x7f);

					//if ((i + 7) > bit && (w & mask)) {
					//	return false;
					//}

					v |= (uint64_t)w << i;// in c++, i auto mod bit, may get wrong value

					if (!(t & 0x80)) {
						break;
					}
				}

				return true;
			}

			bool read_string(std::string& s)const
			{
				uint32_t l = 0;
				if (!read_varints(l)) {
					return false;
				}
				if (pos() + l > size()) {
					return false;
				}
				s.append((char*)data() + pos(), (char*)data() + pos() + l);
				set_pos(pos() + l);
				return true;
			}

			bool read_varints_raw(std::string& s)const
			{
				uint32_t p = pos();

				uint64_t v = 0;
				if (!read_varints(v)) {
					return false;
				}

				s.assign((char*)data() + p, (char*)data() + pos());
				return true;
			}

			bool write(const void* data, uint32_t len)
			{
				if (_pos + len > _data.size()) {
					if (_pos + len > _data.capacity()) {
						_data.reserve(_data.capacity() * 2);
					}
					_data.resize(_pos + len);
				}
				memcpy(_data.data() + _pos, data, len);
				_pos += len;
				return true;
			}

			template<typename T>
			std::enable_if_t<std::is_pod<T>::value, bool> write(const T& v) {
				return write(&v, sizeof(v));
			}

			template<typename T>
			std::enable_if_t<std::is_unsigned<T>::value, bool> write_varints(const T& v)
			{
				auto u = v;
				do
				{
					uint8_t t = (u & 0x7f);
					u >>= 7;

					if (u) {
						t |= 0x80;
					}
					if (!write(t)) {
						return false;
					}

				} while (u);

				return true;
			}

			bool write_stream(const binary_stream& bs) {
				return write_varints(bs.size())
					&& write(bs.data(), bs.size());
			}
			bool write_string(const std::string& s) {
				return write_varints(s.size())
					&& write(s.data(), s.size());
			}
		};
		//////////////////////////////////////////////////////////////////////////

		static bool serialize(const	bool v, binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write(v);
		}

		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, bool>
		serialize(const T v, binary_stream& bs, const member_register* member)
		{
			return bs.write_varints((uint32_t)v);
		}


		static bool serialize(const int32_t v, binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int32):

				return bs.write_varints((uint64_t)v);

			case PB_TYPE(sint32):

				return bs.write_varints(en_zigzag32(v));

			case PB_TYPE(sfixed32):

				return bs.write(en_zigzag32(v));

			}

			return false;
		}
	
		static bool serialize(const int64_t v, binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int64):

				return bs.write_varints((uint64_t)v);

			case PB_TYPE(sint64):

				return bs.write_varints(en_zigzag64(v));

			case PB_TYPE(sfixed64):

				return bs.write(en_zigzag64(v));

			}

			return false;
		}

		static bool serialize(const uint32_t v, binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint32):

				return bs.write_varints(v);

			case PB_TYPE(fixed32):

				return bs.write(v);

			}

			return false;
		}
		static bool serialize(const uint64_t v, binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint64):

				return bs.write_varints(v);

			case PB_TYPE(fixed64):

				return bs.write(v);

			}

			return false;
		}

		static bool serialize(const float v, binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write(v);
		}
		static bool serialize(const double v, binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write(v);
		}

		static bool serialize(const std::string v, binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write_string(v);
		}

		template<typename T>
		static bool serialize(const pb_repeated<T>& v, binary_stream& bs, const member_register* member)
		{
			if (member->get_flag() & PB_BUILDIN_FLAG_PACKED)
			{
				if (v.empty()) {
					return true;
				}

				if (!bs.write_varints(member->get_tag())) {
					return false;
				}

				binary_stream bs2;
				for (size_t i = 0; i < v.size(); i++) {
					if (!serialize(v[i], bs2, member)) {
						return false;
					}
				}

				return bs.write_stream(bs2);
			}
			else
			{
				for (size_t i = 0; i < v.size(); i++) {

					if (!bs.write_varints(member->get_tag())) {
						return false;
					}
					if (!serialize(v[i], bs, member)) {
						return false;
					}
				}
				return true;
			}
		}
		
		template<typename T>
		static std::enable_if_t<std::is_base_of<pb_message_base, T>::value, bool>
		serialize(const pb_map<T>& v, binary_stream& bs, const member_register* member)
		{
			typedef typename pb_map<T>::key_type key_type;
			typedef typename pb_map<T>::pair_type pair_type;

			static_assert(std::is_integral<key_type>::value
				|| std::is_same<key_type, std::string>::value, "illegal key type");

			for (auto& p : v)
			{
				if (!bs.write_varints(member->get_tag())) {
					return false;
				}

				pair_type item;
				item.set_key(p.first);
				item.set_value(p.second);
			
				if (!serialize(item, bs, member)) {
					return false;
				}
			}
			return true;
		}

		template<typename T>
		static bool serialize(const pb_optional<T>& v, binary_stream& bs, const member_register* member)
		{
			if (!v.has()) {
				return true;
			}

			if (!(member->get_flag() & PB_BUILDIN_FLAG_REPEATED))
			{
				if (!bs.write_varints(member->get_tag())) {
					return false;
				}
			}

			return serialize(v.get(), bs, member);
		}

		static bool serialize(uint32_t tag, const std::string& data, binary_stream& bs)
		{
			if (!bs.write_varints(tag)) {
				return false;
			}

			switch (tag & 0x7)
			{
			case 1:
				if (data.size() != 8) {
					return false;
				}
				if (!bs.write(data.data(), data.size())) {
					return false;
				}
				break;
			case 5:
				if (data.size() != 4) {
					return false;
				}
				if (!bs.write(data.data(), data.size())) {
					return false;
				}
				break;
			case 0:
				if (data.size() < 1) {
					return false;
				}
				if (!bs.write(data.data(), data.size())) {
					return false;
				}
				break;
			case 2:
				if (!bs.write_string(data)) {
					return false;
				}
				break;
			default:
				return false;
			}
			return true;
		}

		static bool serialize(const pb_message_base& v, binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			
			binary_stream bs2;

			auto table = v.GetDescriptor()->get_member_table();
			for (auto& item : table)
			{
				if (!item->serialize(&v, bs2)) {
					return false;
				}
			}

			for (auto& item : v.GetUnknownFields())
			{
				if (!serialize(item.first, item.second, bs2)) {
					return false;
				}
			}

			return bs.write_stream(bs2);
		}
		//////////////////////////////////////////////////////////////////////////

		static bool deserialize(bool& v, const binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read(v);
		}
		static bool deserialize(std::vector<bool>::reference v, const binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);

			bool b = false;
			if (!bs.read(b)) {
				return false;
			}

			v = b;
			return true;
		}


		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, bool>
		deserialize(T& v, const binary_stream& bs, const member_register* member)
		{
			return bs.read_varints((uint32_t&)v);
		}

		static bool deserialize(int32_t& v, const binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int32):

				return bs.read_varints((uint32_t&)v);

			case PB_TYPE(sint32):

				if (!bs.read_varints((uint32_t&)v)) {
					return false;
				}
				v = de_zigzag32(v);
				return true;

			case PB_TYPE(sfixed32):

				if (!bs.read(v)) {
					return false;
				}
				v = de_zigzag32(v);
				return true;

			}

			return false;
		}

		static bool deserialize(int64_t& v, const binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int64):

				return bs.read_varints((uint64_t&)v);

			case PB_TYPE(sint64):

				if (!bs.read_varints((uint64_t&)v)) {
					return false;
				}
				v = de_zigzag64(v);
				return true;

			case PB_TYPE(sfixed64):

				if (!bs.read(v)) {
					return false;
				}
				v = de_zigzag64(v);
				return true;

			}

			return false;
		}

		static bool deserialize(uint32_t& v, const binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint32):

				return bs.read_varints(v);

			case PB_TYPE(fixed32):

				return bs.read(v);

			}

			return false;
		}
		static bool deserialize(uint64_t& v, const binary_stream& bs, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint64):

				return bs.read_varints(v);

			case PB_TYPE(fixed64):

				return bs.read(v);

			}

			return false;
		}

		static bool deserialize(float& v, const binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read(v);
		}
		static bool deserialize(double& v, const binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read(v);
		}

		static bool deserialize(std::string& v, const binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			bs.read_string(v);
			return true;
		}

		template<typename T>
		static bool deserialize(pb_repeated<T>& v, const binary_stream& bs, const member_register* member)
		{
			if (member->get_flag() & PB_BUILDIN_FLAG_PACKED)
			{
				uint32_t l = 0;
				if (!bs.read_varints(l)) {
					return false;
				}
				l += bs.pos();

				while (bs.pos() < l)
				{
					v.push_back(T());
					if (!deserialize(v.back(), bs, member)) {
						return false;
					}
				}

				return bs.pos() == l;
			}
			else
			{
				v.push_back(T());
				if (!deserialize(v.back(), bs, member)) {
					return false;
				}
				return true;
			}
		}
		
		template<typename T>
		static std::enable_if_t<std::is_base_of<pb_message_base, T>::value, bool>
		deserialize(pb_map<T>& v, const binary_stream& bs, const member_register* member)
		{
			typedef typename pb_map<T>::key_type key_type;
			typedef typename pb_map<T>::pair_type pair_type;

			static_assert(std::is_integral<key_type>::value
				|| std::is_same<key_type, std::string>::value, "illegal key type");
			
			pair_type item;
			if (!deserialize(item, bs, member)) {
				return false;
			}
			
			if (!item.has_key() || !item.has_value()) {
				return false;
			}

			v[item.key()] = item.value();
			return true;
		}

		template<typename T>
		static bool deserialize(pb_optional<T>& v, const binary_stream& bs, const member_register* member)
		{
			return deserialize(*v.mutable_get(), bs, member);
		}

		static bool deserialize(pb_message_base& v, const binary_stream& bs, const member_register* member)
		{
			ignore_unused(member);

			uint32_t l = 0;
			if (!bs.read_varints(l)) {
				return false;
			}
			l += bs.pos();

			auto table = v.GetDescriptor()->get_member_table();
			//for (auto& item : table) {
			//	item->clear(&v);
			//}

			while (bs.pos() < l)
			{
				uint32_t tag = 0;
				if (!bs.read_varints(tag)) {
					return false;
				}

				auto itor = std::find_if(table.begin(), table.end(),
					[=](const member_register* item) { return item->get_tag() == tag; });
				if (itor != table.end())
				{
					if (!(*itor)->deserialize(&v, bs)) {
						return false;
					}
				}
				else
				{
					char c[sizeof(uint64_t)] = {};
					std::string s;

					switch (tag & 0x7)
					{
					case 1:
						if (!bs.read(c, 8)) {
							return false;
						}
						s.assign(c, c + 8);
						break;
					case 5:
						if (!bs.read(c, 4)) {
							return false;
						}
						s.assign(c, c + 4);
						break;
					case 0:
						if (!bs.read_varints_raw(s)) {
							return false;
						}
						break;
					case 2:
						if (!bs.read_string(s)) {
							return false;
						}
						break;
					default:
						return false;
					}

					v.AddUnknownFields(tag, s);
				}
			}

			return bs.pos() == l;
		}

	}

	static bool serialize_to_binary(const pb_message_base& pb, std::string& buff)
	{
		binary_serializer::binary_stream bs;
		if (!binary_serializer::serialize(pb, bs, nullptr)) {
			return false;
		}

		bs.set_pos(0);
		bs.read_string(buff);
		return true;
	}
	
	static bool	deserialize_from_binary(const std::string& buff, pb_message_base& pb)
	{
		binary_serializer::binary_stream bs;
		if (!bs.write_string(buff)) {
			return false;
		}

		bs.set_pos(0);
		return binary_serializer::deserialize(pb, bs, nullptr);
	}

	static bool deserialize_from_binary(const void* data, uint32_t len, pb_message_base& pb)
	{
		binary_serializer::binary_stream bs;
		if (!bs.write_varints(len)) {
			return false;
		}	
		if (!bs.write(data, len)) {
			return false;
		}
		
		bs.set_pos(0);
		return binary_serializer::deserialize(pb, bs, nullptr);
	}

}


#endif

#endif
