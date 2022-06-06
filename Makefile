.PHONY: test

CFLAGS := -Wall -Wextra -Wpointer-arith

test: bin/dlhiddensym_test
	$<

.PHONY: fmt
fmt:
	clang-format -i dlhiddensym.h

.PHONY: clean
clean:
	rm -rf ./bin

bin/dlhiddensym_test: dlhiddensym_test.c dlhiddensym.h
	@mkdir -p ./bin
	$(CC) $(CFLAGS) -o $@ $<

