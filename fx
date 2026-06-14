#!/usr/bin/env node
/**
 * fx — FreeLang C 네이티브 독립 언어 CLI
 *
 * 명령어:
 *   fx new <name>        새 프로젝트 생성
 *   fx build [file]      빌드 (기본: server.fl)
 *   fx run [file]        빌드 + 실행
 *   fx check [file]      문법 검사
 *   fx info              현재 프로젝트 정보
 *   fx version           버전 정보
 */

const { execSync, spawnSync } = require('child_process');
const fs   = require('fs');
const path = require('path');

const FX_ROOT   = path.resolve(__dirname);
const BUILD_SH  = path.join(FX_ROOT, 'fl-build.sh');
const CGC_BIN   = '/home/kimjin/freelang-v11/bin/cgc-bin';
const V11_BS    = '/home/kimjin/freelang-v11/bootstrap.js';

const VERSION = '1.0.0';

/* ── 색상 ────────────────────────────────────── */
const c = {
  reset:  '\x1b[0m',
  bold:   '\x1b[1m',
  green:  '\x1b[32m',
  yellow: '\x1b[33m',
  red:    '\x1b[31m',
  cyan:   '\x1b[36m',
  gray:   '\x1b[90m',
};
const ok  = s => `${c.green}✅${c.reset} ${s}`;
const err = s => `${c.red}❌${c.reset} ${s}`;
const inf = s => `${c.cyan}ℹ${c.reset}  ${s}`;
const hd  = s => `${c.bold}${c.cyan}${s}${c.reset}`;

/* ── 서브명령 ─────────────────────────────────── */

function cmdVersion() {
  console.log(`\n${hd('fx')} — FreeLang C 네이티브 언어`);
  console.log(`  버전  : ${VERSION}`);
  console.log(`  CGC   : ${CGC_BIN}`);
  console.log(`  빌드  : ${BUILD_SH}`);
  console.log(`  고정점: ${c.green}SHA 18db9db1 (gen-a==gen-b==gen-c)${c.reset}\n`);
}

function cmdNew(name) {
  if (!name) { console.error(err('프로젝트 이름 필요: fx new <name>')); process.exit(1); }
  const dir = path.resolve(name);
  if (fs.existsSync(dir)) { console.error(err(`이미 존재: ${dir}`)); process.exit(1); }
  fs.mkdirSync(dir, { recursive: true });

  // server.fl 템플릿
  fs.writeFileSync(path.join(dir, 'server.fl'), `\
;; ${name} — fx 앱
;; 빌드: fx build
;; 실행: fx run

(define PORT 40300)

(defn handle-index [$req]
  (server_json (json_stringify {"ok" true "app" "${name}" "version" "1.0.0"})))

(defn handle-health [$req]
  (server_json (json_stringify {"status" "ok"})))

(println (str "${name} 포트 " PORT " 시작"))
(server_get "/"        "handle-index")
(server_get "/health"  "handle-health")
(server_start PORT)
`);

  // .projectrc.json
  fs.writeFileSync(path.join(dir, '.projectrc.json'), JSON.stringify({
    name,
    version: '1.0.0',
    language: 'fx (FreeLang C 네이티브)',
    status: 'todo',
    entry: 'server.fl',
    binary: name,
    commands: {
      build: 'fx build',
      run:   'fx run',
    },
  }, null, 2));

  // .gitignore
  fs.writeFileSync(path.join(dir, '.gitignore'), `${name}\n*.o\n/tmp/\n`);

  console.log(ok(`${name}/ 프로젝트 생성 완료`));
  console.log(inf(`cd ${name} && fx run`));
}

