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

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  char *begin;
  char *end;
  char *perms;
  char *offset;
  char *dev;
  char *inode;
  char *pathname;
  char _buf[8192];
} memory_map;

int lookup_memory_map(memory_map *dst, const char *maps, const char *filename) {
  FILE *f = fopen(maps, "r");
  if (f == NULL) {
    return 1;
  }
  memset(dst->_buf, 0, sizeof(dst->_buf));
  int filename_len = strlen(filename);
  int found = 0;
  while (fgets(dst->_buf, sizeof(dst->_buf) - 1, f) != NULL) {
    int n = strlen(dst->_buf);
    if (n > 0 && dst->_buf[n - 1] == '\n') {
      dst->_buf[n - 1] = '\0';
      n--;
    }
    if (n < filename_len || strcmp(dst->_buf + n - filename_len, filename) != 0) {
      continue;
    }
    // begin-end perms offset dev inode[ pathname]
    dst->begin = dst->_buf;
    dst->end = strchr(dst->begin, '-') + 1;
    dst->perms = strchr(dst->end, ' ') + 1;
    dst->offset = strchr(dst->perms, ' ') + 1;
    dst->dev = strchr(dst->offset, ' ') + 1;
    dst->inode = strchr(dst->dev, ' ') + 1;
    dst->pathname = strchr(dst->inode, ' ');
    if (dst->pathname != NULL) {
      while (*dst->pathname == ' ')
        dst->pathname++;
    }
    if (!(dst->perms[0] == 'r' && dst->perms[2] == 'x')) {
      continue;
    }
    found = 1;
    break;
  }
  fclose(f);
  return found;
}

int lookup_symbol(uint64_t *dst, const char *pathname, const char *symbol) {
  typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
  } Elf64_Shdr;
  const int SHT_SYMTAB = 2;
  const int STT_FUNC = 2;

  int fd = open(pathname, O_RDONLY);
  if (fd == -1) {
    return 0;
  }
  off_t length = lseek(fd, 0, SEEK_END);
  char *addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  if (addr == MAP_FAILED) {
    return 0;
  }
  struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
  } *ehdr = (void *)addr;
  char *shtable = addr + ehdr->e_shoff;
  Elf64_Shdr *shstrh = (Elf64_Shdr *)(shtable + (ehdr->e_shstrndx * ehdr->e_shentsize));
  char *shstr = addr + shstrh->sh_offset;
  Elf64_Shdr *symtabh = NULL;
  for (int i = 0; i < ehdr->e_shnum; i++) {
    Elf64_Shdr *h = (Elf64_Shdr *)(shtable + (i * ehdr->e_shentsize));
    if (h->sh_type == SHT_SYMTAB && strcmp(shstr + h->sh_name, ".symtab") == 0) {
      symtabh = h;
      break;
    }
  }
  int found = 0;
  if (symtabh != NULL) {
    Elf64_Shdr *strtabh = (Elf64_Shdr *)(shtable + (symtabh->sh_link * ehdr->e_shentsize));
    char *strtab = addr + strtabh->sh_offset;
    char *symtab = addr + symtabh->sh_offset;
    for (uint64_t sym_offset = 0; sym_offset < symtabh->sh_size; sym_offset += symtabh->sh_entsize) {
      struct {
        uint32_t st_name;
        uint8_t st_info;
        uint8_t st_other;
        uint16_t st_shndx;
        uint64_t st_value;
        uint64_t st_size;
      } *sym = (void *)(symtab + sym_offset);
      if (sym->st_info == STT_FUNC && strcmp(strtab + sym->st_name, symbol) == 0) {
        *dst = sym->st_value;
        found = 1;
        break;
      }
    }
  }
  munmap(addr, length);
  return found;
}

void *dlhiddensym(const char *filename, const char *symbol) {
  memory_map m;
  if (lookup_memory_map(&m, "/proc/self/maps", filename) == 0) {
    return NULL;
  }
  uint64_t offset;
  if (lookup_symbol(&offset, m.pathname, symbol) == 0) {
    return NULL;
  }
  return (char *)strtoull(m.begin, NULL, 16) + (offset - strtoull(m.offset, NULL, 16));
}
