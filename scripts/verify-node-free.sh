#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
TASK_TMP="${V21_TMP_ROOT:-/home/kim/tmp/RUN-20261104-29}"
export TMPDIR="$TASK_TMP/tmp"
AFJ_ROOT="${AFJ_DB95_ROOT:-}"
OUT="$ROOT/docs/node-free-table.md"
ROWS="$TASK_TMP/node-free-rows.tsv"
MATCHES="$TASK_TMP/node-free-matches.txt"
SAFE_PATH="$TASK_TMP/path-no-node"
TEST_LOG="$TASK_TMP/unit-path.log"
TRACE_LOG="$TASK_TMP/unit-trace.log"
TRACE_OUT="$TASK_TMP/unit-trace-output.log"
FIND_BIN="$(command -v find || true)"
LN_BIN="$(command -v ln || true)"
STRACE_BIN="$(command -v strace || true)"
BASH_BIN="/bin/bash"
COUNT_MODE="${1:-}"

if [[ -n "$COUNT_MODE" && "$COUNT_MODE" != "--exclude-docs" ]]; then
    printf 'usage: %s [--exclude-docs]\n' "${BASH_SOURCE[0]}" >&2
    exit 2
fi

mkdir -p "$TASK_TMP" "$TMPDIR" "$SAFE_PATH" "$(dirname "$OUT")"
: > "$ROWS"

declare -A SEEN_TEXT=()
NODE_FREE=0

relative_path() {
    local path="$1"
    if [[ "$path" == "$ROOT/"* ]]; then
        printf '%s' "${path#"$ROOT/"}"
    elif [[ -n "$AFJ_ROOT" && "$path" == "$AFJ_ROOT/"* ]]; then
        printf 'AFJ_DB95/%s' "${path#"$AFJ_ROOT/"}"
    else
        printf '%s' "$path"
    fi
}

add_row() {
    printf '%s\t%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" "${5:-1}" >> "$ROWS"
    if [[ "${5:-1}" == 1 ]]; then
        NODE_FREE=$((NODE_FREE + 1))
    fi
}

