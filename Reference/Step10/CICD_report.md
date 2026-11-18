```bash
$ docker-compose up -d
[+] Running 18/18
 ✔ mattermost-db Pulled                                                                                              22.0s
   ✔ 881fc932939e Already exists                                                                                      0.0s
   ✔ ea98517a315c Pull complete                                                                                       3.6s
   ✔ 41713cbe7496 Pull complete                                                                                       4.5s
   ✔ 5788ee6f91dc Pull complete                                                                                       4.2s
   ✔ f997efa53d88 Pull complete                                                                                       4.7s
   ✔ a2d17f5b24d5 Pull complete                                                                                      16.7s
   ✔ f85a33377d83 Pull complete                                                                                       5.1s
   ✔ 86cc7190d1c0 Pull complete                                                                                       5.5s
   ✔ 5959c7ea857a Pull complete                                                                                       6.1s
   ✔ 6bceb0db2fd1 Pull complete                                                                                       6.5s
   ✔ b7106f8586ee Pull complete                                                                                       6.9s
 ✔ mattermost Pulled                                                                                                 18.6s
   ✔ 9c704ecd0c69 Pull complete                                                                                       2.7s
   ✔ 128899e696b8 Pull complete                                                                                       3.1s
   ✔ 86d071fddf50 Pull complete                                                                                      10.6s
   ✔ b7a18ea578ef Pull complete                                                                                       3.1s
   ✔ 4f4fb700ef54 Pull complete                                                                                       3.5s
[+] Running 7/7
 ✔ Volume "mattermost_mm_db_data"  Created                                                                            0.0s
 ✔ Volume "mattermost_mm_config"   Created                                                                            0.0s
 ✔ Volume "mattermost_mm_data"     Created                                                                            0.0s
 ✔ Volume "mattermost_mm_logs"     Created                                                                            0.0s
 ✔ Volume "mattermost_mm_plugins"  Created                                                                            0.0s
 ✔ Container devsecops_mm_db       Started                                                                            0.1s
 ✔ Container devsecops_mattermost  Started
```


```
forgejo_devsecops_slackbot
BOT
14:18
[SukhovAA/RISCV:main] 1 new commit pushed by SukhovAA

http://localhost:3000/SukhovAA/RISCV
703ff11: update ci - Andrey Sukhov


RISC-V DevSecOps Bot
BOT
14:22
:white_check_mark: Сборка и все гейты безопасности пройдены!
Параметр	Значение
Репозиторий	SukhovAA/RISCV
Ветка / Коммит	main (703ff119549ab1a9817bce5302b896f31df95050)
Инициатор	@sukhovaa
Пайплайн	Открыть в Forgejo
```

