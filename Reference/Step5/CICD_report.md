```bash
$ make docs
docker run --rm \
        --user $(id -u):$(id -g) \
        -v $(pwd):/workspace \
        riscv_os_devsecops:latest \
        doxygen docs/Doxyfile
warning: The selected output language "russian" has not been updated
since release 1.8.15.  As a result some sentences may appear in English.

Adding custom extension mapping: 's' will be treated as language 'c'
Adding custom extension mapping: 's' will be treated as language 'c'
Doxygen version used: 1.9.8
Searching for include files...
...
Running plantuml with JAVA...
Running dot...
Patching output file 1/1
type lookup cache used 27/65536 hits=45 misses=27
symbol lookup cache used 54/65536 hits=124 misses=54
finished...
Документация и Call-графы сгенерированы в папке: docs/generated/html/index.html
```

```bash
$ tree -h ./docs/generated/html/
[ 12K]  ./docs/generated/html/
├── [ 635]  bc_sd.png
├── [ 676]  bc_s.png
├── [ 132]  closed.png
├── [4.0K]  dir_68267d1309a1af8e8297ef4c3efbcdba.html
├── [1.5K]  docd.svg
├── [1.5K]  doc.svg
├── [ 45K]  doxygen.css
├── [ 15K]  doxygen.svg
├── [7.9K]  dynsections.js
├── [4.2K]  files.html


```