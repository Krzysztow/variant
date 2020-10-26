#include <chrono>
#include <cstdio>
#include <iostream>

double bifurcation(const double x_, const uint64_t n)
{
  auto x = x_;

  for( uint64_t _ = 0; _ != n; ++_)
  {
    x = 4.0 * x * (1.0-x);
  }

  return x;
}

int main()
{
  std::cout << "C++ calc performance\n";

  const auto start = std::chrono::high_resolution_clock::now();

  std::cout.precision(16);
  std::cout << bifurcation(0.61, 3'000'000'000);

  const auto finish = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double, std::milli> elapsed = finish - start;
  std::cout << "\t" << elapsed.count() << "ms\n";
}
