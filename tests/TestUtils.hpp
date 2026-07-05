#include <iostream>

#define RESET "\033[0m"
#define RED   "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN  "\033[1;36m"
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef bool (*TestFn)();

struct Test
{
	const char*	name;
	TestFn		fn;
};

#define ASSERT(condition) \
	if (!(condition)) { \
		std::cerr << RED <<"FAIL: " << #condition \
				  << " (" << __FILE__ << ":" << __LINE__ << ")\n" << RESET; \
		return false; \
	}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef bool (*TestFn)();

struct Test
{
	const char*	name;
	TestFn		fn;
};

inline void printTestSummary(const char* name, int passed, int failed)
{
	std::cout << GREEN << name << ": " << passed << "/" << (passed + failed) << " passed" << RESET;
	if (failed > 0)
		std::cout << " (" << failed << " failed)";
	std::cout << "\n";
}
