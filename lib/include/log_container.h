#include <cstdio>
#include <string>

template <typename T>
void LogContainer(const T& container, std::string_view format, std::string_view lbl = "") {
  printf("%s", lbl.data());
  for (const auto& item : container) {
    printf(format.data(), item);
    printf(", ");
  }
  printf("\n");
}