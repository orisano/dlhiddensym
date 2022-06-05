# dlhiddensym
[![test](https://github.com/orisano/dlhiddensym/actions/workflows/test.yml/badge.svg)](https://github.com/orisano/dlhiddensym/actions/workflows/test.yml)

dlhiddensym is a library that lookup hidden symbols in the dynamic link library.

## Installation
```bash
curl -SsLO https://raw.githubusercontent.com/orisano/dlhiddensym/main/dlhiddensym.h
```

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
