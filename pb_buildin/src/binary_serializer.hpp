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
			std::unique_ptr<uint8_t[]> _data;
			uint32_t _capacity;
			uint32_t _size;
			mutable uint32_t _pos;

			binary_stream(const binary_stream&) = delete;
			binary_stream& operator=(const binary_stream&) = delete;
		public:
			binary_stream(uint32_t capacity) :
				_capacity(capacity), _size(0), _pos(0),
				_data(std::make_unique<uint8_t[]>(capacity))
			{
			}
			binary_stream(const void* ptr, uint32_t len):
				_capacity(len), _size(len), _pos(0),
				_data(std::unique_ptr<uint8_t[]>((uint8_t*)ptr))
			{
			}

			std::unique_ptr<uint8_t[]> release() {
				_capacity = _size = _pos = 0;
				auto p = std::move(_data);
				return p;
			}

			inline const uint8_t* data() const {
				return _data.get();
			}
			inline uint8_t* data() {
				return _data.get();
			}

			inline uint32_t capacity() const {
				return _capacity;
			}

			inline uint32_t size() const {
				return _size;
			}

			inline uint32_t pos() const {
				return _pos;
			}

			inline bool set_pos(uint32_t pos) const
			{
				if (pos >= size()) {
					return false;
				}
				_pos = pos;
				return true;
			}

			bool read(void* ptr, uint32_t len) const
			{
				if (pos() + len > size()) {
					return false;
				}
				memcpy(ptr, data() + pos(), len);
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
				//enum {
				//	bit = sizeof(v) * 8,
				//	mask = ~((1 << (bit % 7)) - 1)
				//};


				v = 0;

				for (int i = 0; i < 64/*bit*/; i += 7)
				{
					uint8_t t = 0;
					if (!read(t)) {
						return false;
					}

					//uint8_t w = (t & 0x7f);

					//if ((i + 7) > bit && (w & mask)) {
					//	return false;
					//}

					v |= (uint64_t)(t & 0x7f) << i;// in c++, i auto mod bit, may get wrong value

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
				s.assign((char*)data() + pos(), l);
				_pos += l;
				return true;
			}

			bool read_varints_raw(std::string& s)const
			{
				uint32_t p = pos();

				uint64_t v = 0;
				if (!read_varints(v)) {
					return false;
				}

				s.assign((char*)data() + p, pos() - p);
				return true;
			}

			bool write(const void* ptr, uint32_t len)
			{
				if (pos() + len > size()) {
					if (pos() + len > capacity()) {
						return false;
					}
					_size = pos() + len;
				}
				memcpy(data() + pos(), ptr, len);
				_pos += len;
				return true;
			}

			template<typename T>
			std::enable_if_t<std::is_pod<T>::value, bool> write(const T& v) {
				return write(&v, sizeof(v));
			}

			template<typename T>
			std::enable_if_t<std::is_unsigned<T>::value, bool> write_varints(T v)
			{
				while (v >= 0x80) {
					if (!write((uint8_t)(v | 0x80))) {
						return false;
					}
					v >>= 7;
				}
				return write((uint8_t)v);

				/*auto u = v;
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

				return true;*/
			}

			//bool write_stream(const binary_stream& bs) {
			//	return write_varints(bs.size())
			//		&& write(bs.data(), bs.size());
			//}
			bool write_string(const std::string& s) {
				return write_varints(s.size())
					&& write(s.data(), s.size());
			}


			inline static size_t bytesize(const void* data, uint32_t len) {
				return len;
			}

			template<typename T>
			inline static std::enable_if_t<std::is_pod<T>::value, size_t> bytesize(const T& v) {
				return sizeof(v);
			}

			template<typename T>
			inline static std::enable_if_t<std::is_unsigned<T>::value, size_t> bytesize_varints(T v)
			{
				/*
				size_t l = 0;
				auto u = v;
				while (u >= 0x80) {
					++l;
					u >>= 7;
				}
				return ++l;
				*/

				if (v < (1 << 7 * 1)) return 1;
				if (v < (1 << 7 * 2)) return 2;
				if (v < (1 << 7 * 3)) return 3;
				if (v < (1 << 7 * 4)) return 4;
				if (sizeof(T) == 4) return 5;

				if (v < (1ull << 7 * 5)) return 5;
				if (v < (1ull << 7 * 6)) return 6;
				if (v < (1ull << 7 * 7)) return 7;
				if (v < (1ull << 7 * 8)) return 8;
				if (v < (1ull << 7 * 9)) return 9;
				return 10;
			}

			//static size_t bytesize_stream(size_t l) {
			//	return bytesize_varints(l)
			//		+ bytesize(nullptr, l);
			//}
			inline static size_t bytesize_string(const std::string& s) {
				return bytesize_varints(s.size())
					+ bytesize(s.data(), s.size());
			}
		};
		//////////////////////////////////////////////////////////////////////////

		static size_t bytesize(const bool v, const member_register* member)
		{
			ignore_unused(member);
			return binary_stream::bytesize(v);
		}

		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, size_t>
			bytesize(const T v, const member_register* member)
		{
			return binary_stream::bytesize_varints((uint32_t)v);
		}


		static size_t bytesize(const int32_t v, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int32):

				if (v >= 0) {
					return binary_stream::bytesize_varints((uint32_t)v);
				}
				return binary_stream::bytesize_varints((uint64_t)v);

			case PB_TYPE(sint32):

				return binary_stream::bytesize_varints(en_zigzag32(v));

			case PB_TYPE(sfixed32):

				return binary_stream::bytesize(en_zigzag32(v));

			}

			return 0;
		}

		static size_t bytesize(const int64_t v, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int64):

				return binary_stream::bytesize_varints((uint64_t)v);

			case PB_TYPE(sint64):

				return binary_stream::bytesize_varints(en_zigzag64(v));

			case PB_TYPE(sfixed64):

				return binary_stream::bytesize(en_zigzag64(v));

			}

			return 0;
		}

		static size_t bytesize(const uint32_t v, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint32):

				return binary_stream::bytesize_varints(v);

			case PB_TYPE(fixed32):

				return binary_stream::bytesize(v);

			}

			return 0;
		}
		static size_t bytesize(const uint64_t v, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint64):

				return binary_stream::bytesize_varints(v);

			case PB_TYPE(fixed64):

				return binary_stream::bytesize(v);

			}

			return 0;
		}

		static size_t bytesize(const float v, const member_register* member)
		{
			ignore_unused(member);
			return binary_stream::bytesize(v);
		}
		static size_t bytesize(const double v, const member_register* member)
		{
			ignore_unused(member);
			return binary_stream::bytesize(v);
		}

		static size_t bytesize(const std::string& v, const member_register* member)
		{
			ignore_unused(member);
			return binary_stream::bytesize_string(v);
		}

		template<typename T>
		static size_t bytesize(const pb_repeated<T>& v, const member_register* member)
		{
			if (member->get_flag() & PB_BUILDIN_FLAG_PACKED)
			{
				if (v.empty()) {
					return 0;
				}

				size_t l = 0;
				l += binary_stream::bytesize_varints(member->get_tag());

				size_t l2 = 0;
				for (size_t i = 0; i < v.size(); i++) {
					l2 += bytesize(v[i], member);
				}

				//l += binary_stream::bytesize_stream(l2);
				l += binary_stream::bytesize_varints(l2) + l2;
				return l;
			}
			else
			{
				size_t l = 0;
				l += v.size() * binary_stream::bytesize_varints(member->get_tag());

				for (size_t i = 0; i < v.size(); i++) {

					//l += binary_stream::bytesize_varints(member->get_tag());
					l += bytesize(v[i], member);
				}
				return l;
			}
		}

		template<typename T>
		static std::enable_if_t<std::is_base_of<pb_message_base, T>::value, size_t>
			bytesize(const pb_map<T>& v, const member_register* member)
		{
			typedef typename pb_map<T>::key_type key_type;
			typedef typename pb_map<T>::pair_type pair_type;

			static_assert(std::is_integral<key_type>::value
				|| std::is_same<key_type, std::string>::value, "illegal key type");

			static const pair_type item;
			auto& table = item.GetDescriptor()->get_member_table();

			size_t l = 0;
			l += v.size() * binary_stream::bytesize_varints(member->get_tag());
			for (auto& p : v)
			{
				//l += binary_stream::bytesize_varints(member->get_tag());

				size_t l2 = 0;
				l2 += binary_stream::bytesize_varints(table[0]->get_tag());
				l2 += bytesize(p.first, table[0]);
				l2 += binary_stream::bytesize_varints(table[1]->get_tag());
				l2 += bytesize(p.second, table[1]);

				//l += binary_stream::bytesize_stream(l2);
				l += binary_stream::bytesize_varints(l2) + l2;
			}
			return l;
		}

		template<typename T>
		static size_t bytesize(const pb_optional<T>& v, const member_register* member)
		{
			if (!v.has()) {
				return 0;
			}

			size_t l = 0;

			if (!(member->get_flag() & PB_BUILDIN_FLAG_REPEATED))
			{
				l += binary_stream::bytesize_varints(member->get_tag());
			}

			l += bytesize(v.get(), member);
			return l;
		}

		static size_t bytesize(uint32_t tag, const std::string& data)
		{
			size_t l = 0;
			l += binary_stream::bytesize_varints(tag);

			switch (tag & 0x7)
			{
			case 1:
			case 5:
			case 0:
				l += binary_stream::bytesize(data.data(), data.size());
				break;
			case 2:
				l += binary_stream::bytesize_string(data);
				break;
			default:
				break;
			}

			return l;
		}

		static size_t bytesize(const pb_message_base& v, const member_register* member)
		{
			//ignore_unused(member);

			size_t l = 0;

			auto& table = v.GetDescriptor()->get_member_table();
			for (auto& item : table) {
				l += item->bytesize(&v);
			}

			for (auto& item : v.GetUnknownFields()) {
				l += bytesize(item.first, item.second);
			}

			//return binary_stream::bytesize_stream(l);
			if (member) {
				return binary_stream::bytesize_varints(l) + l;
			}
			else {
				return l;
			}
		}
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

				if (v >= 0) {
					return bs.write_varints((uint32_t)v);
				}
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

		static bool serialize(const std::string& v, binary_stream& bs, const member_register* member)
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

				size_t l2 = 0;
				for (size_t i = 0; i < v.size(); i++) {
					l2 += bytesize(v[i], member);
				}

				bs.write_varints(l2);

				//binary_stream bs2;
				for (size_t i = 0; i < v.size(); i++) {
					if (!serialize(v[i], bs, member)) {
						return false;
					}
				}

				//return bs.write_stream(bs2);
				return true;
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

			static const pair_type item;
			auto& table = item.GetDescriptor()->get_member_table();

			for (auto& p : v)
			{
				if (!bs.write_varints(member->get_tag())) {
					return false;
				}

				//item.set_key(p.first);
				//item.set_value(p.second);

				//if (!serialize(item, bs, member)) {
				//	return false;
				//}
				size_t l2 = 0;
				l2 += binary_stream::bytesize_varints(table[0]->get_tag());
				l2 += bytesize(p.first, table[0]);
				l2 += binary_stream::bytesize_varints(table[1]->get_tag());
				l2 += bytesize(p.second, table[1]);
				bs.write_varints(l2);

				//binary_stream bs2;
				if (!bs.write_varints(table[0]->get_tag())) {
					return false;
				}
				if (!serialize(p.first, bs, table[0])) {
					return false;
				}
				if (!bs.write_varints(table[1]->get_tag())) {
					return false;
				}
				if (!serialize(p.second, bs, table[1])) {
					return false;
				}
				//if (!bs.write_stream(bs2)) {
				//	return false;
				//}
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
			//ignore_unused(member);
			
			auto& table = v.GetDescriptor()->get_member_table();
			//binary_stream bs2;
			if (member)
			{
				size_t l = 0;

				for (auto& item : table) {
					l += item->bytesize(&v);
				}

				for (auto& item : v.GetUnknownFields()) {
					l += bytesize(item.first, item.second);
				}

				bs.write_varints(l);
			}

			for (auto& item : table)
			{
				if (!item->serialize(&v, bs)) {
					return false;
				}
			}

			for (auto& item : v.GetUnknownFields())
			{
				if (!serialize(item.first, item.second, bs)) {
					return false;
				}
			}

			//return bs.write_stream(bs2);
			return true;
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
			return bs.read_string(v);
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

			v[std::move(*item.mutable_key())] = std::move(*item.mutable_value());
			return true;
		}

		template<typename T>
		static bool deserialize(pb_optional<T>& v, const binary_stream& bs, const member_register* member)
		{
			return deserialize(*v.mutable_get(), bs, member);
		}

		static bool deserialize(pb_message_base& v, const binary_stream& bs, const member_register* member)
		{
			//ignore_unused(member);

			uint32_t l = 0;
			if (member) {
				if (!bs.read_varints(l)) {
					return false;
				}
				l += bs.pos();
			}
			else {
				l = bs.size();
			}

			auto& table = v.GetDescriptor()->get_member_table();
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

	static size_t bytesize_to_binary(const pb_message_base& pb)
	{
		return binary_serializer::bytesize(pb, nullptr);
	}

	static bool serialize_to_binary(const pb_message_base& pb, std::string& buff)
	{
		buff.resize(bytesize_to_binary(pb));

		auto bs = binary_serializer::binary_stream(buff.data(), buff.size());
		bool ret = binary_serializer::serialize(pb, bs, nullptr);

		bs.release().release();
		return ret;
	}

	static std::unique_ptr<uint8_t[]> serialize_to_binary(const pb_message_base& pb, uint32_t& len)
	{
		len = bytesize_to_binary(pb);

		auto bs = binary_serializer::binary_stream(len);
		if (!binary_serializer::serialize(pb, bs, nullptr)) {
			return nullptr;
		}

		return bs.release();
	}

	static bool	deserialize_from_binary(const std::string& buff, pb_message_base& pb)
	{
		auto bs = binary_serializer::binary_stream(buff.data(), buff.size());

		bool ret = binary_serializer::deserialize(pb, bs, nullptr);

		bs.release().release();
		return ret;
	}

	static bool deserialize_from_binary(const void* data, uint32_t len, pb_message_base& pb)
	{
		auto bs = binary_serializer::binary_stream(data, len);

		bool ret = binary_serializer::deserialize(pb, bs, nullptr);

		bs.release().release();
		return ret;
	}

}


#endif

#endif
