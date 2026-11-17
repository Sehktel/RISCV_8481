```
$ docker build -t riscv_os_devsecops:latest -f .infrastructure/docker/Dockerfile .
[+] Building 16.8s (8/8) FINISHED                                                                         docker:default
 => [internal] load build definition from Dockerfile                                                                0.0s
 => => transferring dockerfile: 2.17kB                                                                              0.0s
 => [internal] load metadata for docker.io/library/ubuntu:24.04                                                     0.6s
 => [internal] load .dockerignore                                                                                   0.0s
 => => transferring context: 357B                                                                                   0.0s
 => [1/4] FROM docker.io/library/ubuntu:24.04@sha256:73b999393d07f2bf65a40373778a44dc9a6d54ad7718d29247d8ff9413c4d  0.0s
 => => resolve docker.io/library/ubuntu:24.04@sha256:73b999393d07f2bf65a40373778a44dc9a6d54ad7718d29247d8ff9413c4d  0.0s
 => CACHED [2/4] RUN apt-get update && apt-get install -y --no-install-recommends     build-essential     cmake     0.0s
 => [3/4] RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl     && curl -sSfL   6.5s
 => [4/4] WORKDIR /workspace                                                                                        0.1s
 => exporting to image                                                                                              9.4s
 => => exporting layers                                                                                             9.4s
 => => writing image sha256:f17f0c307a83091feeeecabf4b499721f235ceda6e5dc1ae66fb42771a8f39a0                        0.0s
 => => naming to docker.io/library/riscv_os_devsecops:latest                                                        0.0s


$ docker run --rm riscv_os_devsecops:latest ls -l /usr/local/bin/gitleaks
-rwxr-xr-x 1 501 staff 21958840 Mar 21  2026 /usr/local/bin/gitleaks
```

```bash
$ git config --global --add safe.directory /workspace
$ git config --get-all safe.directory
/workspace

```