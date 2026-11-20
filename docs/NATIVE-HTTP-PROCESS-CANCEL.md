# Bounded shell commands during HTTP shutdown

An actual Native application built and served its root successfully, but an API
request entered an external account CLI. Stopping the application exceeded the
existing three-second development shutdown bound. `shell_capture` creates its
own process group, so signalling the application group cannot stop that command.
Its normal 30-second deadline was still in force. The original failure and
subsequent recovery of owned children are retained.

The HTTP runtime now exposes a private optional cancellation hook. It is true
only on the thread currently handling a request, after HTTP shutdown has been
requested. The bounded `shell_capture` boundary observes it while collecting
output and while waiting for a child that already closed both output pipes.
Polling uses at most 100 ms slices while retaining the original monotonic
command deadline, so the request thread can also observe HTTP shutdown.

Cancellation sends TERM only to the invocation's own process group, gives it
200 ms to stop, completes group cleanup and reaps the direct child. `waitid` with
`WNOWAIT` keeps the direct child unreaped during signalling so its PID/group
cannot be reused. An already reaped child does not authorize signalling that
group. The three-field result retains captured output and reports `code: 1`,
even when the interrupted command handles TERM by returning 0. The FreeLang
request handler can then handle the failed command and finish normally.

This adds no application policy or language operation. Outside an HTTP request,
the optional hook is inactive. Normal commands retain the original 30,000 ms
budget, one-MiB limit per output stream, UTF-8 conversion and error/result shape.
The unbounded legacy `shell-exec-result` operation retains its behavior.

Receipt `native-http-process-cancel-20261120` covers open pipes, closed pipes,
TERM handlers returning 0, concurrent request groups, an unrelated process and
ordinary CLI execution after HTTP shutdown. It also repeats the original shell,
HTTP, SDK, frontend and default-development contracts with their original
fixtures, expected outputs and limits. See the
[exact evidence](evidence/native-http-process-cancel-20261120.json).

This is orderly HTTP shutdown of bounded core `shell_capture` work. Abrupt host
death, commands that create new sessions, the frontend's separate literal-argv
host, arbitrary process manipulation and transactional outcomes after interrupted
writes remain separate qualification work. Commercial HOLD; production NOT_RUN.
Private paths and owners: <가림>.
