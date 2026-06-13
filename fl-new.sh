#!/bin/bash
# fl-new — FreeLang 네이티브 앱 스캐폴딩
# 사용법: fl-new <앱이름> [포트] [타입: api|web|full]
# 예시:  fl-new my-service 40920 api

APP_NAME="${1:-my-app}"
PORT="${2:-40920}"
TYPE="${3:-api}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEMPLATE_DIR="$SCRIPT_DIR/templates"

if [ -z "$1" ]; then
  echo "사용법: fl-new <앱이름> [포트] [타입: api|web]"
  echo "예시:  fl-new task-api 40920 api"
  exit 1
fi

TARGET_DIR="$(pwd)/$APP_NAME"

if [ -d "$TARGET_DIR" ]; then
  echo "❌ 디렉토리가 이미 존재합니다: $TARGET_DIR"
  exit 1
fi

mkdir -p "$TARGET_DIR"
echo "📁 디렉토리 생성: $TARGET_DIR"

# ── .projectrc.json ─────────────────────────────────────────────
cat > "$TARGET_DIR/.projectrc.json" << RCEOF
{
  "name": "$APP_NAME",
  "version": "1.0.0",
  "language": "FreeLang v11 (C Native)",
  "runtime": "cgc-bin → gcc → ELF",
  "status": "todo",
  "description": "$APP_NAME 서비스",
  "ports": [$PORT],
  "entry": "server.fl",
  "commands": {
    "build": "fl-build server.fl $APP_NAME",
    "start": "pm2 start ./$APP_NAME --name $APP_NAME",
    "dev": "fl-build server.fl $APP_NAME && ./$APP_NAME"
  }
}
RCEOF

# ── server.fl (API 타입) ─────────────────────────────────────────
if [ "$TYPE" = "api" ]; then
cat > "$TARGET_DIR/server.fl" << FLEOF
;; $APP_NAME — FreeLang 네이티브 API 서버
;; 빌드: fl-build server.fl $APP_NAME
;; 실행: ./$APP_NAME

(define PORT (num (or (env-get "PORT") "$PORT")))
(define DB_HOST  (or (env-get "DB_HOST") "localhost"))
(define DB_USER  (or (env-get "DB_USER") "akl"))
(define DB_PASS  (or (env-get "DB_PASS") "akl_pass_2026"))
(define DB_NAME  (or (env-get "DB_NAME") "akl"))

(define db (mariadb_connect DB_HOST 3306 DB_USER DB_PASS DB_NAME))

;; GET /health
(defn handle-health [\$req]
  (server_json "{\"status\":\"ok\",\"app\":\"$APP_NAME\",\"runtime\":\"native-c\"}"))

;; GET /api/items
(defn handle-list [\$req]
  (let [\$rows (mariadb_query db "SELECT 1 AS id, 'hello' AS name")]
    (server_json (json_stringify \$rows))))

;; POST /api/items
(defn handle-create [\$req]
  (let [\$body    (json_parse (server_req_body \$req))
        \$name    (or (get \$body "name") "")]
    (server_json "{\"ok\":true}")))

(server_get  "/health"    "handle-health")
(server_get  "/api/items" "handle-list")
(server_post "/api/items" "handle-create")

(server_start PORT)
FLEOF

elif [ "$TYPE" = "web" ]; then
cat > "$TARGET_DIR/server.fl" << FLEOF
;; $APP_NAME — FreeLang 네이티브 웹 앱
;; 빌드: fl-build server.fl $APP_NAME
;; 실행: ./$APP_NAME

(define PORT (num (or (env-get "PORT") "$PORT")))

;; GET /
(defn handle-index [\$req]
  (server_html "<!DOCTYPE html>
<html lang='ko'>
<head><meta charset='utf-8'><title>$APP_NAME</title>
<style>body{font-family:sans-serif;max-width:800px;margin:40px auto;padding:20px}</style>
</head>
<body>
<h1>$APP_NAME</h1>
<p>FreeLang 네이티브 C 런타임 실행 중 (포트 $PORT)</p>
</body></html>"))

;; GET /health
(defn handle-health [\$req]
  (server_json "{\"status\":\"ok\",\"app\":\"$APP_NAME\",\"runtime\":\"native-c\"}"))

(server_get "/" "handle-index")
(server_get "/health" "handle-health")

(server_start PORT)
FLEOF
fi

# ── Makefile ─────────────────────────────────────────────────────
cat > "$TARGET_DIR/Makefile" << MKEOF
.PHONY: build run dev clean

build:
	fl-build server.fl $APP_NAME

run: build
	./$APP_NAME

dev:
	@while inotifywait -e modify server.fl 2>/dev/null; do \\
		fl-build server.fl $APP_NAME && pkill -f ./$APP_NAME; \\
		./$APP_NAME & \\
	done

clean:
	rm -f $APP_NAME
MKEOF

echo ""
echo "✅ 앱 스캐폴딩 완료!"
echo ""
echo "  폴더:  $TARGET_DIR"
echo "  포트:  $PORT"
echo "  타입:  $TYPE"
echo ""
echo "다음 단계:"
echo "  cd $APP_NAME"
echo "  fl-build server.fl $APP_NAME"
echo "  ./$APP_NAME"