classify_row() {
    local where="$1" what="$2"
    local file="${where%:*}" source_line trimmed term lower_file
    source_line="${what#*: }"
    trimmed="${source_line#"${source_line%%[![:space:]]*}"}"
    term="${what#literal \'}"
    term="${term%%\'*}"
    lower_file="/${file,,}/"

    if [[ "$file" == Makefile || "$file" == */Makefile || "$file" == *package.json ]]; then
        printf 'build'
    elif [[ "$file" == *.md || "$file" == *.txt || "$trimmed" == \#* || "$trimmed" == //* || "$trimmed" == /\** || "$trimmed" == \** ]]; then
        printf 'docs'
    elif [[ "$lower_file" == */archive/* || "$lower_file" == */archived/* || "$lower_file" == */legacy/* || "$lower_file" == */backup/* || "$lower_file" == */old/* || "$lower_file" == */deprecated/* || "$lower_file" == */history/* ]]; then
        printf 'archive'
    elif [[ "$lower_file" == */generated/* || "$lower_file" == */build/* || "$lower_file" == */dist/* || "$lower_file" == */artifacts/* || "$lower_file" == */vendor/* || "$lower_file" == */coverage/* ]]; then
        printf 'generated'
    elif [[ "$file" == *.sh && ( "$source_line" == *'node "'* || "$source_line" == *'npm '* || "$source_line" == *'npx '* ) ]]; then
        printf 'execution'
    elif [[ "$file" == *.js || ( "$file" == *.sh && ( "$source_line" == *'run_case '*'.js"'* || "$source_line" == *'*.js)'* ) ) ]]; then
        printf 'input'
    elif [[ "$term" == .js || "$term" == .ts ]]; then
        printf 'extension-text'
    elif [[ "$term" == 'node ' || "$term" == npm || "$term" == npx ]]; then
        printf 'identifier-string'
    else
        printf 'other'
    fi
}

escape_cell() {
    local value="$1"
    value="${value//$'\t'/ }"
    value="${value//|/\\|}"
    value="${value//\`/\\\`}"
    printf '%s' "$value"
}

SEARCH_PATHS=("$ROOT/scripts" "$ROOT/tests" "$ROOT/self" "$ROOT/runtime")
[[ -e "$ROOT/Makefile" ]] && SEARCH_PATHS+=("$ROOT/Makefile")
if [[ -n "$AFJ_ROOT" ]]; then
    [[ -d "$AFJ_ROOT/scripts" ]] && SEARCH_PATHS+=("$AFJ_ROOT/scripts")
    [[ -d "$AFJ_ROOT/tests" ]] && SEARCH_PATHS+=("$AFJ_ROOT/tests")
fi

TERMS=("node " npm npx .js .ts fl-cli package.json)
for term in "${TERMS[@]}"; do
    : > "$MATCHES"
    rg -n --no-heading --fixed-strings --glob '!verify-node-free.sh' "$term" "${SEARCH_PATHS[@]}" > "$MATCHES" 2>/dev/null || true
    while IFS= read -r match; do
        [[ -n "$match" ]] || continue
        file="${match%%:*}"
        rest="${match#*:}"
        line_no="${rest%%:*}"
        source_line="${rest#*:}"
        rel="$(relative_path "$file")"
        key="$rel:$line_no"
        [[ -n "${SEEN_TEXT[$key]+x}" ]] && continue
        SEEN_TEXT[$key]=1
        add_row "$rel:$line_no" "literal '$term': $(escape_cell "$source_line")" "글자" "해당 호출을 native FreeLang/C 경로로 대체 검토"
    done < "$MATCHES"
done

# Verify the previously reported AFJ JS branch and the 14 JS gate inputs.
KNOWN_GATE="$AFJ_ROOT/tests/native-release-gate.sh"
KNOWN_RELEASE="$AFJ_ROOT/tests/release-gate.sh"
KNOWN_GATE_FOUND=0
KNOWN_JS_CASES=0
if [[ -n "$AFJ_ROOT" && -f "$KNOWN_GATE" ]]; then
    if rg -n -F '*.js)' "$KNOWN_GATE" > "$MATCHES" 2>/dev/null; then
        KNOWN_GATE_FOUND=1
    fi
fi
if [[ -n "$AFJ_ROOT" && -f "$KNOWN_RELEASE" ]]; then
    while IFS= read -r match; do
        [[ -n "$match" ]] || continue
        if [[ "$match" == *' node '* && "$match" == *.js* ]]; then
            KNOWN_JS_CASES=$((KNOWN_JS_CASES + 1))
        fi
    done < <(rg -n 'run_case .*node .*\.js"$' "$KNOWN_RELEASE" 2>/dev/null || true)
fi

SAFE_PATH="${SAFE_PATH}"
if [[ -n "$FIND_BIN" && -n "$LN_BIN" ]]; then
    OLD_IFS="$IFS"
    IFS=:
    for path_dir in $PATH; do
        [[ -d "$path_dir" ]] || continue
        while IFS= read -r -d '' candidate; do
            command_name="${candidate##*/}"
            case "$command_name" in
                node|nodejs|npm|npx|python|python[0-9]*|deno|bun) continue ;;
            esac
            [[ -x "$candidate" && ! -e "$SAFE_PATH/$command_name" ]] || continue
            "$LN_BIN" -s "$candidate" "$SAFE_PATH/$command_name" 2>/dev/null || true
        done < <("$FIND_BIN" "$path_dir" -maxdepth 1 \( -type f -o -type l \) -executable -print0 2>/dev/null)
    done
    IFS="$OLD_IFS"
fi

install_script=""
for candidate in "$ROOT/install.sh" "$ROOT/scripts/install.sh"; do
    if [[ -f "$candidate" ]]; then install_script="$candidate"; break; fi
done
if [[ -n "$install_script" ]]; then
    if PATH="$SAFE_PATH" "$BASH_BIN" "$install_script" > "$TASK_TMP/install-path.log" 2>&1; then
        install_rc=0
    else
        install_rc=$?
    fi
    add_row "install.sh" "PATH without node/npm/npx: rc=$install_rc" "실행" "필요 명령을 native 설치 절차로 대체"
else
    add_row "install.sh" "파일 없음; 실행하지 않음" "실행" "설치 진입점을 저장소 문서에 명시" 0
fi

UNIT_TEST="$ROOT/tests/run-tests.sh"
if [[ -f "$UNIT_TEST" ]]; then
    if PATH="$SAFE_PATH" "$BASH_BIN" "$UNIT_TEST" > "$TEST_LOG" 2>&1; then
        unit_rc=0
    else
        unit_rc=$?
    fi
    if rg -q 'node:.*(not found|No such file)|npm:.*(not found|No such file)|npx:.*(not found|No such file)' "$TEST_LOG"; then
        add_row "tests/run-tests.sh" "node/npm/npx 필요 호출; PATH에서 차단됨 (rc=$unit_rc)" "실행" "FreeLang native runner로 실행"
    else
        add_row "tests/run-tests.sh" "node/npm/npx 차단 PATH에서 rc=$unit_rc" "실행" "실패가 있으면 로그 원인 확인" 0
    fi
else
    unit_rc=127
    add_row "tests/run-tests.sh" "단위 시험 러너 파일 없음" "실행" "native 단위 시험 진입점 추가" 0
fi

if [[ -n "$STRACE_BIN" && -f "$UNIT_TEST" ]]; then
    if PATH="$SAFE_PATH" "$STRACE_BIN" -f -e trace=execve -o "$TRACE_LOG" "$BASH_BIN" "$UNIT_TEST" > "$TRACE_OUT" 2>&1; then
        trace_rc=0
    else
        trace_rc=$?
    fi
    : > "$MATCHES"
    rg -n 'execve\("[^\"]*/(node|nodejs|npm|npx|python|python[0-9.]*|deno|bun)"' "$TRACE_LOG" > "$MATCHES" 2>/dev/null || true
    trace_count=0
    while IFS= read -r match; do
        [[ -n "$match" ]] || continue
        trace_count=$((trace_count + 1))
        add_row "tests/run-tests.sh" "금지 런타임 execve 확인 (rc=$trace_rc)" "추적" "프로세스를 FreeLang/C로 대체"
    done < "$MATCHES"
    if [[ "$trace_count" -eq 0 ]]; then
        add_row "tests/run-tests.sh" "node/npm/npx/python execve 없음 (rc=$trace_rc)" "추적" "해당 없음" 0
    fi
else
    add_row "strace" "strace 또는 단위 시험 러너 없음; 추적 미수행" "추적" "strace 설치 경로를 제공해 재검증" 0
fi

CLASS_EXECUTION=0
CLASS_INPUT=0
CLASS_BUILD=0
CLASS_DOCS=0
CLASS_ARCHIVE=0
CLASS_GENERATED=0
CLASS_EXTENSION_TEXT=0
CLASS_IDENTIFIER_STRING=0
CLASS_OTHER=0
while IFS=$'\t' read -r where what kind suggestion counted; do
    [[ "$counted" == 1 ]] || continue
    row_class="$(classify_row "$where" "$what")"
    case "$row_class" in
        execution) CLASS_EXECUTION=$((CLASS_EXECUTION + 1)) ;;
        input) CLASS_INPUT=$((CLASS_INPUT + 1)) ;;
        build) CLASS_BUILD=$((CLASS_BUILD + 1)) ;;
        docs) CLASS_DOCS=$((CLASS_DOCS + 1)) ;;
        archive) CLASS_ARCHIVE=$((CLASS_ARCHIVE + 1)) ;;
        generated) CLASS_GENERATED=$((CLASS_GENERATED + 1)) ;;
        extension-text) CLASS_EXTENSION_TEXT=$((CLASS_EXTENSION_TEXT + 1)) ;;
        identifier-string) CLASS_IDENTIFIER_STRING=$((CLASS_IDENTIFIER_STRING + 1)) ;;
        *) CLASS_OTHER=$((CLASS_OTHER + 1)) ;;
    esac
