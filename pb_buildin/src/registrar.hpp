
#if !defined(__PB_BUILDIN__REGISTER_HPP__)
#define __PB_BUILDIN__REGISTER_HPP__

namespace pb_buildin {

	struct member_info {
		const char*				name;
		proto_type				type;
		//const std::type_info*	type_info;

		uint32_t				flag;
		size_t					offset;
		size_t					size;

		uint32_t				num;
		uint32_t				wire_type;
		uint32_t				tag;

		serialize_interface* ptr_interface;
	};


	class member_register {
	protected:
		member_info info = {};
		member_register(const member_register&) = delete;
	public:
		member_register(const std::function<void(member_info*)>& func) {
			func(&info);
		}

		const char* get_name()const {
			//return pb_buildin::make_lower(info.name);
			return info.name;
		}

		proto_type get_type()const {
			return info.type;
		}

		uint32_t get_flag()const { return info.flag; }

		uint32_t get_tag()const { return info.tag; }
		/* {
			if (info.flag & PB_BUILDIN_FLAG_REPEATED) {
				if (info.flag & PB_BUILDIN_FLAG_PACKED) {
					return (info.num << 3 | 2);
				}
			}
			return info.num << 3 | info.wire_type;
		}*/

		size_t get_offset()const { return info.offset; }

		void clear(void* base_ptr)const {
			info.ptr_interface->clear(static_cast<uint8_t*>(base_ptr) + info.offset);
		}

		void swap(void* base_ptr, void* base_ptr2)const {
			info.ptr_interface->swap(
				static_cast<uint8_t*>(base_ptr) + info.offset,
				static_cast<uint8_t*>(base_ptr2) + info.offset);
		}

		template<typename U>
		bool serialize(const void* base_ptr, const U& u)const {
			return info.ptr_interface->serialize(static_cast<const uint8_t*>(base_ptr) + info.offset, u, this);
		}
		template<typename U>
		bool deserialize(void* base_ptr, const U& u)const {
			return info.ptr_interface->deserialize(static_cast<uint8_t*>(base_ptr) + info.offset, u, this);
		}
		template<typename U = void>
		size_t bytesize(const void* base_ptr, uint32_t flags)const {
			return info.ptr_interface->bytesize(static_cast<const uint8_t*>(base_ptr) + info.offset, flags, this);
		}
	};

	class class_register
	{
	protected:
		std::vector<const member_register*> _member_table;
	public:
		class_register() {}
		
		~class_register() 
		{
			for (auto& p : _member_table) {
				delete p;
			}
			_member_table.clear();
		}
		
		const std::vector<const member_register*>& get_member_table()const {
			return _member_table;
		}
		void push_member_table(const std::function<void(member_info*)>& func) {
			_member_table.emplace_back(new member_register(func));
		}
	};

	//template<typename T>
	//T register_backtracking(class_register<T>*);

}

#endif
