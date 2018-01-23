#include <stdio.h>


//
// Example enum
//
enum TestEnum
{
	// List your enums
	// They don't *actually* have to be sequential and you can assign any values
	// you want. However, the bigger the values, the longer it will take to find
	// your enum string!
	Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Monkeys,

	// Uniquely named total count
	// With enum classes this could be made a little simpler
	TestEnum_Count
};


//
// The magic.
// This is terrible and there are ways to make it even more useful.
// Please send help, I should not be writing crap like this!
//
template <typename ENUM_TYPE, ENUM_TYPE ENUM_VALUE>
const char* EnumName()
{
	return __PRETTY_FUNCTION__ + 59;
}
template <typename ENUM_TYPE, ENUM_TYPE ENUM_COUNT, ENUM_TYPE ENUM_VALUE>
struct EnumMatch
{
	static const char* Do(ENUM_TYPE enum_value)
	{
		if(enum_value == ENUM_VALUE)
			return EnumName<ENUM_TYPE, ENUM_VALUE>();
		return EnumMatch<ENUM_TYPE, ENUM_COUNT, ENUM_TYPE(ENUM_VALUE + 1)>::Do(enum_value);
	}
};

template <typename ENUM_TYPE, ENUM_TYPE ENUM_COUNT>
struct EnumMatch<ENUM_TYPE, ENUM_COUNT, ENUM_COUNT>
{
	static const char* Do(ENUM_TYPE enum_value)
	{
		return "Enum not found";
	}
};
#define ENUM_NAME(enum_type, enum_value) \
	EnumMatch<enum_type, enum_type##_Count, enum_type(0)>::Do(enum_value)


int main()
{
	for(int i = 0; i < TestEnum_Count; i++)
		printf("%s\n", ENUM_NAME(TestEnum, TestEnum(i)));
	return 0;
}
