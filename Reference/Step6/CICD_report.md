```bash
$ docker build -t riscv_os_devsecops:latest -f .infrastructure/docker/Dockerfile .
[+] Building 68.1s (7/7) FINISHED                                                           docker:default
 => [internal] load build definition from Dockerfile                                                  0.0s
 => => transferring dockerfile: 1.77kB                                                                0.0s
 => [internal] load metadata for docker.io/library/ubuntu:24.04                                       0.7s
 => [internal] load .dockerignore                                                                     0.0s
 => => transferring context: 357B                                                                     0.0s
 => CACHED [1/3] FROM docker.io/library/ubuntu:24.04@sha256:2198e4c65076e9cb0b3d695219df667c0e3b0072  0.0s
 => [2/3] RUN apt-get update && apt-get install -y --no-install-recommends     build-essential       55.9s
 => [3/3] WORKDIR /workspace                                                                          0.0s
 => exporting to image                                                                               11.4s
 => => exporting layers                                                                              11.4s
 => => writing image sha256:3d7075647b88165dfd3dcb17f8003c4c4b53aa87c56eddeb4ea5d827720b9b1e          0.0s
 => => naming to docker.io/library/riscv_os_devsecops:latest                                          0.0s
$
```

```c
        // Обработка клавиши Backspace (ASCII 8 или 127)
        else if (c == 8 || c == 127) {
                                            if (index > 0) {
                index--;
                print("\b \b"); // Сдвигаем курсор назад, стираем пробелом, возвращаем курсор
            }
```
```bash
$ make format
docker run --rm \
        --user $(id -u):$(id -g) \
        -v $(pwd):/workspace \
        riscv_os_devsecops:latest \
        sh -c "find src/ -name '*.c' -o -name '*.h' | xargs clang-format -i"
Код в папке src/ успешно отформатирован по стандарту проекта.
```

```c
        // Обработка клавиши Backspace (ASCII 8 или 127)
        else if (c == 8 || c == 127) {
            if (index > 0) {
                index--;
                print("\b \b"); // Сдвигаем курсор назад, стираем пробелом, возвращаем курсор
            }
```

```bash
$ .git/hooks/pre-commit
=========================================================
    DevSecOps: Запуск pre-commit проверок безопасности...
=========================================================
Очистка trailing-пробелов в измененных файлах...
Файлы очищены от мусорных пробелов.
Проверка стиля кода (clang-format)...
src/main.c:460:39: error: code should be clang-formatted [-Wclang-format-violations]
        else if (c == 8 || c == 127) {
                                      ^
Ошибка стиля кода!
Выполните 'make format' для автоматического исправления и повторите коммит.
$ make format
docker run --rm \
        --user $(id -u):$(id -g) \
        -v $(pwd):/workspace \
        riscv_os_devsecops:latest \
        sh -c "find src/ -name '*.c' -o -name '*.h' | xargs clang-format -i"
Код в папке src/ успешно отформатирован по стандарту проекта.
$ .git/hooks/pre-commit
=========================================================
    DevSecOps: Запуск pre-commit проверок безопасности...
=========================================================
Очистка trailing-пробелов в измененных файлах...
Файлы очищены от мусорных пробелов.
Проверка стиля кода (clang-format)...
Запуск SAST-анализа кода через Cppcheck...
Checking src/main.c ...
Все проверки успешно пройдены! Коммит разрешен.
$
```