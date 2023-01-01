
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
		void regist_member(std::function<void(member_info*)> func)
		{
			if (_instance_register) {
				_instance_register->push_member_table(func);
			}
		}

		void Clear() 
		{ 
			auto table = GetDescriptor()->get_member_table();
			for (auto& item : table) {
				item->clear(this);
			}
			_unknown_fields.clear();
		}


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
			message->regist_member(func);
		}
		~pb_map() {}

		value_type at_or(const key_type& k, const value_type& d = {})const {
			auto itor = find(k);
			return (itor != end() ? itor->second : d);
		}

		const value_type& get(const key_type& k)const {
			static value_type def = {};
			auto itor = find(k);
			return (itor != end() ? itor->second : def);
		}
	};

	template<typename T>
	class pb_repeated :
		public std::vector<T>
	{
	public:
		pb_repeated(pb_message_base* message, std::function<void(member_info*)> func) {
			message->regist_member(func);
		}
		~pb_repeated() {}
	};


	template<typename T>
	class pb_repeated_mutable_helper {
	public:
		typedef T* type;
		type operator()(T& r) { return &r; }
	};

	template<>
	class pb_repeated_mutable_helper<bool> {
	public:
		typedef std::vector<bool>::reference type;
		type operator()(std::vector<bool>::reference& r) { return r; }
	};

	template<typename T>
	class pb_optional
	{
	protected:
		std::unique_ptr<T> _ptr;
	public:
		typedef T value_type;

		pb_optional(pb_message_base* message, std::function<void(member_info*)> func) {
			message->regist_member(func);
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
			return (_ptr ? *_ptr : def);
		}
		T* mutable_get()
		{
			if (!_ptr) {
				_ptr = std::make_unique<T>();
			}
			return _ptr.get();
		}
		void set(const T& v) {
			_ptr = std::make_unique<T>(v);
		}
		bool has()const {
			return !!_ptr;
		}
		void clear() {
			_ptr.reset();
		}
	};

}
#endif
