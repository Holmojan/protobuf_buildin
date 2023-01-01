
#if !defined(__PB_BUILDIN__MACRO_HPP__)
#define __PB_BUILDIN__MACRO_HPP__


#	define PB_MESSAGE(_name)												\
		enum {PB_TYPE(_name)};												\
		class _name : public pb_message {									\
		protected:															\
			_name(class_register* p) : pb_message(p) { }					\
		public:																\
			static const class_register* GetDescriptor() {					\
				static class_register _register;							\
				static _name instance(&_register);							\
				return ((pb_message&)instance).GetDescriptor();}			\
			_name() : pb_message(GetDescriptor()) { }

#	define PB_MESSAGE_END													\
		};

/////////////////////////////////////////////////////////////////////////////
#	define PB_MEMBER_NUM(_num)		PB_MEMBER_##_num
#	define PB_MEMBER_VAR(_var)		_##_var
#	define PB_REPEATED_MUTABLE_HELPER(_type)								\
		pb_repeated_mutable_helper<type_identity_t<_type>>

#	define PB_MEMBER_INIT(_info, _raw_type, _type, _var, _num, _flag)		\
		{																	\
			_info->num = _num;												\
			_info->name = #_var;											\
			_info->type = (proto_type)(PB_TYPE(_raw_type));					\
			_info->flag = _flag;											\
			typedef std::decay_t<decltype(*this)> message;					\
			_info->offset = member_offsetof(message, PB_MEMBER_VAR(_var));	\
			_info->size = member_sizeof(message, PB_MEMBER_VAR(_var));		\
			_info->wire_type = get_wire_type((_raw_type*)nullptr,			\
				(proto_type)(PB_TYPE(_raw_type)));							\
			/*_info->type_info = &typeid(*/									\
				/*PB_BUILDIN_TYPE_IDENTITY(_type));*/						\
			_info->ptr_interface = serialize_implement<						\
				type_identity_t<_type>>::get_instance();					\
		}

#if defined(_MSC_VER) && _MSC_VER <= 1800

#	define PB_MEMBER(_raw_type, _type, _var, _num, _flag)					\
		protected: enum {PB_MEMBER_NUM(_num)};								\
			void init_##_var (member_info* info) { PB_MEMBER_INIT(info,		\
				 _raw_type, _type, _var, _num, _flag) }						\
			type_identity_t<_type> PB_MEMBER_VAR(_var) = { this,			\
				std::bind(&std::decay_t<decltype(*this)>::init_##_var,		\
					this, std::placeholders::_1) };							\
		public:

#else

#	define PB_MEMBER(_raw_type, _type, _var, _num, _flag)					\
		protected: enum {PB_MEMBER_NUM(_num)};								\
			type_identity_t<_type> PB_MEMBER_VAR(_var) = { this,			\
				[&] (member_info* info) { PB_MEMBER_INIT(info,				\
					_raw_type, _type, _var, _num, _flag) } };				\
		public:

#endif

/////////////////////////////////////////////////////////////////////////////

#	define PB_OPTIONAL_HAS(_type, _var)										\
		public: bool has_##_var()const{										\
			return PB_MEMBER_VAR(_var).has(); }	
#	define PB_OPTIONAL_GET(_type, _var)										\
		public: const type_identity_t<_type>& _var()const{					\
			return PB_MEMBER_VAR(_var).get(); }											
#	define PB_OPTIONAL_SET(_type, _var)										\
		public: void set_##_var(const type_identity_t<_type>& v) {			\
			PB_MEMBER_VAR(_var).set(v); }												
#	define PB_OPTIONAL_MUTABLE(_type, _var)									\
		public: type_identity_t<_type>* mutable_##_var(){					\
			return PB_MEMBER_VAR(_var).mutable_get(); }		
#	define PB_OPTIONAL_CLEAR(_type, _var)									\
		public: void clear_##_var(){ PB_MEMBER_VAR(_var).clear(); }		

#	define PB_OPTIONAL(_type, _var, _num)									\
		PB_OPTIONAL_HAS(_type, _var)										\
		PB_OPTIONAL_GET(_type, _var)										\
		PB_OPTIONAL_SET(_type, _var)										\
		PB_OPTIONAL_MUTABLE(_type, _var)									\
		PB_OPTIONAL_CLEAR(_type, _var)										\
		PB_MEMBER(_type, pb_optional<_type>, _var, _num, 0)

/////////////////////////////////////////////////////////////////////////////
#	define PB_REPEATED_SIZE(_type, _var)									\
		public: size_t _var##_size()const{									\
			return PB_MEMBER_VAR(_var).size(); }	
#	define PB_REPEATED_ADD(_type, _var)										\
		public: PB_REPEATED_MUTABLE_HELPER(_type)::type add_##_var(){		\
			PB_MEMBER_VAR(_var).emplace_back();								\
			return PB_REPEATED_MUTABLE_HELPER(_type)()(						\
				PB_MEMBER_VAR(_var).back()); }								\
		public: void add_##_var(const type_identity_t<_type>& v) {			\
			PB_MEMBER_VAR(_var).emplace_back(v); }									
