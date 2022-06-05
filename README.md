# dlhiddensym
dlhiddensym is a library that lookup hidden symbols in the dynamic link library.

## How to use
```c
#include "dlhiddensym.h"

int main() {
  void *addr = dlhiddensym("libjvm.so", "_ZN6Method26checked_resolve_jmethod_idEP10_jmethodID");
  return 0;
}
```

## Author
Nao Yonashiro

## License
Apache 2.0
