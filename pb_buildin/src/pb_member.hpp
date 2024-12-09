
#if !defined(__PB_BUILDIN__PB_MEMBER_HPP__)
#define __PB_BUILDIN__PB_MEMBER_HPP__

namespace pb_buildin {

	class pb_message_base
	{
	protected:
		std::vector<std::pair<uint32_t, std::string>> _unknown_fields;
		class_register* _instance_register = nullptr;
		const class_register* _register = nullptr;
		//bool _resist_instance = false;
	public:
		const std::vector<std::pair<uint32_t, std::string>>& GetUnknownFields()const {
			return _unknown_fields;
		}
		void AddUnknownFields(uint32_t tag, const std::string& data) {
			_unknown_fields.emplace_back(tag, data);
		}
		const class_register* GetDescriptor()const {
			return _instance_register ? _instance_register : _register;
		}
		void RegistMember(std::function<void(member_info*)> func)
		{
			if (_instance_register) {
				_instance_register->push_member_table(func);
			}
		}

		void Clear() 
		{ 
			auto& table = GetDescriptor()->get_member_table();
			for (auto& item : table) {
				item->clear(this);
			}
			_unknown_fields.clear();
		}

		void Swap(pb_message_base& v)
		{
			auto& table = GetDescriptor()->get_member_table();
			for (auto& item : table) {
				item->swap(this, &v);
			}
			_unknown_fields.swap(v._unknown_fields);
		}

		template<uint32_t>
		struct pb_member_num {
			enum { value = 0 };
		};

		pb_message_base(const class_register* p) :
			_instance_register(nullptr), _register(p) {}

		pb_message_base(class_register* p) :
			_instance_register(p), _register(nullptr) {}
	};


	template<typename P>
	class pb_map :
		public std::map<typename P::key_type, typename P::value_type>
	{
	public:
		typedef P pair_type;
		typedef typename P::key_type key_type;
		typedef typename P::value_type value_type;
		typedef std::map<key_type, value_type> map_type;

		pb_map(pb_message_base* message, std::function<void(member_info*)> func) {
			message->RegistMember(func);
		}
		~pb_map() {}

		value_type at_or(const key_type& k, const value_type& d = {})const {
			auto itor = this->find(k);
			return (itor != this->end() ? itor->second : d);
		}

		const value_type& get(const key_type& k)const {
			static value_type def = {};
			auto itor = this->find(k);
			return (itor != this->end() ? itor->second : def);
		}
	};

	template<typename T>
	class pb_repeated :
		public std::vector<T>
	{
	public:
		pb_repeated(pb_message_base* message, std::function<void(member_info*)> func) {
			message->RegistMember(func);
		}
		~pb_repeated() {}
	};


	template<typename T>
	class pb_repeated_helper {
	public:
		typedef T* mutable_type;
		static mutable_type mutable_get(T& r) { return &r; }

		typedef const T& get_type;
		static get_type get(const T& v) { return v; }
	};

	template<>
	class pb_repeated_helper<bool> {
	public:
		typedef std::vector<bool>::reference mutable_type;
		static mutable_type mutable_get(std::vector<bool>::reference r) { return r; }

		typedef bool get_type;
		static get_type get(bool v) { return v; }
	};

	template<typename T>
	class pb_optional
	{
#if defined(PB_USE_CPP17)
	protected:
		std::optional<T> _val;
	public:
		T* mutable_get() {
			if (!_val) {
				_val = std::make_optional<T>();
			}
			return &_val.value();
		}
		void set(const T& v) {
			_val = std::make_optional<T>(v);
		}
		void set(T&& v) {
			_val = std::make_optional<T>(std::move(v));
		}
#else
	protected:
		std::unique_ptr<T> _val;
	public:
		T* mutable_get() {
			if (!_val) {
				_val = std::make_unique<T>();
			}
			return _val.get();
		}
		void set(const T& v) {
			_val = std::make_unique<T>(v);
		}
#endif
	public:
		typedef T value_type;

		pb_optional(pb_message_base* message, std::function<void(member_info*)> func) {
			message->RegistMember(func);
		}
		~pb_optional() {}

		pb_optional(const pb_optional& v)
		{
			if (v.has()) { 
				set(v.get());
			}
		}
		pb_optional& operator=(const pb_optional& v)
		{
			if (v.has()) {
				set(v.get());
			}
			else {
				clear();
			}
			return *this;
		}

		const T& get()const
		{
			static T def = {};
			return (_val ? *_val : def);
		}

		bool has()const {
			return !!_val;
		}
		void clear() {
			_val.reset();
		}
		void swap(pb_optional& v) {
			_val.swap(v._val);
		}
	};

}
#endif
