# Native atomic file publication

`fd-publish` / `fd_publish` takes an exclusively created synthetic handle and
a destination path. It is intended for small durable pointer/manifest files;
storage algorithms and recovery policy remain FreeLang code.

~~~clojure
(let [fd (fd_create candidate)]
  (try
    (do (fd_write fd contents) (fd_publish fd destination))
    (finally (fd_close fd))))
~~~

The caller supplies a unique candidate in the destination directory and owns
that directory/candidate while publishing. fd_create retains its checked
parent descriptor, uses O_EXCL/O_NOFOLLOW and creates mode 0600. Publication
checks that the source still names the same regular inode, has one link and
fits the existing file-size limit. It resolves the destination through the
same path policy, requires the same physical parent directory, and rejects
the same name/inode, symlinks and nonregular targets. A missing target is
allowed. It does not create parent directories or copy files across them.

The order is candidate fsync, descriptor-relative renameat, destination
directory fsync. All failures throw; no error becomes a successful boolean.
After rename, writes/truncation through that handle are disabled, including
when directory fsync fails. The handle stays open and must be closed. An
error named fd_publish_after_rename means publication may already have
occurred: callers must not assume the old target remains or blindly retry.
Before rename, the old target is untouched. Published files retain mode 0600.

The runtime closes its additional directory descriptor and releases its
handle-registry mutex before raising. Existing descriptor operations, size
limits and legacy rename behavior are unchanged. This operation does not
protect against a trusted owner independently rewriting the candidate via
another OS descriptor, provide a database/process ownership lock, implement
generation GC, or define transaction recovery. Callers must keep published
data immutable and preserve older generations until a safe retention policy
exists. Real power-loss durability also depends on filesystem/storage fsync
semantics; SIGKILL tests are not a power-loss certification.

## Evidence

Host paths/owners: <가림>. Private receipt native-fd-publish.ucdhr9i5.

| Check | Result |
|---|---|
| Prior verified runtime capability | Exit 77, fd-publish unsupported |
| New boundary contract | 29 cases PASS; pipeline exit 0 |
| File sync / rename failure | EIO/EACCES propagated; old target retained |
| Directory sync failure after rename | Error propagated; complete new target; explicit after-rename error |
| Actual SIGKILL after successful file fsync / rename | Prior / complete new target respectively |
| Same/cross-directory/path traversal/base escape/symlink/nonregular target | Rejected |
| Replaced source / hardlinked source / oversize source / wrong handle or arity | Rejected |
| Post-publication write/truncate/republication | Rejected |
| Descriptor lifecycle | 256 actual create/publish/close cycles; descriptor delta 0 |
| Existing fd-writer.fl / fd-reader.fl | Unmodified expected output matches; exit 0 |
| Public FreeLang fd_publish call with Unicode data and quoted/spaced root | PASS; exit 0 |
| Collected boundary trace | 528 fsync calls, 264 renameat calls; Node executions/connect calls 0 |

The contracts were authored privately before the production edit; a later
oversize-source case extended them. Existing repository tests, expected
outputs, protected runners, limits and seed were not changed.

Fresh bootstrap receipt native-fd-fixedpoint.vqfg8d9r builds the actual seed
through Stage4. Stage2 = Stage3 = Stage4 SHA-256 is
c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa.
The matching compiler ELF SHA-256 is
e28b5062811fe1d65125e4d37f77b3f791fca85586fd2985c84d5ef1ceb39c5c.
The final two generation measurements were 39.54 / 39.39 seconds and
1,525,688 / 1,526,968 KiB maximum RSS under the unchanged 2 GiB cap.
Seed bootstrap/stage0.c remains
0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.

Fresh core receipt native-fd-core.rbl02jq_ passes frontend 71/71, CLI 22/22,
FX regressions 11/11, checker fixtures 32/32 plus default checking, literal/
value/syntax contracts, and original DB95 cases 7/13/36/40. Collected core
exec traces contain no Node-family executions; source/seed are unchanged.
This is not a full DB95 gate or commercial-readiness acceptance. The stale
seed pin in the original protected verifier remains unchanged; this receipt
uses the previously documented private bootstrap recipe with the actual
unchanged seed. Existing private curl 8.9.1 is used only through environment
variables, with its approved header/library hashes rechecked. Distribution
packaging remains outstanding. No download/install/deployment was performed.