done < "$ROWS"
NODE_CLASS_SUM=$((CLASS_EXECUTION + CLASS_INPUT + CLASS_BUILD + CLASS_DOCS + CLASS_ARCHIVE + CLASS_GENERATED + CLASS_EXTENSION_TEXT + CLASS_IDENTIFIER_STRING + CLASS_OTHER))
NODE_REAL=$((CLASS_EXECUTION + CLASS_BUILD))

# Keep the generated table stable even if rg returns filesystem entries in a different order.
LC_ALL=C sort -u "$ROWS" -o "$ROWS"

{
    printf '# Node-free 의존성 탐지표\n\n'
    printf '검색 대상: `scripts/`, `tests/`, `self/`, `runtime/`, `Makefile`'
    [[ -n "$AFJ_ROOT" ]] && printf ', AFJ DB95의 `scripts/`, `tests/`'
    printf '. 글자 검색어: `node `, `npm`, `npx`, `.js`, `.ts`, `fl-cli`, `package.json`.\n\n'
    printf 'AFJ native-release-gate `.js` 분기 탐지: %s; AFJ `.js` run_case 수: %s.\n\n' "$KNOWN_GATE_FOUND" "$KNOWN_JS_CASES"
    printf '| 어디서 | 무엇을 | 종류 | 대체 방법 제안 |\n|---|---|---|---|\n'
    while IFS=$'\t' read -r where what kind suggestion counted; do
        printf '| `%s` | %s | %s | %s |\n' "$(escape_cell "$where")" "$(escape_cell "$what")" "$kind" "$(escape_cell "$suggestion")"
    done < "$ROWS"
    printf '\nNODE_FREE=%s\n' "$NODE_FREE"
} > "$OUT"

printf 'NODE_FREE=%s\n' "$NODE_FREE"
printf 'KNOWN_GATE_FOUND=%s\n' "$KNOWN_GATE_FOUND"
printf 'KNOWN_JS_CASES=%s\n' "$KNOWN_JS_CASES"
printf 'PATH_UNIT_RC=%s\n' "$unit_rc"
if [[ "$COUNT_MODE" == "--exclude-docs" ]]; then
    printf 'CLASS_EXECUTION=%s\n' "$CLASS_EXECUTION"
    printf 'CLASS_INPUT=%s\n' "$CLASS_INPUT"
    printf 'CLASS_BUILD=%s\n' "$CLASS_BUILD"
    printf 'CLASS_DOCS=%s\n' "$CLASS_DOCS"
    printf 'CLASS_ARCHIVE=%s\n' "$CLASS_ARCHIVE"
    printf 'CLASS_GENERATED=%s\n' "$CLASS_GENERATED"
    printf 'CLASS_EXTENSION_TEXT=%s\n' "$CLASS_EXTENSION_TEXT"
    printf 'CLASS_IDENTIFIER_STRING=%s\n' "$CLASS_IDENTIFIER_STRING"
    printf 'CLASS_OTHER=%s\n' "$CLASS_OTHER"
    printf 'CLASS_SUM=%s\n' "$NODE_CLASS_SUM"
    printf 'NODE_REAL=%s\n' "$NODE_REAL"
fi
