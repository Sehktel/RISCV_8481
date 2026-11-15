.clang-tidy
```
---
Checks: >
  -*,
  bugprone-*,
  cert-*,
  -bugprone-easily-swappable-parameters,
  -bugprone-implicit-widening-of-multiplication-result,
  -bugprone-branch-clone,
  -cert-err33-c,
  -cert-msc30-c,
  -cert-msc50-cpp,
  -cert-dcl37-c,
  -cert-dcl51-cpp

WarningsAsErrors: '*'

HeaderFilterRegex: 'src/.*\.h$'

CheckOptions:
  - key: bugprone-sizeof-expression.WarnOnSizeOfPointer
    value: 'true'
  - key: bugprone-suspicious-memset-usage.WarnOnZeroLength
    value: 'true'
...
```

main.h
```c
// NOLINTBEGIN(bugprone-reserved-identifier)
extern char _heap_start[];
extern char _heap_end[];
// NOLINTEND(bugprone-reserved-identifier)

```

Makefile
```make
.PHONY: all build style sast format clean clear \
        docker-build docker-clear docker-format docker-style docker-sast \
        run debug gdb tidy docker-tidy

## Глубокий статический анализ кода (clang-tidy: bugprone-*, cert-*)
tidy:
	$(DOCKER_RUN) sh -c "clang-tidy src/*.c \
	         --warnings-as-errors='*' \
	         -- \
	         --target=riscv64-unknown-elf \
	         -march=rv64gc \
	         -mabi=lp64d \
	         -ffreestanding \
	         -nostdlib \
	         -Isrc"

docker-tidy: tidy
```

CMakeLists.txt
```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

ci.yaml
```yaml
      - name: Advanced SAST - CERT & Bugprone (Clang-Tidy)
        run: make tidy
```