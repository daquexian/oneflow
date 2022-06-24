#include <iostream>

void f() {
  std::cout << "f" << std::endl;
}

int main() {
  auto a = f;
  a();
  (******a)();
  return 0;
}