function cmdBuild(file) {
  const src = file || findEntry();
  if (!fs.existsSync(src)) { console.error(err(`파일 없음: ${src}`)); process.exit(1); }

  const rc  = loadRc();
  const out = rc ? rc.binary || path.basename(src, '.fl') : path.basename(src, '.fl');

  console.log(`\n${hd('fx build')} ${src} → ${out}`);
  const t0 = Date.now();

  const r = spawnSync('bash', [BUILD_SH, src, out], { stdio: 'inherit' });
  if (r.status !== 0) { console.error(err('빌드 실패')); process.exit(1); }

  console.log(inf(`완료: ${Date.now() - t0}ms`));
  return out;
}

function cmdRun(file) {
  const bin = cmdBuild(file);
  console.log(`\n${hd('fx run')} ./${bin}\n`);
  const r = spawnSync(`./${bin}`, [], { stdio: 'inherit' });
  process.exit(r.status || 0);
}

function cmdCheck(file) {
  const src = file || findEntry();
  if (!fs.existsSync(src)) { console.error(err(`파일 없음: ${src}`)); process.exit(1); }
  console.log(`\n${hd('fx check')} ${src}`);

  // cgc-bin으로 C 코드 생성 시도 (문법 오류 검출)
  const tmp = `/tmp/fx_check_${Date.now()}.c`;
  const r = spawnSync(CGC_BIN, [src, tmp], { stdio: ['inherit', 'inherit', 'pipe'] });
  const stderr = r.stderr ? r.stderr.toString() : '';

  if (r.status === 0) {
    fs.existsSync(tmp) && fs.unlinkSync(tmp);
    console.log(ok('문법 검사 통과'));
  } else {
    console.error(err('문법 오류:'));
    console.error(stderr);
    process.exit(1);
  }
}

function cmdInfo() {
  const rc = loadRc();
  if (!rc) { console.log(inf('.projectrc.json 없음 — fx new <name>으로 생성하세요')); return; }
  console.log(`\n${hd('fx info')}`);
  console.log(`  이름    : ${rc.name}`);
  console.log(`  버전    : ${rc.version || '-'}`);
  console.log(`  언어    : ${rc.language || 'fx'}`);
  console.log(`  상태    : ${rc.status || '-'}`);
  console.log(`  진입점  : ${rc.entry || 'server.fl'}`);
  console.log(`  바이너리: ${rc.binary || rc.name}`);
  if (rc.ports) console.log(`  포트    : ${rc.ports.join(', ')}`);
  console.log();
}

/* ── 유틸 ─────────────────────────────────────── */

function loadRc() {
  const p = path.resolve('.projectrc.json');
  if (!fs.existsSync(p)) return null;
  try { return JSON.parse(fs.readFileSync(p, 'utf8')); } catch { return null; }
}

function findEntry() {
  const rc = loadRc();
  if (rc && rc.entry) return rc.entry;
  if (fs.existsSync('server.fl')) return 'server.fl';
  if (fs.existsSync('main.fl'))   return 'main.fl';
  const fls = fs.readdirSync('.').filter(f => f.endsWith('.fl'));
  if (fls.length > 0) return fls[0];
  return 'server.fl';
}

/* ── 메인 ─────────────────────────────────────── */

const [,, cmd, ...args] = process.argv;

switch (cmd) {
  case 'new':     cmdNew(args[0]);     break;
  case 'build':   cmdBuild(args[0]);   break;
  case 'run':     cmdRun(args[0]);     break;
  case 'check':   cmdCheck(args[0]);   break;
  case 'info':    cmdInfo();           break;
  case 'version':
  case '-v':
  case '--version': cmdVersion();      break;
  default:
    console.log(`\n${hd('fx')} — FreeLang C 네이티브 독립 언어 v${VERSION}`);
    console.log(`
  ${c.cyan}fx new <name>${c.reset}    새 프로젝트 생성
  ${c.cyan}fx build${c.reset}         빌드 (→ ELF 바이너리)
  ${c.cyan}fx run${c.reset}           빌드 + 즉시 실행
  ${c.cyan}fx check${c.reset}         문법 검사
  ${c.cyan}fx info${c.reset}          프로젝트 정보
  ${c.cyan}fx version${c.reset}       버전 정보
`);
}