#	define PB_REPEATED_GET(_type, _var)										\
		public: const type_identity_t<_type>& _var(int index)const{			\
			return PB_MEMBER_VAR(_var)[index]; }							\
		public: const pb_repeated<_type>& _var()const {						\
			return PB_MEMBER_VAR(_var); }
#	define PB_REPEATED_MUTABLE(_type, _var)									\
		public: PB_REPEATED_MUTABLE_HELPER(_type)::type						\
				mutable_##_var(int index){									\
			return PB_REPEATED_MUTABLE_HELPER(_type)()(						\
				PB_MEMBER_VAR(_var)[index]); }								\
		public: pb_repeated<_type>* mutable_##_var(){						\
			return &PB_MEMBER_VAR(_var); }											
#	define PB_REPEATED_CLEAR(_type, _var)									\
		public: void clear_##_var(){ PB_MEMBER_VAR(_var).clear(); }		

#	define PB_REPEATED(_type, _var, _num)									\
		PB_REPEATED_SIZE(_type, _var)										\
		PB_REPEATED_ADD(_type, _var)										\
		PB_REPEATED_GET(_type, _var)										\
		PB_REPEATED_MUTABLE(_type, _var)									\
		PB_REPEATED_CLEAR(_type, _var)										\
		PB_MEMBER(_type, pb_repeated<_type>, _var, _num,					\
			PB_BUILDIN_FLAG_REPEATED)

#	define PB_REPEATED_PACKED(_type, _var, _num)							\
		static_assert(0 != PB_TYPE(_type), "message can't packed!");		\
		PB_REPEATED_SIZE(_type, _var)										\
		PB_REPEATED_ADD(_type, _var)										\
		PB_REPEATED_GET(_type, _var)										\
		PB_REPEATED_MUTABLE(_type, _var)									\
		PB_REPEATED_CLEAR(_type, _var)										\
		PB_MEMBER(_type, pb_repeated<_type>, _var, _num,					\
			PB_BUILDIN_FLAG_REPEATED | PB_BUILDIN_FLAG_PACKED)

/////////////////////////////////////////////////////////////////////////////

#	define PB_MAP_SIZE(_pair_type, _var)									\
		public: size_t _var##_size()const{									\
			return PB_MEMBER_VAR(_var).size(); }	
#	define PB_MAP_GET(_pair_type, _var)										\
		public: const pb_map<_pair_type>::value_type& _var(					\
			const pb_map<_pair_type>::key_type& key)const{					\
			return PB_MEMBER_VAR(_var).get(key); }							\
		public: const pb_map<_pair_type>& _var()const {						\
			return PB_MEMBER_VAR(_var); }
#	define PB_MAP_MUTABLE(_pair_type, _var)									\
		public: pb_map<_pair_type>::value_type* mutable_##_var(				\
			const pb_map<_pair_type>::key_type& key){						\
			return &PB_MEMBER_VAR(_var)[key]; }								\
		public: pb_map<_pair_type>* mutable_##_var(){						\
			return &PB_MEMBER_VAR(_var); }											
#	define PB_MAP_CLEAR(_pair_type, _var)									\
		public: void clear_##_var(){ PB_MEMBER_VAR(_var).clear(); }		

#	define PB_MAP_PAIR(_var)	_var##_PB_MAP_PAIR

#	define PB_MAP(_key_type, _value_type, _var, _num)						\
		PB_MESSAGE(PB_MAP_PAIR(_var))										\
			PB_OPTIONAL(_key_type, key, 1)									\
			PB_OPTIONAL(_value_type, value, 2)								\
			public: typedef _key_type key_type;								\
			public: typedef _value_type value_type;							\
		PB_MESSAGE_END														\
		PB_MAP_SIZE(PB_MAP_PAIR(_var), _var)								\
		PB_MAP_GET(PB_MAP_PAIR(_var), _var)									\
		PB_MAP_MUTABLE(PB_MAP_PAIR(_var), _var)								\
		PB_MAP_CLEAR(PB_MAP_PAIR(_var), _var)								\
		PB_MEMBER(PB_MAP_PAIR(_var), pb_map<PB_MAP_PAIR(_var)>, _var, _num, \
			PB_BUILDIN_FLAG_REPEATED)

/////////////////////////////////////////////////////////////////////////////

#	define PB_PACKAGE(_name)												\
		namespace _name { 

#	define PB_PACKAGE_END													\
		};

/////////////////////////////////////////////////////////////////////////////

#	define PB_ENUM(_name)													\
		enum {PB_TYPE(_name)};												\
		enum class _name : int32_t { 

#	define PB_ENUM_END														\
		};

#endif