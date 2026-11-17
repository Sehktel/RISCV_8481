./infrastructure/docker/Docekrfile
```Dockerfile
ARG GITLEAKS_VERSION=8.30.1
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl \
    && curl -sSfL "https://github.com/gitleaks/gitleaks/releases/download/v${GITLEAKS_VERSION}/gitleaks_${GITLEAKS_VERSION}_linux_x64.tar.gz" \
       | tar -xz -C /usr/local/bin gitleaks \
    && chmod +x /usr/local/bin/gitleaks \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
```

Makefile
```
.PHONY
		secrets-staged secrets docker-secrets \


## Аудит всей истории Git на наличие секретов (для CI)
secrets:
	$(DOCKER_RUN) sh -c "git config --global --add safe.directory /workspace && gitleaks detect --source . --verbose --redact"

## Аудит только подготовленных к коммиту изменений (для pre-commit хука)
secrets-staged:
	$(DOCKER_RUN) sh -c "git config --global --add safe.directory /workspace && gitleaks protect --staged --verbose --redact"

```

pre-commit
```bash

# 4. Аудит секретов перед коммитом (Gitleaks внутри контейнера)
echo "Запуск сканирования на утечку секретов (Gitleaks)..."
make -s secrets-staged
if [ $? -ne 0 ]; then
    echo "Критическая ошибка: Gitleaks обнаружил секрет или приватный ключ в индексе Git!"
    echo "Коммит заблокирован. Удалите секрет из файлов."
    exit 1
fi
echo "Secrets Audit: проверка пройдена."


```

.forgejo/wokrflows/ci.yaml
```yaml
    steps:
      - name: Checkout repository
        run: |
          git config --global --add safe.directory "$(pwd)"
          git clone --branch "${GITHUB_REF_NAME}" "http://oauth2:${GITHUB_TOKEN}@forgejo:3000/${GITHUB_REPOSITORY}.git" .

      - name: Secret Scanning Gate (Gitleaks)
        run: make secrets

      - name: Code Style Gate (clang-format)
        run: make style

```

.gitleaks.toml
```ini
[extend]
useDefault = true

[allowlist]
  description = "curl --user с переменными окружения CI — не секрет"
  regexTarget = "line"
  regexes = [
    '''--user\s+"[^"]*:\$\{[A-Z_]+\}"''',
    '''-u\s+"[^"]*:\$\{[A-Z_]+\}"''',
  ]
```