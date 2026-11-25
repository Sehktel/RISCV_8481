2026-11-25T14:22:14.0856141Z Runner 7511d556-129b-4ea5-af77-ec8d8584c033 (version:v13.2.1) received task 66 of job build-and-sast, triggered by event: push
2026-11-25T14:22:14.0878744Z workflow prepared
2026-11-25T14:22:14.0952236Z 🚀  Start image=riscv_os_devsecops:latest
2026-11-25T14:22:14.1051931Z   🐳  docker create image=riscv_os_devsecops:latest platform=linux/amd64 entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-25T14:22:14.1335825Z   🐳  docker run image=riscv_os_devsecops:latest platform=linux/amd64 entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-25T14:22:14.4398480Z ⭐ Run Main Checkout repository
2026-11-25T14:22:14.4479537Z   🐳  docker exec cmd=[sh /var/run/act/1071fd234c9fc1a0.sh] user= workdir=
2026-11-25T14:22:14.5080532Z   🐳  docker exec cmd=[bash --noprofile --norc -e -o pipefail /var/run/act/workflow/0.sh] user= workdir=
2026-11-25T14:22:14.5700409Z Cloning into '.'...
2026-11-25T14:22:14.9097256Z sh -c "HOME=/tmp git config --global --add safe.directory '*' && HOME=/tmp gitleaks detect --source . --config .gitleaks.toml --platform gitea --verbose --redact"
2026-11-25T14:22:15.2588169Z
2026-11-25T14:22:15.2588726Z     ○
2026-11-25T14:22:15.2588823Z     │╲
2026-11-25T14:22:15.2588929Z     │ ○
2026-11-25T14:22:15.2589036Z     ○ ░
2026-11-25T14:22:15.2589101Z     ░    gitleaks
2026-11-25T14:22:15.2589158Z
2026-11-25T14:22:15.4851992Z [90m2:22PM[0m [32mINF[0m [1m1 commits scanned.[0m
2026-11-25T14:22:15.4853293Z [90m2:22PM[0m [32mINF[0m [1mscanned ~302806 bytes (302.81 KB) in 227ms[0m
2026-11-25T14:22:15.4853624Z [90m2:22PM[0m [32mINF[0m [1mno leaks found[0m
2026-11-25T14:22:15.6414840Z sh -c "clang-format --dry-run --Werror src/main.c src/drivers/uart.c src/drivers/timer.c src/mm/heap.c src/arch/csr.c src/arch/pmp.c src/lib/console.c src/lib/string.c src/sched/sched.c src/shell/shell.c src/include/riscv.h src/include/timer.h src/include/console.h src/include/types.h src/include/string.h src/include/kernel_layout.h src/include/uart.h src/include/csr.h src/include/pmp.h src/include/sched.h src/include/mm.h src/include/shell.h src/include/heap.h"
2026-11-25T14:22:15.8718955Z cppcheck --enable=warning,style,performance,portability \
2026-11-25T14:22:15.8719801Z          --platform=unix64 \
2026-11-25T14:22:15.8719943Z          --library=.infrastructure/cppcheck/kernel.cfg \
2026-11-25T14:22:15.8720285Z          --error-exitcode=1 \
2026-11-25T14:22:15.8720397Z          --suppress=missingIncludeSystem \
2026-11-25T14:22:15.8720459Z          --suppress=unusedFunction \
2026-11-25T14:22:15.8720513Z          --inline-suppr \
2026-11-25T14:22:15.8720572Z          src/
2026-11-25T14:22:15.8821069Z Checking src/arch/csr.c ...
2026-11-25T14:22:15.8836974Z 1/10 files checked 9% done
2026-11-25T14:22:15.8837528Z Checking src/arch/pmp.c ...
2026-11-25T14:22:15.8852076Z 2/10 files checked 13% done
2026-11-25T14:22:15.8852265Z Checking src/drivers/timer.c ...
2026-11-25T14:22:15.8874681Z 3/10 files checked 15% done
2026-11-25T14:22:15.8874950Z Checking src/drivers/uart.c ...
2026-11-25T14:22:15.8888024Z 4/10 files checked 21% done
2026-11-25T14:22:15.8888629Z Checking src/lib/console.c ...
2026-11-25T14:22:15.9060567Z 5/10 files checked 30% done
2026-11-25T14:22:15.9061496Z Checking src/lib/string.c ...
2026-11-25T14:22:15.9069523Z 6/10 files checked 32% done
2026-11-25T14:22:15.9070196Z Checking src/main.c ...
2026-11-25T14:22:15.9129981Z 7/10 files checked 37% done
2026-11-25T14:22:15.9130511Z Checking src/mm/heap.c ...
2026-11-25T14:22:15.9625837Z 8/10 files checked 60% done
2026-11-25T14:22:15.9626402Z Checking src/sched/sched.c ...
2026-11-25T14:22:15.9737825Z 9/10 files checked 89% done
2026-11-25T14:22:15.9738290Z Checking src/shell/shell.c ...
2026-11-25T14:22:15.9774654Z 10/10 files checked 100% done
2026-11-25T14:22:16.1716033Z sh -c "clang-tidy src/main.c src/drivers/uart.c src/drivers/timer.c src/mm/heap.c src/arch/csr.c src/arch/pmp.c src/lib/console.c src/lib/string.c src/sched/sched.c src/shell/shell.c \
2026-11-25T14:22:16.1717980Z          --warnings-as-errors='*' \
2026-11-25T14:22:16.1718114Z          -header-filter='^/workspace/src/' \
2026-11-25T14:22:16.1718180Z          -- \
2026-11-25T14:22:16.1718298Z          --target=riscv64-unknown-elf \
2026-11-25T14:22:16.1718406Z          -march=rv64gc \
2026-11-25T14:22:16.1718559Z          -mabi=lp64d \
2026-11-25T14:22:16.1718678Z          -ffreestanding \
2026-11-25T14:22:16.1718829Z          -nostdlib \
2026-11-25T14:22:16.1718952Z          -Isrc \
2026-11-25T14:22:16.1719086Z          -Isrc/include"
2026-11-25T14:22:16.2026787Z [1/10] Processing file /workspace/SukhovAA/RISCV/src/main.c.
2026-11-25T14:22:16.2144199Z 10 warnings generated.
2026-11-25T14:22:16.2147178Z [2/10] Processing file /workspace/SukhovAA/RISCV/src/drivers/uart.c.
2026-11-25T14:22:16.2207559Z 20 warnings generated.
2026-11-25T14:22:16.2209720Z [3/10] Processing file /workspace/SukhovAA/RISCV/src/drivers/timer.c.
2026-11-25T14:22:16.2278590Z 20 warnings generated.
2026-11-25T14:22:16.2280887Z [4/10] Processing file /workspace/SukhovAA/RISCV/src/mm/heap.c.
2026-11-25T14:22:16.2432959Z 20 warnings generated.
2026-11-25T14:22:16.2435732Z [5/10] Processing file /workspace/SukhovAA/RISCV/src/arch/csr.c.
2026-11-25T14:22:16.2484008Z 20 warnings generated.
2026-11-25T14:22:16.2485631Z [6/10] Processing file /workspace/SukhovAA/RISCV/src/arch/pmp.c.
2026-11-25T14:22:16.2537705Z 20 warnings generated.
2026-11-25T14:22:16.2539637Z [7/10] Processing file /workspace/SukhovAA/RISCV/src/lib/console.c.
2026-11-25T14:22:16.2638950Z 30 warnings generated.
2026-11-25T14:22:16.2640990Z [8/10] Processing file /workspace/SukhovAA/RISCV/src/lib/string.c.
2026-11-25T14:22:16.2691496Z 30 warnings generated.
2026-11-25T14:22:16.2693282Z [9/10] Processing file /workspace/SukhovAA/RISCV/src/sched/sched.c.
2026-11-25T14:22:16.2871920Z 30 warnings generated.
2026-11-25T14:22:16.2902668Z [10/10] Processing file /workspace/SukhovAA/RISCV/src/shell/shell.c.
2026-11-25T14:22:16.2994173Z 30 warnings generated.
2026-11-25T14:22:16.2995521Z Suppressed 30 warnings (30 in non-user code).
2026-11-25T14:22:16.2995639Z Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.
2026-11-25T14:22:16.4649028Z sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"
2026-11-25T14:22:16.5365263Z -- The C compiler identification is GNU 13.2.0
2026-11-25T14:22:16.5507168Z -- The ASM compiler identification is GNU
2026-11-25T14:22:16.5525790Z -- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
2026-11-25T14:22:16.5664331Z -- Detecting C compiler ABI info
2026-11-25T14:22:16.7379208Z -- Detecting C compiler ABI info - done
2026-11-25T14:22:16.7453883Z -- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
2026-11-25T14:22:16.7457206Z -- Detecting C compile features
2026-11-25T14:22:16.7463100Z -- Detecting C compile features - done
2026-11-25T14:22:16.7590775Z -- Configuring done (0.3s)
2026-11-25T14:22:16.7747923Z -- Generating done (0.0s)
2026-11-25T14:22:16.7772498Z -- Build files have been written to: /workspace/SukhovAA/RISCV/build
2026-11-25T14:22:16.8027093Z gmake[1]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:16.8390474Z gmake[2]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:16.8405834Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:16.8565889Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:16.8591979Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:16.8729069Z [  5%] Building C object src/shell/CMakeFiles/shell.dir/shell.c.obj
2026-11-25T14:22:16.9301734Z [ 11%] Linking C static library libshell.a
2026-11-25T14:22:17.0025877Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.0185961Z [ 11%] Built target shell
2026-11-25T14:22:17.0230368Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.0415768Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.0442350Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.0592295Z [ 16%] Building C object src/arch/CMakeFiles/arch.dir/csr.c.obj
2026-11-25T14:22:17.0967994Z [ 22%] Building C object src/arch/CMakeFiles/arch.dir/pmp.c.obj
2026-11-25T14:22:17.1345305Z [ 27%] Building ASM object src/arch/CMakeFiles/arch.dir/start.S.obj
2026-11-25T14:22:17.1644697Z [ 33%] Linking C static library libarch.a
2026-11-25T14:22:17.2405625Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.2517468Z [ 33%] Built target arch
2026-11-25T14:22:17.2551680Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.2678677Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.2699663Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.2806023Z [ 38%] Building C object src/drivers/CMakeFiles/drivers.dir/uart.c.obj
2026-11-25T14:22:17.3210273Z [ 44%] Building C object src/drivers/CMakeFiles/drivers.dir/timer.c.obj
2026-11-25T14:22:17.3531784Z [ 50%] Linking C static library libdrivers.a
2026-11-25T14:22:17.4298851Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.4414281Z [ 50%] Built target drivers
2026-11-25T14:22:17.4442143Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.4569680Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.4590706Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.4699478Z [ 55%] Building C object src/lib/CMakeFiles/klib.dir/console.c.obj
2026-11-25T14:22:17.5352045Z [ 61%] Building C object src/lib/CMakeFiles/klib.dir/string.c.obj
2026-11-25T14:22:17.5708380Z [ 66%] Linking C static library libklib.a
2026-11-25T14:22:17.6480147Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.6604825Z [ 66%] Built target klib
2026-11-25T14:22:17.6640302Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.6761273Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.6778925Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.6876278Z [ 72%] Building C object src/mm/CMakeFiles/mm.dir/heap.c.obj
2026-11-25T14:22:17.7736831Z [ 77%] Linking C static library libmm.a
2026-11-25T14:22:17.8397949Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.8506453Z [ 77%] Built target mm
2026-11-25T14:22:17.8531024Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.8667789Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.8689186Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:17.8802676Z [ 83%] Building C object src/sched/CMakeFiles/sched.dir/sched.c.obj
2026-11-25T14:22:17.9786091Z [ 88%] Linking C static library libsched.a
2026-11-25T14:22:18.0592479Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.0745223Z [ 88%] Built target sched
2026-11-25T14:22:18.0772853Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.0896986Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.0919034Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.1042716Z [ 94%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
2026-11-25T14:22:18.1412122Z [100%] Linking C executable kernel.elf
2026-11-25T14:22:18.1785080Z Генерация артефактов: bin/kernel.bin и bin/kernel.elf
2026-11-25T14:22:18.2025073Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.2129929Z [100%] Built target kernel.elf
2026-11-25T14:22:18.2138604Z gmake[2]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.2243162Z gmake[1]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-25T14:22:18.2297517Z -rwxr-xr-x 1 root root 11K Nov 25 14:22 bin/kernel.bin
2026-11-25T14:22:18.2297879Z -rwxr-xr-x 1 root root 61K Nov 25 14:22 bin/kernel.elf
2026-11-25T14:22:18.4358360Z Артефакт: Packages -> kernel -> version cad0a52d7a651babd230a6627d5f158dc336820b
2026-11-25T14:22:18.6241527Z HTTP status: 200
2026-11-25T14:22:18.6386889Z Найдено версий: 11
2026-11-25T14:22:18.6388598Z Оставляем (1/10): cad0a52d7a651babd230a6627d5f158dc336820b
2026-11-25T14:22:18.6388729Z Оставляем (2/10): 74a6bf7ce9d1f5c4afda505644964fb0e493490a
2026-11-25T14:22:18.6389081Z Оставляем (3/10): 03cf952185ceca760ab1c6e508c93bff173ae526
2026-11-25T14:22:18.6389159Z Оставляем (4/10): 703ff119549ab1a9817bce5302b896f31df95050
2026-11-25T14:22:18.6389231Z Оставляем (5/10): 9b7c1c7c62e8eb0a5e68a18721d7a36b21e5223d
2026-11-25T14:22:18.6389314Z Оставляем (6/10): a5c54c0c3d2be58958b99bb697f44751f3925e7b
2026-11-25T14:22:18.6389392Z Оставляем (7/10): b65d9556f58365f0fd4a8508e03563d9b7878736
2026-11-25T14:22:18.6389477Z Оставляем (8/10): 567993f3f055317b4f206b288fdd9f0749181c17
2026-11-25T14:22:18.6389549Z Оставляем (9/10): 0d7dde077a200e24f55224c433e192523adfa893
2026-11-25T14:22:18.6389832Z Оставляем (10/10): dd2b2e23d3fe84ae21d45ece8b9eb9daf2782a1f
2026-11-25T14:22:18.6390009Z Удаляю старую версию (11): 4d43d3b682bdb444fc72686a3768eb3c0f2ac419
2026-11-25T14:22:18.6807440Z   DELETE 4d43d3b682bdb444fc72686a3768eb3c0f2ac419: 204
2026-11-25T14:22:18.9254519Z 🏁  Job succeeded
