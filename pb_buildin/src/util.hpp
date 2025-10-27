
#if !defined(__PB_BUILDIN__UTIL_HPP__)
#define __PB_BUILDIN__UTIL_HPP__

namespace pb_buildin {

#if defined(_MSC_VER) && _MSC_VER <= 1800
	inline std::recursive_mutex& get_static_constructor_lock(int init = 0) {
		static std::atomic<int> flag = 0;
		assert(0 != (flag += init) && "need call pb_buildin_init");
		static std::recursive_mutex m;
		return m;
	}
#endif
	inline void pb_buildin_init() {
#if defined(_MSC_VER) && _MSC_VER <= 1800
		get_static_constructor_lock(1);
#endif
	}
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

	inline std::string en_base64(const void* data, size_t len, bool padding = false)
	{
		std::string cipher;

		PB_STATIC(char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

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
				k |= (static_cast<const uint8_t*>(data))[i + j] << 8 * (2 - j);
			}
			for (int j = 3; j >= 4 - q; j--) {
				cipher.push_back(table[(k >> j * 6) & 0x3f]);
			}
		}

		if (padding) {
			while (cipher.size() % 4 != 0) {
				cipher.push_back('=');
			}
		}

		return cipher;
	}

	inline bool en_base64(const std::string& plain, std::string& cipher, bool padding = false)
	{
		cipher = en_base64(plain.data(), plain.size(), padding);
		return true;
	}

	inline bool de_base64(const std::string& cipher, std::string& plain)
	{
		PB_STATIC(char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
		PB_STATIC(int inv_table[256] = { 0 });

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


	inline uint32_t log2_floor_nonzero(uint32_t n) {
#if defined(__GNUC__)
		return 31 ^ static_cast<uint32_t>(__builtin_clz(n));
#elif defined(_MSC_VER)
		unsigned long where;
		_BitScanReverse(&where, n);
		return where;
#else
		uint32_t l = 0;
		if (n >> 16) { l += 16; n >>= 16; }
		if (n >> 8) { l += 8; n >>= 8; }
		if (n >> 4) { l += 4; n >>= 4; }
		if (n >> 2) { l += 2; n >>= 2; }
		if (n >> 1) { l += 1; n >>= 1; }
		assert(n == 1);
		return l;
#endif
	}


#if !defined(member_offsetof)
#	define member_offsetof(s,m) (size_t)&(((s *)0)->m)
#endif

#if !defined(member_sizeof)
#	define member_sizeof(s,m) sizeof(((s *)0)->m)
#endif
	
	template<typename T>
	struct type_identity {
		typedef T type;
	};

	template<typename T>
	using type_identity_t = typename type_identity<T>::type;


	template<typename... ARGS>
	struct type_select;

	template<typename T>
	struct type_select<T> {
		typedef T type;
	};

	template<typename T, typename... ARGS>
	struct type_select<T, ARGS...> {
		typedef typename type_select<ARGS...>::type type;
	};

	template<typename... ARGS>
	using type_select_t = typename type_select<ARGS...>::type;


}

#endif