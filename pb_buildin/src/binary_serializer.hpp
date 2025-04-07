#pragma once

#if !defined(__PB_BUILDIN__BINARY_SERIALIZER_HPP__)
#define __PB_BUILDIN__BINARY_SERIALIZER_HPP__

#if defined(PB_BUILDIN__USE_BINARY_SERIALIZER)

namespace pb_buildin {

	namespace binary_serializer
	{
		template<bool CHECK>
		class binary_stream
		{
		protected:
			std::unique_ptr<uint8_t[]> _data;
			uint8_t* _end;
			mutable uint8_t* _cursor;

			binary_stream(const binary_stream&) = delete;
			binary_stream& operator=(const binary_stream&) = delete;
		public:

			binary_stream(size_t size) {
				_data = std::make_unique<uint8_t[]>(size);
				_end = _data.get() + size;
				_cursor = _data.get();
			}
			binary_stream(void* ptr, size_t len) {
				_data = std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(ptr));
				_end = _data.get() + len;
				_cursor = _data.get();
			}

			binary_stream(binary_stream&& bs) {
				_data = std::unique_ptr<uint8_t[]>(nullptr);
				_end = nullptr;
				_cursor = nullptr;
				(*this) = std::move(bs);
			}
			binary_stream& operator=(binary_stream&& bs) {
				_data.swap(bs._data);
				std::swap(_end, bs._end);
				std::swap(_cursor, bs._cursor);
			}

			std::unique_ptr<uint8_t[]> release() {
				auto p = std::move(_data);
				_end = nullptr;
				_cursor = nullptr;
				return p;
			}

			inline const uint8_t* data() const {
				return _data.get();
			}
			inline uint8_t* data() {
				return _data.get();
			}
		
			inline size_t size() const {
				return _end - _data.get();
			}

			inline size_t pos() const {
				return _cursor - _data.get();
			}

			inline bool set_pos(size_t pos) const
			{
				if (_data.get() + pos > _end) {
					return false;
				}
				_cursor = _data.get() + pos;
				return true;
			}

			bool read(void* ptr, size_t len) const
			{
				if (CHECK && _cursor + len > _end) {
					return false;
				}
				memcpy(ptr, _cursor, len);
				_cursor += len;
				return true;
			}

			template<typename T>
			std::enable_if_t<std::is_pod<T>::value, bool> read(T& v) const {
				return read(&v, sizeof(v));
			}

			template<typename T>
			std::enable_if_t<std::is_unsigned<T>::value, bool> read_varints(T& v)const
			{
				v = 0;

				for (size_t i = 0; i < 64/*bit*/; i += 7)
				{
					uint8_t t = 0;
					if (!read(t)) {
						return false;
					}

					v |= (T)(t & 0x7f) << i;// in c++, i auto mod bit, may get wrong value

					if (!(t & 0x80)) {
						return true;
					}
				}

				return false;
			}

			bool read_string(std::string& s)const
			{
				size_t l = 0;
				if (!read_varints(l)) {
					return false;
				}
				if (CHECK && _cursor + l > _end) {
					return false;
				}
				s.assign(reinterpret_cast<const char*>(_cursor), l);
				_cursor += l;
				return true;
			}

			bool read_varints_raw(std::string& s)const
			{
				auto c = _cursor;

				uint64_t v = 0;
				if (!read_varints(v)) {
					return false;
				}

				s.assign(reinterpret_cast<const char*>(c), _cursor - c);
				return true;
			}

			bool write(const void* ptr, size_t len)
			{
				if (CHECK && _cursor + len > _end) {
					return false;
				}
				memcpy(_cursor, ptr, len);
				_cursor += len;
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


			inline static size_t bytesize(const void* data, size_t len) {
				return len;
			}

			template<typename T>
			inline static std::enable_if_t<std::is_pod<T>::value, size_t> bytesize(const T& v) {
				return sizeof(v);
			}

			enum {
				bytesize_varints_min = 1,
				bytesize_varints_max = 10
			};
#if 0
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
#else
			template<typename T>
			inline static std::enable_if_t<std::is_unsigned<T>::value && (sizeof(T) <= 4), size_t> bytesize_varints(T v)
			{
				uint32_t n = log2_floor_nonzero(1 | (uint32_t)v);
				return (n * 9 + 73) / 64;
			}
			
			template<typename T>
			inline static std::enable_if_t<std::is_unsigned<T>::value && (sizeof(T) > 4), size_t> bytesize_varints(T v)
			{
				uint32_t h = v >> 32;
				uint32_t n = h ? log2_floor_nonzero(h) + 32 : log2_floor_nonzero(1 | (uint32_t)v);
				return (n * 9 + 73) / 64;
			}
#endif
			//static size_t bytesize_stream(size_t l) {
			//	return bytesize_varints(l)
			//		+ bytesize(nullptr, l);
			//}
			inline static size_t bytesize_string(const std::string& s) {
				return bytesize_varints(s.size())
					+ bytesize(s.data(), s.size());
			}
		};

