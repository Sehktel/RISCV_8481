```bash
2026-11-15T16:16:20.5596220Z Runner 7511d556-129b-4ea5-af77-ec8d8584c033 (version:v13.2.1) received task 38 of job build-and-sast, triggered by event: push
2026-11-15T16:16:20.5617318Z workflow prepared
2026-11-15T16:16:20.5696143Z 🚀  Start image=riscv_os_devsecops:latest
2026-11-15T16:16:20.5783116Z   🐳  docker create image=riscv_os_devsecops:latest platform=linux/amd64 entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-15T16:16:20.6128042Z   🐳  docker run image=riscv_os_devsecops:latest platform=linux/amd64 entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-15T16:16:20.9720063Z ⭐ Run Main Checkout repository
2026-11-15T16:16:20.9783357Z   🐳  docker exec cmd=[sh /var/run/act/4f78fdfa86c21861.sh] user= workdir=
2026-11-15T16:16:21.0443618Z   🐳  docker exec cmd=[bash --noprofile --norc -e -o pipefail /var/run/act/workflow/0.sh] user= workdir=
2026-11-15T16:16:21.1104951Z Cloning into '.'...
2026-11-15T16:16:21.3137848Z sh -c "find src/ -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror"
2026-11-15T16:16:21.4966869Z cppcheck --enable=warning,style,performance,portability \
2026-11-15T16:16:21.4967731Z          --platform=unix64 \
2026-11-15T16:16:21.4968159Z          --library=.infrastructure/cppcheck/kernel.cfg \
2026-11-15T16:16:21.4968226Z          --error-exitcode=1 \
2026-11-15T16:16:21.4968290Z          --suppress=missingIncludeSystem \
2026-11-15T16:16:21.4968438Z          --suppress=unusedFunction \
2026-11-15T16:16:21.4968489Z          --inline-suppr \
2026-11-15T16:16:21.4968591Z          src/
2026-11-15T16:16:21.5091377Z Checking src/main.c ...
2026-11-15T16:16:21.7510696Z sh -c "clang-tidy src/*.c \
2026-11-15T16:16:21.7511525Z          --warnings-as-errors='*' \
2026-11-15T16:16:21.7511855Z          -- \
2026-11-15T16:16:21.7511939Z          --target=riscv64-unknown-elf \
2026-11-15T16:16:21.7512012Z          -march=rv64gc \
2026-11-15T16:16:21.7512099Z          -mabi=lp64d \
2026-11-15T16:16:21.7512170Z          -ffreestanding \
2026-11-15T16:16:21.7512239Z          -nostdlib \
2026-11-15T16:16:21.7512306Z          -Isrc"
2026-11-15T16:16:21.8115385Z 10 warnings generated.
2026-11-15T16:16:21.8118815Z Suppressed 12 warnings (10 in non-user code, 2 NOLINT).
2026-11-15T16:16:21.8119000Z Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.
2026-11-15T16:16:21.9870647Z sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"
2026-11-15T16:16:22.0947035Z -- The C compiler identification is GNU 13.2.0
2026-11-15T16:16:22.1127097Z -- The ASM compiler identification is GNU
2026-11-15T16:16:22.1146465Z -- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
2026-11-15T16:16:22.1304765Z -- Detecting C compiler ABI info
2026-11-15T16:16:22.2948178Z -- Detecting C compiler ABI info - done
2026-11-15T16:16:22.3024809Z -- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
2026-11-15T16:16:22.3032270Z -- Detecting C compile features
2026-11-15T16:16:22.3042179Z -- Detecting C compile features - done
2026-11-15T16:16:22.3149849Z -- Configuring done (0.3s)
2026-11-15T16:16:22.3184102Z -- Generating done (0.0s)
2026-11-15T16:16:22.3191917Z -- Build files have been written to: /workspace/SukhovAA/RISCV/build
2026-11-15T16:16:22.3381985Z gmake[1]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.3681962Z gmake[2]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.3696770Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.3849222Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.3873226Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.3996754Z [ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
2026-11-15T16:16:22.4733109Z [ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
2026-11-15T16:16:22.4984675Z [100%] Linking C executable /workspace/SukhovAA/RISCV/bin/kernel.elf
2026-11-15T16:16:22.5344865Z Генерация сырого бинарника: bin/kernel.bin
2026-11-15T16:16:22.5375019Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.5491905Z [100%] Built target kernel.elf
2026-11-15T16:16:22.5507015Z gmake[2]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.5633839Z gmake[1]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T16:16:22.5686668Z -rwxr-xr-x 1 root root 9.0K Nov 15 16:16 bin/kernel.bin
2026-11-15T16:16:22.5687135Z -rwxr-xr-x 1 root root  29K Nov 15 16:16 bin/kernel.elf
2026-11-15T16:16:22.8330710Z Артефакт: Packages -> kernel -> version 7bc963f76705219384de8ceb8e604e259a5064fb
2026-11-15T16:16:22.9949585Z HTTP status: 200
2026-11-15T16:16:23.0009474Z Найдено версий: 11
2026-11-15T16:16:23.0011922Z Оставляем (1/10): 7bc963f76705219384de8ceb8e604e259a5064fb
2026-11-15T16:16:23.0012341Z Оставляем (2/10): d2178a0af3213c07102f98903f3da97d852275de
2026-11-15T16:16:23.0012451Z Оставляем (3/10): 6bb5b0c4b19b476aca9e1bb4dd6a1057d286e288
2026-11-15T16:16:23.0012516Z Оставляем (4/10): 1d1740efc08c65631b91f9685e3121a15e82db8a
2026-11-15T16:16:23.0012594Z Оставляем (5/10): a057008240cb78c5053e01dd1661776ab4ce9092
2026-11-15T16:16:23.0012677Z Оставляем (6/10): f98af45717c06cb8964a5d0ab79a2ccbff653218
2026-11-15T16:16:23.0012735Z Оставляем (7/10): df19de3277bacd78fa3b05d8c7c931316264321e
2026-11-15T16:16:23.0012813Z Оставляем (8/10): b013a572a1176b872a65f4638126bdf63e30f6b3
2026-11-15T16:16:23.0012871Z Оставляем (9/10): 1e9f193822b4c5c0777d6a34abee7e479aff4491
2026-11-15T16:16:23.0012943Z Оставляем (10/10): a9fd5419a63a3a4faf52d28626f6c215f9a34642
2026-11-15T16:16:23.0013002Z Удаляю старую версию (11): af53326e537d74e574af0e6ff9e3799fa5df05b0
2026-11-15T16:16:23.0489531Z   DELETE af53326e537d74e574af0e6ff9e3799fa5df05b0: 204
2026-11-15T16:16:23.0652088Z 🏁  Job succeeded
```