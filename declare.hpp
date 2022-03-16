
PB_PACKAGE(Test)

	PB_MESSAGE(A)
		PB_OPTIONAL(sint32, x, 1)
		PB_OPTIONAL(bool, y, 2)
		PB_OPTIONAL(string, z, 3)

		PB_OPTIONAL(bytes, t, 4)

		PB_MAP(int32,string,m,1024)
	PB_MESSAGE_END

PB_PACKAGE_END