#!/usr/bin/env bash
set -Eeuo pipefail
test "$#" -gt 0
# Inspect the executed program, not regex arguments mentioning bootstrap.js.
awk -F '"' '/execve\(/ {
  p=$2; sub(/^.*\//,"",p)
  if (p ~ /^(node|nodejs)$/) node++
  if (p == "npm") npm++
  if (p == "npx") npx++
  if (p ~ /^(python[0-9.]*|pypy[0-9.]*)$/ || p ~ /\.py$/) python++
  if (p ~ /^(node|nodejs|js|qjs|jsc|d8|deno|bun|ts-node|tsx)$/ || p ~ /\.[cm]?js$/) js++
} END {
  printf "NODE=%d NPM=%d NPX=%d JS=%d PYTHON=%d\n",node,npm,npx,js,python
  if (node+npm+npx+js+python) exit 1
}' "$@"
