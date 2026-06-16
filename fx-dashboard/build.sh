#!/bin/bash
# fx-dashboard 빌드 — FX-TRAPS 수치 주입(file_read 미지원 우회) + 표준 빌드(shim)
set -e
FX=/root/kim/freelang-v11-fx
TRAPS="$FX/FX-TRAPS.airc"
TC=$(grep -c '^@trap' "$TRAPS"); LC=$(grep -c '^@lock' "$TRAPS"); FC=$(grep -c '^@fact' "$TRAPS")
RT=$(grep '^  id: trap-' "$TRAPS" | tail -1 | awk '{print $2}')
sed -i "s/(define TRAP_COUNT [0-9]*)/(define TRAP_COUNT $TC)/;
        s/(define LOCK_COUNT [0-9]*)/(define LOCK_COUNT $LC)/;
        s/(define FACT_COUNT [0-9]*)/(define FACT_COUNT $FC)/;
        s|(define RECENT_TRAP \"[^\"]*\")|(define RECENT_TRAP \"$RT\")|" server.fl
echo "   FX-TRAPS 주입: trap=$TC lock=$LC fact=$FC recent=$RT"
bash "$FX/.fl-build-root.sh" server.fl fx-dashboard
