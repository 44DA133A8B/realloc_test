#include <array>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <limits>

#if _WIN32
	#define NOMINMAX
	#include <windows.h>
#elif __linux__
	#include <unistd.h>
#endif

constexpr size_t ALLOCATION_NUM = 256;
constexpr size_t ALLOCATION_SIZE = 1024;
constexpr size_t SIZE_MULTIPLIER = 2;

int main(int argc, const char* argv[])
{
	std::array<void*, ALLOCATION_NUM> allocations = { };

	size_t worst_cases = 0;
	for (size_t i = 0; i < allocations.size(); i++)
	{
		void* memory = malloc(ALLOCATION_SIZE);
		memset(memory, 0xff, ALLOCATION_SIZE);
		void* new_memory = realloc(memory, ALLOCATION_SIZE * SIZE_MULTIPLIER);
		memset(new_memory, 0xff, ALLOCATION_SIZE * SIZE_MULTIPLIER);

		allocations[i] = new_memory;
		worst_cases += (memory != new_memory ? 1 : 0);
	}

	size_t min_address = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < allocations.size(); i++)
		min_address = std::min(min_address, (size_t)allocations[i]);

#if _WIN32
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	const size_t page_size = info.dwPageSize;
#elif __linux__
	const size_t page_size = sysconf(_SC_PAGE_SIZE);
#endif

	std::cout << "page_size: " << page_size << std::endl;

	for (size_t i = 0; i < allocations.size(); i++)
	{
		std::cout << "realloc" << i << ": " << '\t' << (size_t)allocations[i] - min_address;
		std::cout << '\t' << "offset_in_page: " << (size_t)allocations[i] % page_size;

		if (i > 0)
		{
			const size_t size = ALLOCATION_SIZE * SIZE_MULTIPLIER;
			const size_t prev_memory_end = (size_t)allocations[i - 1] + size;
			const size_t current_memory_begin = (size_t)allocations[i];
			std::cout << '\t' << "distance: " << current_memory_begin - prev_memory_end;
		}

		std::cout << std::endl;
	}

	for (size_t i = 0; i < allocations.size(); i++)
		free(allocations[i]);

	std::cout << worst_cases << "/" << ALLOCATION_NUM << " worst cases. ";
	std::cout << "(malloc_size: " << ALLOCATION_SIZE << "B realloc_size: " << ALLOCATION_SIZE * SIZE_MULTIPLIER << "B)";
	std::cout << std::endl;
}
