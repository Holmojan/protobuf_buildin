
//#define PB_BUILDIN__USE_JSON_SERIALIZER
#define PB_BUILDIN__USE_BINARY_SERIALIZER
#include "pb_buildin/pb_buildin.hpp"

namespace pb_buildin {

	PB_PACKAGE(Test)

		PB_MESSAGE(A)
			PB_OPTIONAL(sint32, x, 1)
			PB_OPTIONAL(bool, y, 2)
			PB_OPTIONAL(string, z, 3)

			PB_OPTIONAL(bytes, t, 4)

			PB_MAP(int32, string, m, 1024)
		PB_MESSAGE_END

	PB_PACKAGE_END

}

int main()
{
	pb_buildin::Test::A a;
	a.set_x(123);
	auto s = a.SerializeAsString();

	return 0;
}
