# DB95-8 fixed process_exit oracle

Freeze exit statuses 0, 1 and 37 for both the FreeLang builtin and legacy native
C entrypoint. The buffered pre-exit output must survive termination byte for
byte. The native observer registers a CLI arena cleanup and an atexit observer;
cleanup must run exactly once before the observer, without returning from exit.
The oracle pins the output markers and status values in process-exit.expected.tsv.
No platform exit-code range, existing handler validation or threshold changes.
The mainline already supplies the FreeLang handler. Add the missing native C ABI
entrypoint by delegating to that handler, preserving shutdown and C exit cleanup.
