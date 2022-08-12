
#if !defined(__PB_BUILDIN__MACRO_HPP__)
#define __PB_BUILDIN__MACRO_HPP__


#	define PB_MESSAGE(_name)												\
		enum {_name##_PB_TYPE};												\
		class _name : public pb_message {									\
		protected:															\
			_name(class_register* p) : pb_message(p) { }					\
		public:																\
			static const class_register* GetDescriptor() {					\
				static class_register _register;							\
				static _name instance(&_register);							\
				return ((pb_message&)instance).GetDescriptor();}			\
			_name() : pb_message(GetDescriptor()) { }						\
			void Clear(){ *this = _name(); }

#	define PB_MESSAGE_END													\
		};

/////////////////////////////////////////////////////////////////////////////

#	define PB_MEMBER_INIT(_info, _raw_type, _type, _raw_var, _var, _num, _flag)	\
		{																	\
			_info->num = _num;												\
			_info->name = #_raw_var;										\
			_info->type = (proto_type)(_raw_type##_PB_TYPE);				\
			_info->flag = _flag;											\
			typedef std::decay_t<decltype(*this)> message;					\
			_info->offset = member_offsetof(message, _var);					\
			_info->size = member_sizeof(message, _var);						\
			_info->wire_type = get_wire_type((_raw_type*)nullptr,			\
				(proto_type)(_raw_type##_PB_TYPE));							\
			/*_info->type_info = &typeid(*/									\
				/*PB_BUILDIN_TYPE_IDENTITY(_type));*/						\
			_info->ptr_interface = serialize_implement<						\
			type_identity_t<_type>>::get_instance();						\
		}

#if defined(_MSC_VER) && _MSC_VER <= 1800

#	define PB_MEMBER(_raw_type, _type, _raw_var, _var, _num, _flag)			\
		protected: enum {PB_MEMBER_##_num};									\
			void init_##_var (member_info* info) { PB_MEMBER_INIT(info,		\
				 _raw_type, _type, _raw_var, _var, _num, _flag) }			\
			type_identity_t<_type> _var = { this,							\
				std::bind(&std::decay_t<decltype(*this)>::init_##_var,		\
					this, std::placeholders::_1) };							\
		public:

#else

#	define PB_MEMBER(_raw_type, _type, _raw_var, _var, _num, _flag)			\
		protected: enum {PB_MEMBER_##_num};									\
			type_identity_t<_type> _var = { this,							\
				[&] (member_info* info) { PB_MEMBER_INIT(info,				\
					_raw_type, _type, _raw_var, _var, _num, _flag) } };		\
		public:

#endif

/////////////////////////////////////////////////////////////////////////////

#	define PB_OPTIONAL_HAS(_type, _var)										\
		public: bool has_##_var()const{ return (_##_var).has(); }	
#	define PB_OPTIONAL_GET(_type, _var)										\
		public: const type_identity_t<_type>& _var()const{					\
			return (_##_var).get(); }											
#	define PB_OPTIONAL_SET(_type, _var)										\
		public: void set_##_var(const type_identity_t<_type>& v) {			\
			(_##_var).set(v); }												
#	define PB_OPTIONAL_MUTABLE(_type, _var)									\
		public: type_identity_t<_type>* mutable_##_var(){					\
			return (_##_var).mutable_get(); }		
#	define PB_OPTIONAL_CLEAR(_type, _var)									\
		public: void clear_##_var(){ (_##_var).clear(); }		

#	define PB_OPTIONAL(_type, _var, _num)									\
		PB_OPTIONAL_HAS(_type, _var)										\
		PB_OPTIONAL_GET(_type, _var)										\
		PB_OPTIONAL_SET(_type, _var)										\
		PB_OPTIONAL_MUTABLE(_type, _var)									\
		PB_OPTIONAL_CLEAR(_type, _var)										\
		PB_MEMBER(_type, pb_optional<_type>, _var, _##_var, _num, 0)

/////////////////////////////////////////////////////////////////////////////
#	define PB_REPEATED_SIZE(_type, _var)									\
		public: size_t _var##_size()const{ return (_##_var).size(); }	
#	define PB_REPEATED_ADD(_type, _var)										\
		public: type_identity_t<_type>* add_##_var(){						\
			(_##_var).emplace_back(); return &(_##_var).back(); }	
#	define PB_REPEATED_GET(_type, _var)										\
		public: const type_identity_t<_type>& _var(int index)const{			\
			return (_##_var)[index]; }										\
		public: const pb_repeated<_type>& _var()const {						\
			return (_##_var); }
#	define PB_REPEATED_MUTABLE(_type, _var)									\
		public: type_identity_t<_type>* mutable_##_var(int index){			\
			return &(_##_var)[index]; }										\
		public: pb_repeated<_type>* mutable_##_var(){						\
			return &(_##_var); }											
#	define PB_REPEATED_CLEAR(_type, _var)									\
		public: void clear_##_var(){ (_##_var).clear(); }		

#	define PB_REPEATED(_type, _var, _num)									\
		PB_REPEATED_SIZE(_type, _var)										\
		PB_REPEATED_ADD(_type, _var)										\
		PB_REPEATED_GET(_type, _var)										\
		PB_REPEATED_MUTABLE(_type, _var)									\
		PB_REPEATED_CLEAR(_type, _var)										\
		PB_MEMBER(_type, pb_repeated<_type>, _var, _##_var, _num,			\
			PB_BUILDIN_FLAG_REPEATED)

#	define PB_REPEATED_PACKED(_type, _var, _num)							\
		static_assert(0 != (_type##_PB_TYPE), "message can't packed!");		\
		PB_REPEATED_SIZE(_type, _var)										\
		PB_REPEATED_ADD(_type, _var)										\
		PB_REPEATED_GET(_type, _var)										\
		PB_REPEATED_MUTABLE(_type, _var)									\
		PB_REPEATED_CLEAR(_type, _var)										\
		PB_MEMBER(_type, pb_repeated<_type>, _var, _##_var, _num,			\
			PB_BUILDIN_FLAG_REPEATED | PB_BUILDIN_FLAG_PACKED)

/////////////////////////////////////////////////////////////////////////////

#	define PB_MAP_SIZE(_pair_type, _var)									\
		public: size_t _var##_size()const{ return (_##_var).size(); }	
#	define PB_MAP_GET(_pair_type, _var)										\
		public: const pb_map<_pair_type>& _var()const {						\
			return (_##_var); }
#	define PB_MAP_MUTABLE(_pair_type, _var)									\
		public: pb_map<_pair_type>* mutable_##_var(){						\
			return &(_##_var); }											
#	define PB_MAP_CLEAR(_pair_type, _var)									\
		public: void clear_##_var(){ (_##_var).clear(); }		

#	define PB_MAP(_key_type, _value_type, _var, _num)						\
		PB_MESSAGE(_var##_pair_type)										\
			PB_OPTIONAL(_key_type, key, 1)									\
			PB_OPTIONAL(_value_type, value, 2)								\
			public: typedef _key_type key_type;								\
			public: typedef _value_type value_type;							\
		PB_MESSAGE_END														\
		PB_MAP_SIZE(_var##_pair_type, _var)									\
		PB_MAP_GET(_var##_pair_type, _var)									\
		PB_MAP_MUTABLE(_var##_pair_type, _var)								\
		PB_MAP_CLEAR(_var##_pair_type, _var)								\
		PB_MEMBER(_var##_pair_type, pb_map<_var##_pair_type>,				\
			_var, _##_var, _num, PB_BUILDIN_FLAG_REPEATED)

/////////////////////////////////////////////////////////////////////////////

#	define PB_PACKAGE(_name)												\
		namespace _name { 

#	define PB_PACKAGE_END													\
		};

/////////////////////////////////////////////////////////////////////////////

#	define PB_ENUM(_name)													\
		enum {_name##_PB_TYPE};												\
		enum class _name : int32_t { 

#	define PB_ENUM_END														\
		};

#endif