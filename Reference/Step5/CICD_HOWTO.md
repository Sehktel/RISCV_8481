```bash
$ docker build -t riscv_os_devsecops:latest -f .infrastructure/docker/Dockerfile .
[+] Building 63.5s (7/7) FINISHED                                                                                                                      docker:default
 => [internal] load build definition from Dockerfile                                                                                                             0.0s
 => => transferring dockerfile: 1.69kB                                                                                                                           0.0s
 => [internal] load metadata for docker.io/library/ubuntu:24.04                                                                                                  2.1s
 => [internal] load .dockerignore                                                                                                                                0.0s
 => => transferring context: 357B                                                                                                                                0.0s
 => [1/3] FROM docker.io/library/ubuntu:24.04@sha256:2198e4c65076e9cb0b3d695219df667c0e3b0072da64ca3c2648aca17dcecaf5                                            3.4s
 => => resolve docker.io/library/ubuntu:24.04@sha256:2198e4c65076e9cb0b3d695219df667c0e3b0072da64ca3c2648aca17dcecaf5                                            0.0s
 => => sha256:2198e4c65076e9cb0b3d695219df667c0e3b0072da64ca3c2648aca17dcecaf5 6.69kB / 6.69kB                                                                   0.0s
 => => sha256:77eb101c607d6e3d4f5ad0c8a02c2136c689b98aba1f359e6502ba517e5c57ec 424B / 424B                                                                       0.0s
 => => sha256:7da97f743f397971f35dd21cd2d19a7da070a150e0cfa3eda559e71891ec7533 2.05kB / 2.05kB                                                                   0.0s
 => => sha256:26538842dfbfed820e62531e047bc6cfd3359a7e0a52f59c1a951ccc6ca787fe 29.79MB / 29.79MB                                                                 1.6s
 => => extracting sha256:26538842dfbfed820e62531e047bc6cfd3359a7e0a52f59c1a951ccc6ca787fe                                                                        1.7s
 => [2/3] RUN apt-get update && apt-get install -y --no-install-recommends     build-essential     cmake     git     make     python3     python3-pip     pyth  49.7s
 => [3/3] WORKDIR /workspace                                                                                                                                     0.0s
 => exporting to image                                                                                                                                           8.2s
 => => exporting layers                                                                                                                                          8.2s
 => => writing image sha256:2a01253d44c632f46aef343ff449460b213cfd308d2facd1ce0a00011c45548d                                                                     0.0s
 => => naming to docker.io/library/riscv_os_devsecops:latest
 ```

 ```Dockerfile
     # Инструменты автогенерации документации и графов вызовов
    doxygen \
    graphviz \
 ```

 ```Makefile
 .PHONY: all clean clear run debug gdb docker-build docker-clear docs

 # Генерация HTML-документации и интерактивных графов вызовов внутри Docker
docs:
	@mkdir -p docs/generated
	docker run --rm \
		--user $$(id -u):$$(id -g) \
		-v $$(pwd):/workspace \
		riscv_os_devsecops:latest \
		doxygen docs/Doxyfile
	@echo "Документация и Call-графы сгенерированы в папке: docs/generated/html/index.html"

 ```