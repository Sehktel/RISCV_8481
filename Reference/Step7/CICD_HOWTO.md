.infrastructure/cppcheck/kernel.cfg
```xml
<?xml version="1.0"?>
<def format="2">
  <memory>
    <dealloc>kfree</dealloc>
    <alloc init="false" buffer-size="malloc">kmalloc</alloc>
  </memory>

  <function name="kmalloc">
    <noreturn>false</noreturn>
    <use-retval/>
    <arg nr="1">
      <not-uninit/>
      <valid>1:</valid>
    </arg>
  </function>

  <function name="kfree">
    <noreturn>false</noreturn>
    <arg nr="1">
      <not-uninit/>
    </arg>
  </function>
</def>
```

.forgejo/workflows/ci.yaml
```yaml
name: RISC-V Microkernel CI (Stage 7)

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]

jobs:
  build-and-sast:
    runs-on: ubuntu-latest
    container:
      image: riscv_os_devsecops:latest

    steps:
      - name: Checkout repository
        run: |
          git config --global --add safe.directory "$(pwd)"
          git clone --depth 1 --branch "${GITHUB_REF_NAME}" "http://oauth2:${GITHUB_TOKEN}@forgejo:3000/${GITHUB_REPOSITORY}.git" .

      - name: Code Style Gate (clang-format)
        run: make style

      - name: First-Line SAST & Memory Leak Audit (Cppcheck)
        run: make sast

      - name: Cross-Compile Kernel (RV64GC)
        run: |
          make build
          ls -lh bin/kernel.elf bin/kernel.bin

```

.infrastructure/forgejo/docker-compose.yaml
```yaml
services:
  forgejo:
    image: codeberg.org/forgejo/forgejo:15
    container_name: devsecops_forgejo
    restart: unless-stopped
    environment:
      - USER_UID=1000
      - USER_GID=1000
      - FORGEJO__database__DB_TYPE=sqlite3
      - FORGEJO__server__DOMAIN=localhost
      - FORGEJO__server__SSH_DOMAIN=localhost
      - FORGEJO__server__ROOT_URL=http://localhost:3000/
      - FORGEJO__server__HTTP_PORT=3000
      - FORGEJO__actions__ENABLED=true
    ports:
      - "3000:3000"
      - "2222:22"
    volumes:
      - forgejo_data:/data
      - /etc/timezone:/etc/timezone:ro
      - /etc/localtime:/etc/localtime:ro
    networks:
      - devsecops_net

  forgejo-runner:
    image: code.forgejo.org/forgejo/runner:13
    container_name: devsecops_runner
    restart: unless-stopped
    depends_on:
      - forgejo
    user: root
    environment:
      - CONFIG_FILE=/config.yaml
      - FORGEJO_RUNNER_TOKEN=${FORGEJO_RUNNER_TOKEN}
    volumes:
      - ./runner-config.yaml:/config.yaml:ro
      - runner_data:/data
      - /var/run/docker.sock:/var/run/docker.sock
    working_dir: /data
    command: >
      sh -c 'if [ ! -f /data/.runner ]; then forgejo-runner register --no-interactive --instance http://forgejo:3000 --token "$FORGEJO_RUNNER_TOKEN" --name riscv-local-runner --labels ubuntu-latest:docker://riscv_os_devsecops:latest; fi && forgejo-runner daemon --config /config.yaml'
    networks:
      - devsecops_net

volumes:
  forgejo_data:
  runner_data:

networks:
  devsecops_net:
    name: devsecops_net
    driver: bridge

```

.infrastructure/forgejo/runner-config.yaml
```yaml
log:
  level: info

runner:
  file: .runner
  capacity: 1
  timeout: 3h
  insecure: true

container:
  network: "devsecops_net"
  force_pull: false
  options: --volumes-from devsecops_runner

```


Зарегестрироваться в Forgejo,
Зарегестрировать Runner
FORGEJO_RUNNER_TOKEN=<TOKEN> docker-compose up -d forgejo-runner
Создать ssh ключи, перевести репо в ssh режим
Зарегестрировать 2222 порт по умолчанию


```bash
$ cat << 'EOF' >> ~/.ssh/config
Host forgejo-local
    HostName 127.0.0.1
    Port 2222
    User git
    IdentityFile ~/.ssh/id_ed25519
    IdentitiesOnly yes
EOF
```

pre-commit hook
```bash
# 3. Запуск статического анализа Cppcheck внутри нашего Docker-контейнера
echo "Запуск SAST-анализа кода через Cppcheck..."

if [ -d "src" ]; then
    docker run --rm \
        --user $(id -u):$(id -g) \
        -v "$REPO_ROOT":/workspace \
        riscv_os_devsecops:latest \
        make sast

    if [ $? -ne 0 ]; then
        echo "Критическая ошибка: Cppcheck нашел баги или утечки памяти в коде!"
        echo "Коммит заблокирован. Исправьте предупреждения выше."
        exit 1
    fi
fi
```

Makefile
```Makefile

.PHONY: all build style sast clean clear run debug gdb docs format docker-build docker-clear

all: docker-clear docker-build run

# ==============================================================================
# 1. ЕДИНЫЕ АТОМАРНЫЕ ЦЕЛИ (выполняются внутри контейнера riscv_os_devsecops)
# ==============================================================================

build:
	@mkdir -p build bin
	cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake .
	cmake --build build

style:
	find src/ -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror

sast:
	cppcheck --enable=warning,style,performance,portability \
	         --platform=unix64 \
	         --library=.infrastructure/cppcheck/kernel.cfg \
	         --error-exitcode=1 \
	         --suppress=missingIncludeSystem \
	         --suppress=unusedFunction \
	         --inline-suppr \
	         src/

format:
	find src/ -name '*.c' -o -name '*.h' | xargs clang-format -i

clean:
	rm -rf build bin/*

clear: clean

# ==============================================================================
# 2. DOCKER-ПРОКСИ ДЛЯ ХОСТ-МАШИНЫ РАЗРАБОТЧИКА
# ==============================================================================

DOCKER_RUN = docker run --rm --user $$(id -u):$$(id -g) -v $$(pwd):/workspace riscv_os_devsecops:latest

docker-build:
	$(DOCKER_RUN) make build

docker-clear:
	$(DOCKER_RUN) make clean

docs:
	@mkdir -p docs/generated
	$(DOCKER_RUN) doxygen docs/Doxyfile
	@echo "Документация и Call-графы сгенерированы в папке: docs/generated/html/index.html"

# ==============================================================================
# 3. ЭМУЛЯЦИЯ И ОТЛАДКА НА ХОСТЕ (QEMU / GDB)
# ==============================================================================

run:
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

debug:
	@echo "========================================================="
	@echo "QEMU ожидает GDB на порту 1234. Откройте новый терминал и выполните: make gdb"
	@echo "========================================================="
	qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic -s -S

gdb:
	gdb-multiarch -ex "target remote localhost:1234" bin/kernel.elf

```