// compile with:
//    debug: cl /EHsc /W4 /WX /std:c++latest /Fereduce_debug /MDd /Zi .\Source.cpp
//  release: cl /EHsc /W4 /WX /std:c++latest /Fereduce_release /MD /Zi /O2 .\Source.cpp
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <execution>
#include <ratio>
#include <charconv>
#include <vector>

using namespace std; // sorry Titus
using namespace std::chrono;
using namespace std::execution;

const int iterationCount = 10;

template<class ExPo>
steady_clock::duration run_reduce(vector<double>& doubles, ExPo&& ex,
	const char * const tag) {
	steady_clock::duration totalTime{};
	for (int i = 0; i < iterationCount; ++i)
	{
		const auto startTime = steady_clock::now();
		double sum = reduce(forward<ExPo>(ex), doubles.begin(), doubles.end());
		const auto endTime = steady_clock::now();
		totalTime += endTime - startTime;
		printf("%s: Sum: %g Time: %gms\n", tag, sum,
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
	vector doubles(testSize, 1.2);
	// time how long it takes to reduce them:
	const auto serialTime = run_reduce(doubles, seq, "Serial");
	const auto parallelTime = run_reduce(doubles, par, "Parallel");
	print_diff("Parallel", serialTime, parallelTime);
}
