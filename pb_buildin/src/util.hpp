
#if !defined(__PB_BUILDIN__UTIL_HPP__)
#define __PB_BUILDIN__UTIL_HPP__

namespace pb_buildin {
	/*
	inline std::string ws_to_utf8(const std::wstring& src) {
		std::wstring_convert<std::codecvt_utf8<std::wstring::value_type>> conv;
		return conv.to_bytes(src);
	}

	inline std::wstring utf8_to_ws(const std::string& src) {
		std::wstring_convert<std::codecvt_utf8<std::wstring::value_type>> conv;
		return conv.from_bytes(src);
	}
	*/
	inline std::string make_lower(std::string name) {
		std::transform(name.begin(), name.end(), name.begin(),
			[](char c) {return (char)::tolower(c); });
		return name;
	}

	inline bool en_base64(const std::string& plain, std::string& cipher)
	{
		static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		if (plain.empty()) {
			cipher.clear();
			return true;
		}

		std::string tmp;
		tmp.reserve((plain.size() + 2) / 3 * 4);

		for (size_t i = 0; i < plain.size(); i += 3)
		{
			int p = (std::min)(plain.size() - i, (size_t)3);
			int q = (p * 8 + 5) / 6;

			uint32_t k = 0;
			for (int j = 0; j < p; j++) {
				k |= (uint8_t)plain[i + j] << 8 * j;
			}
			for (int j = 0; j < q; j++) {
				tmp.push_back(table[k & 0x3f]);
				k >>= 6;
			}
		}

		cipher = std::move(tmp);
		return true;
	}

	inline bool de_base64(const std::string& cipher, std::string& plain)
	{
		static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		static int inv_table[256] = { 0 };


		if (cipher.empty()) {
			plain.clear();
			return true;
		}

		if (cipher.size() % 4 == 1) {
			return false;
		}

		if (inv_table[0] == 0) {
			memset(inv_table, -1, sizeof(inv_table));
			for (auto p = table; *p; ++p) {
				inv_table[*p] = p - table;
			}
		}

		std::string tmp;
		tmp.reserve((cipher.size() + 3) / 4 * 3);

		for (size_t i = 0; i < cipher.size(); i += 4)
		{
			int p = (std::min)(cipher.size() - i, (size_t)4);
			int q = p * 6 / 8;

			uint32_t k = 0;
			for (int j = 0; j < p; j++)
			{
				int b = inv_table[cipher[i + j]];
				if (b == -1) {
					return false;
				}
				k |= b << 6 * j;
			}
			for (int j = 0; j < q; j++) {
				tmp.push_back(k & 0xff);
				k >>= 8;
			}
		}

		plain = std::move(tmp);
		return true;
	}

    inline uint32_t en_zigzag32(int32_t n) {
        return (n << 1) ^ ((n >> 31) & 1);
    }

    inline int32_t de_zigzag32(uint32_t n) {
        return (n >> 1) ^ (0 - (n & 1));
    }

    inline uint64_t en_zigzag64(int64_t n) {
        return (n << 1) ^ ((n >> 63) & 1);
    }

    inline int64_t de_zigzag64(uint64_t n) {
        return (n >> 1) ^ (0 - (n & 1));
    }

	template<typename T>
	inline void ignore_unused(const T&) {}


#if !defined(member_offsetof)
#	define member_offsetof(s,m) (size_t)&(((s *)0)->m)
#endif

#if !defined(member_sizeof)
#	define member_sizeof(s,m) sizeof(((s *)0)->m)
#endif
	
	template<typename T>
	class type_identity {
	public:
		typedef T type;
	};

	template<typename T>
	using type_identity_t = typename type_identity<T>::type;
}

#endif