```bash
2026-11-18T11:13:56.5908009Z Runner 7511d556-129b-4ea5-af77-ec8d8584c033 (version:v13.2.1) received task 53 of job build-and-sast, triggered by event: push
2026-11-18T11:13:56.5938731Z workflow prepared
2026-11-18T11:13:56.6095553Z 🚀  Start image=riscv_os_devsecops:latest
2026-11-18T11:13:56.6239029Z   🐳  docker create image=riscv_os_devsecops:latest platform=linux/amd64 entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-18T11:13:56.6687359Z   🐳  docker run image=riscv_os_devsecops:latest platform=linux/amd64 entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-18T11:13:57.0587701Z ⭐ Run Main Checkout repository
2026-11-18T11:13:57.0659323Z   🐳  docker exec cmd=[sh /var/run/act/073818475d1495a4.sh] user= workdir=
2026-11-18T11:13:57.1389475Z   🐳  docker exec cmd=[bash --noprofile --norc -e -o pipefail /var/run/act/workflow/0.sh] user= workdir=
2026-11-18T11:13:57.2006987Z Cloning into '.'...
2026-11-18T11:13:57.4630052Z sh -c "HOME=/tmp git config --global --add safe.directory '*' && HOME=/tmp gitleaks detect --source . --config .gitleaks.toml --platform gitea --verbose --redact"
2026-11-18T11:13:57.7593523Z
2026-11-18T11:13:57.7594045Z     ○
2026-11-18T11:13:57.7594135Z     │╲
2026-11-18T11:13:57.7594542Z     │ ○
2026-11-18T11:13:57.7594611Z     ○ ░
2026-11-18T11:13:57.7594672Z     ░    gitleaks
2026-11-18T11:13:57.7594728Z
2026-11-18T11:13:57.9707206Z [90m11:13AM[0m [32mINF[0m [1m1 commits scanned.[0m
2026-11-18T11:13:57.9707911Z [90m11:13AM[0m [32mINF[0m [1mscanned ~263959 bytes (263.96 KB) in 211ms[0m
2026-11-18T11:13:57.9731903Z [90m11:13AM[0m [32mINF[0m [1mno leaks found[0m
2026-11-18T11:13:58.1498070Z sh -c "find src/ -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror"
2026-11-18T11:13:58.3886457Z cppcheck --enable=warning,style,performance,portability \
2026-11-18T11:13:58.3887886Z          --platform=unix64 \
2026-11-18T11:13:58.3888029Z          --library=.infrastructure/cppcheck/kernel.cfg \
2026-11-18T11:13:58.3888096Z          --error-exitcode=1 \
2026-11-18T11:13:58.3888420Z          --suppress=missingIncludeSystem \
2026-11-18T11:13:58.3888556Z          --suppress=unusedFunction \
2026-11-18T11:13:58.3888655Z          --inline-suppr \
2026-11-18T11:13:58.3888744Z          src/
2026-11-18T11:13:58.4059639Z Checking src/main.c ...
2026-11-18T11:13:58.7763975Z sh -c "clang-tidy src/*.c \
2026-11-18T11:13:58.7765096Z          --warnings-as-errors='*' \
2026-11-18T11:13:58.7765418Z          -- \
2026-11-18T11:13:58.7765521Z          --target=riscv64-unknown-elf \
2026-11-18T11:13:58.7765652Z          -march=rv64gc \
2026-11-18T11:13:58.7765734Z          -mabi=lp64d \
2026-11-18T11:13:58.7765852Z          -ffreestanding \
2026-11-18T11:13:58.7765907Z          -nostdlib \
2026-11-18T11:13:58.7765957Z          -Isrc"
2026-11-18T11:13:58.8621963Z 10 warnings generated.
2026-11-18T11:13:58.8625309Z Suppressed 14 warnings (10 in non-user code, 4 NOLINT).
2026-11-18T11:13:58.8625746Z Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.
2026-11-18T11:13:59.0288527Z sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"
2026-11-18T11:13:59.1152614Z -- The C compiler identification is GNU 13.2.0
2026-11-18T11:13:59.1315734Z -- The ASM compiler identification is GNU
2026-11-18T11:13:59.1335534Z -- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
2026-11-18T11:13:59.1453335Z -- Detecting C compiler ABI info
2026-11-18T11:13:59.2810475Z -- Detecting C compiler ABI info - done
2026-11-18T11:13:59.2857738Z -- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
2026-11-18T11:13:59.2859289Z -- Detecting C compile features
2026-11-18T11:13:59.2865741Z -- Detecting C compile features - done
2026-11-18T11:13:59.2978699Z -- Configuring done (0.3s)
2026-11-18T11:13:59.3006942Z -- Generating done (0.0s)
2026-11-18T11:13:59.3015595Z -- Build files have been written to: /workspace/SukhovAA/RISCV/build
2026-11-18T11:13:59.3173959Z gmake[1]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.3398679Z gmake[2]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.3411712Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.3521387Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.3539795Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.3646951Z [ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
2026-11-18T11:13:59.4295520Z [ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
2026-11-18T11:13:59.4556453Z [100%] Linking C executable /workspace/SukhovAA/RISCV/bin/kernel.elf
2026-11-18T11:13:59.4884011Z Генерация сырого бинарника: bin/kernel.bin
2026-11-18T11:13:59.4905420Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.5007978Z [100%] Built target kernel.elf
2026-11-18T11:13:59.5023325Z gmake[2]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.5126733Z gmake[1]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-18T11:13:59.5170895Z -rwxr-xr-x 1 root root 13K Nov 18 11:13 bin/kernel.bin
2026-11-18T11:13:59.5171230Z -rwxr-xr-x 1 root root 37K Nov 18 11:13 bin/kernel.elf
2026-11-18T11:13:59.7449163Z Артефакт: Packages -> kernel -> version 9b7c1c7c62e8eb0a5e68a18721d7a36b21e5223d
2026-11-18T11:13:59.9206045Z HTTP status: 200
2026-11-18T11:13:59.9270657Z Найдено версий: 10
2026-11-18T11:13:59.9271092Z Оставляем (1/10): 9b7c1c7c62e8eb0a5e68a18721d7a36b21e5223d
2026-11-18T11:13:59.9271201Z Оставляем (2/10): a5c54c0c3d2be58958b99bb697f44751f3925e7b
2026-11-18T11:13:59.9271270Z Оставляем (3/10): b65d9556f58365f0fd4a8508e03563d9b7878736
2026-11-18T11:13:59.9271337Z Оставляем (4/10): 567993f3f055317b4f206b288fdd9f0749181c17
2026-11-18T11:13:59.9271398Z Оставляем (5/10): 0d7dde077a200e24f55224c433e192523adfa893
2026-11-18T11:13:59.9271465Z Оставляем (6/10): dd2b2e23d3fe84ae21d45ece8b9eb9daf2782a1f
2026-11-18T11:13:59.9271522Z Оставляем (7/10): 4d43d3b682bdb444fc72686a3768eb3c0f2ac419
2026-11-18T11:13:59.9271601Z Оставляем (8/10): 7fe44e71bf12cb04b47f24c00faaf5ab07ebd96f
2026-11-18T11:13:59.9271654Z Оставляем (9/10): bc66b3b8d8e37d919dd0a82523cf99a9a59f6dda
2026-11-18T11:13:59.9271707Z Оставляем (10/10): 6a1f1164c6655b88eb5fbe2088ab1ede4b450793
2026-11-18T11:14:00.1156122Z ⚙️ [runner]: exitcode '3': failure
2026-11-18T11:14:00.2779037Z ⚙️ [runner]: exitcode '3': failure
2026-11-18T11:14:00.2780790Z 🏁  Job failed
2026-11-18T11:14:00.4665841Z Job 'build-and-sast' failed
```