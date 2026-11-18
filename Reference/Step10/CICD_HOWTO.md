.infrastructure/mattermost/

.infrastructure/mattermost/docker-compose.yml (используем лёгкую связку postgres:16-alpine + mattermost/mattermost-team-edition:10.5 в общей сети devsecops_net):

```yaml
services:
  mattermost-db:
    image: postgres:16-alpine
    container_name: devsecops_mm_db
    restart: unless-stopped
    environment:
      - POSTGRES_USER=${POSTGRES_USER}
      - POSTGRES_PASSWORD=${POSTGRES_PASSWORD}
      - POSTGRES_DB=${POSTGRES_DB}
    volumes:
      - mm_db_data:/var/lib/postgresql/data
    networks:
      - devsecops_net

  mattermost:
    image: mattermost/mattermost-team-edition:10.5
    container_name: devsecops_mattermost
    restart: unless-stopped
    depends_on:
      - mattermost-db
    environment:
      - MM_SQLSETTINGS_DRIVERNAME=postgres
      - MM_SQLSETTINGS_DATASOURCE=postgres://${POSTGRES_USER}:${POSTGRES_PASSWORD}@mattermost-db:5432/${POSTGRES_DB}?sslmode=disable&connect_timeout=10
      - MM_SERVICESETTINGS_SITEURL=http://localhost:8065
      - MM_SERVICESETTINGS_ENABLEINCOMINGWEBHOOKS=true
      - MM_SERVICESETTINGS_ENABLEPOSTUSERNAMEOVERRIDE=true
      - MM_SERVICESETTINGS_ENABLEPOSTICONOVERRIDE=true
    ports:
      - "127.0.0.1:8065:8065"
    volumes:
      - mm_config:/mattermost/config
      - mm_data:/mattermost/data
      - mm_logs:/mattermost/logs
      - mm_plugins:/mattermost/plugins
    networks:
      - devsecops_net

volumes:
  mm_db_data:
  mm_config:
  mm_data:
  mm_logs:
  mm_plugins:

networks:
  devsecops_net:
    external: true

```


.infrastructure/mattermost/.env
```
POSTGRES_USER=mmuser
POSTGRES_PASSWORD=mmuser_password
POSTGRES_DB=mattermost
```

mailpit
```yaml
services:
  mailpit:
    image: axllent/mailpit:v1.22
    container_name: devsecops_mailpit
    restart: unless-stopped
    environment:
      - MP_MAX_MESSAGES=${MP_MAX_MESSAGES}
    ports:
      - "127.0.0.1:${MP_WEB_PORT}:8025"
      - "127.0.0.1:${MP_SMTP_PORT}:1025"
    networks:
      - devsecops_net

networks:
  devsecops_net:
    external: true
```