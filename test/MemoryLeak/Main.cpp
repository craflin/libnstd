
#include <nstd/Memory.hpp>

class A
{
public:
  int a;
};

int main(int argc, char* argv[])
{
  new A; // leak #1
  A* b = new A; // no leak
  Memory::alloc(123); // leak #2
  void* d = Memory::alloc(123); // no leak
  
  delete b;
  Memory::free(d);

  return 0;
}
