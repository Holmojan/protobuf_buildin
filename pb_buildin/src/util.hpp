
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
	inline std::string make_upper(std::string name) {
		std::transform(name.begin(), name.end(), name.begin(),
			[](char c) {return (char)::toupper(c); });
		return name;
	}

	inline std::string en_base64(const void* data, size_t len)
	{
		std::string cipher;

		static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		if (len == 0) {
			return cipher;
		}

		cipher.reserve((len + 2) / 3 * 4);

		for (size_t i = 0; i < len; i += 3)
		{
			int p = (std::min)(len - i, (size_t)3);
			int q = (p * 8 + 5) / 6;

			uint32_t k = 0;
			for (int j = 0; j < p; j++) {
				k |= ((uint8_t*)data)[i + j] << 8 * (2 - j);
			}
			for (int j = 3; j >= 4 - q; j--) {
				cipher.push_back(table[(k >> j * 6) & 0x3f]);
			}
		}

		return cipher;
	}

	inline bool en_base64(const std::string& plain, std::string& cipher)
	{
		cipher = en_base64(plain.data(), plain.size());
		return true;
	}

	inline bool de_base64(const std::string& cipher, std::string& plain)
	{
		static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		static int inv_table[256] = { 0 };

		size_t cipher_size = cipher.size();
		while (cipher_size && cipher[cipher_size - 1] == '=') {
			cipher_size--;
		}

		if (!cipher_size) {
			plain.clear();
			return true;
		}

		if (cipher_size % 4 == 1) {
			return false;
		}

		if (inv_table[0] == 0) {
			memset(inv_table, -1, sizeof(inv_table));
			for (auto p = table; *p; ++p) {
				inv_table[*p] = p - table;
			}
		}

		std::string tmp;
		tmp.reserve((cipher_size + 3) / 4 * 3);

		for (size_t i = 0; i < cipher_size; i += 4)
		{
			int p = (std::min)(cipher_size - i, (size_t)4);
			int q = p * 6 / 8;

			uint32_t k = 0;
			for (int j = 0; j < p; j++)
			{
				int b = inv_table[cipher[i + j]];
				if (b == -1) {
					return false;
				}
				k |= b << 6 * (3 - j);
			}
			for (int j = 2; j >= 3 - q; j--) {
				tmp.push_back((k >> j * 8) & 0xff);
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