		typedef binary_stream<true> read_stream;
		typedef binary_stream<false> write_stream;
		//////////////////////////////////////////////////////////////////////////

		static size_t bytesize(const bool v, uint32_t flags, const member_register* member)
		{
			ignore_unused(member);
			return write_stream::bytesize(v);
		}

		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, size_t>
			bytesize(const T v, uint32_t flags, const member_register* member)
		{
			return write_stream::bytesize_varints((uint32_t)v);
		}


		static size_t bytesize(const int32_t v, uint32_t flags, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int32):

				if (v >= 0) {
					return write_stream::bytesize_varints((uint32_t)v);
				}
				//return write_stream::bytesize_varints((uint64_t)v);
				return write_stream::bytesize_varints_max;

			case PB_TYPE(sint32):

				return write_stream::bytesize_varints(en_zigzag32(v));

			case PB_TYPE(sfixed32):

				return write_stream::bytesize((decltype(en_zigzag32(v)))0);

			}

			return 0;
		}

		static size_t bytesize(const int64_t v, uint32_t flags, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(int64):

				return write_stream::bytesize_varints((uint64_t)v);

			case PB_TYPE(sint64):

				return write_stream::bytesize_varints(en_zigzag64(v));

			case PB_TYPE(sfixed64):

				return write_stream::bytesize((decltype(en_zigzag64(v)))0);

			}

			return 0;
		}

		static size_t bytesize(const uint32_t v, uint32_t flags, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint32):

				return write_stream::bytesize_varints(v);

			case PB_TYPE(fixed32):

				return write_stream::bytesize(v);

			}

			return 0;
		}
		static size_t bytesize(const uint64_t v, uint32_t flags, const member_register* member)
		{
			switch (member->get_type())
			{
			case PB_TYPE(uint64):

				return write_stream::bytesize_varints(v);

			case PB_TYPE(fixed64):

				return write_stream::bytesize(v);

			}

			return 0;
		}

		static size_t bytesize(const float v, uint32_t flags, const member_register* member)
		{
			ignore_unused(member);
			return write_stream::bytesize(v);
		}
		static size_t bytesize(const double v, uint32_t flags, const member_register* member)
		{
			ignore_unused(member);
			return write_stream::bytesize(v);
		}

		static size_t bytesize(const std::string& v, uint32_t flags, const member_register* member)
		{
			ignore_unused(member);
			return write_stream::bytesize_string(v);
		}

		template<typename T>
		static size_t bytesize(const pb_repeated<T>& v, uint32_t flags, const member_register* member)
		{
			if (member->get_flag() & PB_BUILDIN_FLAG_PACKED)
			{
				if (v.empty()) {
					return 0;
				}

				size_t l = 0;
				l += write_stream::bytesize_varints(member->get_tag());

				size_t l2 = 0;
				for (size_t i = 0; i < v.size(); i++) {
					l2 += bytesize(v[i], flags, member);
				}

				//l += write_stream::bytesize_stream(l2);
				l += write_stream::bytesize_varints(l2) + l2;
				return l;
			}
			else
			{
				size_t l = 0;
				l += v.size() * write_stream::bytesize_varints(member->get_tag());

				for (size_t i = 0; i < v.size(); i++) {

					//l += write_stream::bytesize_varints(member->get_tag());
					l += bytesize(v[i], flags, member);
				}
				return l;
			}
		}

		template<typename T>
		static std::enable_if_t<std::is_base_of<pb_message_base, T>::value, size_t>
			bytesize(const pb_map<T>& v, uint32_t flags, const member_register* member)
		{
			typedef typename pb_map<T>::key_type key_type;
			typedef typename pb_map<T>::pair_type pair_type;

			static_assert(std::is_integral<key_type>::value
				|| std::is_same<key_type, std::string>::value, "illegal key type");

			static const pair_type item;
			auto& table = item.GetDescriptor()->get_member_table();
			static const size_t l01 =
				write_stream::bytesize_varints(table[0]->get_tag()) +
				write_stream::bytesize_varints(table[1]->get_tag());

			size_t l = 0;
			l += v.size() * write_stream::bytesize_varints(member->get_tag());
			for (auto& p : v)
			{
				//l += write_stream::bytesize_varints(member->get_tag());

				size_t l2 = l01;
				//l2 += write_stream::bytesize_varints(table[0]->get_tag());
				l2 += bytesize(p.first, flags, table[0]);
				//l2 += write_stream::bytesize_varints(table[1]->get_tag());
				l2 += bytesize(p.second, flags, table[1]);

				//l += write_stream::bytesize_stream(l2);
				l += write_stream::bytesize_varints(l2) + l2;
			}
			return l;
		}

		template<typename T>
		static size_t bytesize(const pb_optional<T>& v, uint32_t flags, const member_register* member)
		{
			if (!v.has()) {
				return 0;
			}

			size_t l = 0;

			//if (!(member->get_flag() & PB_BUILDIN_FLAG_REPEATED))
			//{
			l += write_stream::bytesize_varints(member->get_tag());
			//}

			l += bytesize(v.get(), flags, member);
			return l;
		}

		static size_t bytesize(uint32_t tag, uint32_t flags, const std::string& data)
		{
			size_t l = 0;
			l += write_stream::bytesize_varints(tag);

			switch (tag & 0x7)
			{
			case 1:
			case 5:
			case 0:
				l += write_stream::bytesize(data.data(), data.size());
				break;
			case 2:
				l += write_stream::bytesize_string(data);
				break;
			default:
				break;
			}

			return l;
		}

		static size_t bytesize(const pb_message_base& v, uint32_t flags, const member_register* member)
		{
			//ignore_unused(member);
			size_t l = PB_BUILDIN_BYTESIZE_EMPTY;

			if (flags & PB_BUILDIN_BYTESIZE_USE_CACHE) {
				l = v.GetSerializedByteSize();
				v.SetSerializedByteSize(PB_BUILDIN_BYTESIZE_EMPTY);
			}

			if (l == PB_BUILDIN_BYTESIZE_EMPTY)
			{
				l = 0;

				auto& table = v.GetDescriptor()->get_member_table();
				for (auto& item : table) {
					l += item->bytesize(&v, flags);
				}

				for (auto& item : v.GetUnknownFields()) {
					l += bytesize(item.first, flags, item.second);
				}

				if (flags & PB_BUILDIN_BYTESIZE_USE_CACHE) {
					v.SetSerializedByteSize(l);
				}
			}

			//return write_stream::bytesize_stream(l);
			if (member) {
				l += write_stream::bytesize_varints(l);
			}

			return l;
		}
		//////////////////////////////////////////////////////////////////////////

		static bool serialize(const	bool v, write_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write(v);
		}

		template<typename T>
		static std::enable_if_t<std::is_enum<T>::value, bool>
		serialize(const T v, write_stream& bs, const member_register* member)
		{
			return bs.write_varints((uint32_t)v);
		}


		static bool serialize(const int32_t v, write_stream& bs, const member_register* member)
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
	
		static bool serialize(const int64_t v, write_stream& bs, const member_register* member)
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

		static bool serialize(const uint32_t v, write_stream& bs, const member_register* member)
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
		static bool serialize(const uint64_t v, write_stream& bs, const member_register* member)
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

		static bool serialize(const float v, write_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write(v);
		}
		static bool serialize(const double v, write_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write(v);
		}

		static bool serialize(const std::string& v, write_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.write_string(v);
		}

		template<typename T>
		static bool serialize(const pb_repeated<T>& v, write_stream& bs, const member_register* member)
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
					l2 += bytesize(v[i], PB_BUILDIN_BYTESIZE_USE_CACHE, member);
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
		serialize(const pb_map<T>& v, write_stream& bs, const member_register* member)
		{
			typedef typename pb_map<T>::key_type key_type;
			typedef typename pb_map<T>::pair_type pair_type;

			static_assert(std::is_integral<key_type>::value
				|| std::is_same<key_type, std::string>::value, "illegal key type");

			static const pair_type item;
			auto& table = item.GetDescriptor()->get_member_table();
			static const size_t l01 =
				write_stream::bytesize_varints(table[0]->get_tag()) +
				write_stream::bytesize_varints(table[1]->get_tag());

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
				size_t l2 = l01;
				//l2 += write_stream::bytesize_varints(table[0]->get_tag());
				l2 += bytesize(p.first, PB_BUILDIN_BYTESIZE_USE_CACHE, table[0]);
				//l2 += write_stream::bytesize_varints(table[1]->get_tag());
				l2 += bytesize(p.second, PB_BUILDIN_BYTESIZE_USE_CACHE, table[1]);
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
		static bool serialize(const pb_optional<T>& v, write_stream& bs, const member_register* member)
		{
			if (!v.has()) {
				return true;
			}

			//if (!(member->get_flag() & PB_BUILDIN_FLAG_REPEATED))
			//{
			if (!bs.write_varints(member->get_tag())) {
				return false;
			}
			//}

			return serialize(v.get(), bs, member);
		}

		static bool serialize(uint32_t tag, const std::string& data, write_stream& bs)
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

		static bool serialize(const pb_message_base& v, write_stream& bs, const member_register* member)
		{
			//ignore_unused(member);
			
			auto& table = v.GetDescriptor()->get_member_table();
			//binary_stream bs2;
			if (member)
			{

				size_t l = 0;

				l += bytesize(v, PB_BUILDIN_BYTESIZE_USE_CACHE, nullptr);

				//for (auto& item : table) {
				//	l += item->bytesize(&v, PB_BUILDIN_BYTESIZE_USE_CACHE);
				//}

				//for (auto& item : v.GetUnknownFields()) {
				//	l += bytesize(item.first, item.second);
				//}

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

		static bool deserialize(bool& v, const read_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read(v);
		}
		static bool deserialize(std::vector<bool>::reference v, const read_stream& bs, const member_register* member)
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
		deserialize(T& v, const read_stream& bs, const member_register* member)
		{
			return bs.read_varints((uint32_t&)v);
		}

		static bool deserialize(int32_t& v, const read_stream& bs, const member_register* member)
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

		static bool deserialize(int64_t& v, const read_stream& bs, const member_register* member)
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

		static bool deserialize(uint32_t& v, const read_stream& bs, const member_register* member)
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
		static bool deserialize(uint64_t& v, const read_stream& bs, const member_register* member)
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

		static bool deserialize(float& v, const read_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read(v);
		}
		static bool deserialize(double& v, const read_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read(v);
		}

		static bool deserialize(std::string& v, const read_stream& bs, const member_register* member)
		{
			ignore_unused(member);
			return bs.read_string(v);
		}

		template<typename T>
		static bool deserialize(pb_repeated<T>& v, const read_stream& bs, const member_register* member)
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
					v.emplace_back();
					if (!deserialize(v.back(), bs, member)) {
						return false;
					}
				}

				return bs.pos() == l;
			}
			else
			{
				v.emplace_back();
				if (!deserialize(v.back(), bs, member)) {
					return false;
				}
				return true;
			}
		}
		
		template<typename T>
		static std::enable_if_t<std::is_base_of<pb_message_base, T>::value, bool>
		deserialize(pb_map<T>& v, const read_stream& bs, const member_register* member)
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
#if defined(PB_USE_CPP17)
			v.insert_or_assign(std::move(*item.mutable_key()), std::move(*item.mutable_value()));
