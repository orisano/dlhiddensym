// Copyright 2022 Nao Yonashiro
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dlhiddensym.h"
#include <assert.h>

int main() {
  memory_map_t m;
  assert(lookup_memory_map(&m, "testdata/notfound", "libjvm.so") == 0);
  assert(lookup_memory_map(&m, "testdata/maps", "libjvm.so") != 0);
  assert(strtoull(m.begin, NULL, 16) == 0x7f4f5d420000ull);
  assert(strtoull(m.offset, NULL, 16) == 0);
  assert(m.perms[0] == 'r');
  assert(m.perms[2] == 'x');
  assert(strcmp(m.pathname, "/usr/local/openjdk-8/lib/amd64/server/libjvm.so") == 0);

  uint64_t offset;
  assert(lookup_symbol(&offset, "testdata/libjvm.so", "_ZN6Method26checked_resolve_jmethod_idEP10_jmethodID") != 0);
  assert(offset == 0x8bc850ull);
  return 0;
}
