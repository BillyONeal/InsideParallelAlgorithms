// compile with:
//    debug: cl /EHsc /W4 /WX /std:c++latest /Festable_sort_debug /MDd /Zi .\demo_stable_sort.cpp
//  release: cl /EHsc /W4 /WX /std:c++latest /Festable_sort_release /MD /Zi /O2 .\demo_stable_sort.cpp
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <execution>
#include <ratio>
#include <random>
#include <charconv>
#include <vector>

using namespace std; // sorry Titus
using namespace std::chrono;
using namespace std::execution;

const int iterationCount = 10;

template<class ExPo>
steady_clock::duration run_stable_sort(vector<double>& doubles, ExPo&& ex,
	const char * const tag) {
	steady_clock::duration totalTime{};
	for (int i = 0; i < iterationCount; ++i)
	{
		const auto startTime = steady_clock::now();
		stable_sort(forward<ExPo>(ex), doubles.begin(), doubles.end());
		const auto endTime = steady_clock::now();
		totalTime += endTime - startTime;
		printf("%s: Min: %g Max: %g Time: %gms\n", tag, doubles.front(), doubles.back(),
			duration_cast<duration<double, milli>>(endTime - startTime).count());
	}

	return totalTime;
}

void print_diff(const char * const name,
	const steady_clock::duration baselineTime,
	const steady_clock::duration compareTime) {
	const char * direction;
	double times;
	if (baselineTime < compareTime) {
		times = static_cast<double>(compareTime.count()) / baselineTime.count();
		direction = "slower";
	}
	else {
		times = static_cast<double>(baselineTime.count()) / compareTime.count();
		direction = "faster";
	}

	printf("%s was %g times %s\n", name, times, direction);
}

int main(int argc, char **argv) {
	if (argc != 2) { printf("Usage: %s number\n", argv[0]); return 1; }
	size_t testSize;
	auto[ptr, ec] = from_chars(argv[1], argv[1] + strlen(argv[1]), testSize);
	if (ec != errc{}) { printf("Bad %s parse as integer.\n", argv[1]); return 2; }

	// generate some doubles:
	printf("Testing with %zu doubles (%gmb)...\n", testSize,
		static_cast<double>(testSize) * sizeof(double) / (1024*1024));
	vector<double> doubles;
	doubles.reserve(testSize);
	minstd_rand lce(1729);
	for (size_t idx = 0; idx < testSize; ++idx) {
		doubles.push_back(lce());
	}
	// time how long it takes to stable_sort them:
	const auto serialTime = run_stable_sort(doubles, seq, "Serial");
	const auto parallelTime = run_stable_sort(doubles, par, "Parallel");
	print_diff("Parallel", serialTime, parallelTime);
}