#else
			v[std::move(*item.mutable_key())] = std::move(*item.mutable_value());
#endif
			return true;
		}

		template<typename T>
		static bool deserialize(pb_optional<T>& v, const read_stream& bs, const member_register* member)
		{
			return deserialize(*v.mutable_get(), bs, member);
		}

		static bool deserialize(pb_message_base& v, const read_stream& bs, const member_register* member)
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
		return binary_serializer::bytesize(pb, PB_BUILDIN_BYTESIZE_USE_CACHE, nullptr);
	}

	static bool serialize_to_binary(const pb_message_base& pb, std::string& buff)
	{
		buff.resize(bytesize_to_binary(pb));

		auto bs = binary_serializer::write_stream(const_cast<char*>(buff.data()), buff.size());
		bool ret = binary_serializer::serialize(pb, bs, nullptr);

		bs.release().release();
		return ret;
	}

	static std::unique_ptr<uint8_t[]> serialize_to_binary(const pb_message_base& pb, uint32_t& len)
	{
		len = bytesize_to_binary(pb);

		auto bs = binary_serializer::write_stream(len);
		if (!binary_serializer::serialize(pb, bs, nullptr)) {
			return nullptr;
		}

		return bs.release();
	}

	static bool	deserialize_from_binary(const std::string& buff, pb_message_base& pb)
	{
		auto bs = binary_serializer::read_stream(const_cast<char*>(buff.data()), buff.size());

		bool ret = binary_serializer::deserialize(pb, bs, nullptr);

		bs.release().release();
		return ret;
	}

	static bool deserialize_from_binary(const void* data, uint32_t len, pb_message_base& pb)
	{
		auto bs = binary_serializer::read_stream(const_cast<void*>(data), len);

		bool ret = binary_serializer::deserialize(pb, bs, nullptr);

		bs.release().release();
		return ret;
	}

}


#endif

#endif
