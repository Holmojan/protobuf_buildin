
//#define PB_BUILDIN__USE_JSON_SERIALIZER
#define PB_BUILDIN__USE_BINARY_SERIALIZER
#include "pb_buildin/pb_buildin.hpp"

namespace pb_buildin {

#include "declare.hpp"

}

int main()
{
	pb_buildin::Test::A a;
	a.set_x(123);
	auto s = a.SerializeAsString();

	return 0;
}
