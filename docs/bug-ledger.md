# Lenovo Y710 case LED bug ledger

Bugs Daniel reported in Lenovo Y710 case LED, with causes, fixes and status. Read it before bug
hunting and keep it current. Other projects keep their own `docs/bug-ledger.md`;
log a bug in the ledger of the project whose code it lives in.

## Maintain this file

- Append a `BUG-NNN` entry whenever Daniel reports a bug, including reports made while
  working in another project. Reuse the ID for a recurrence and add the new source; reopen
  it when a later report contradicts a fix. Never renumber.
- Update root cause, fix commits and status when diagnosed or fixed. Keep hypotheses
  marked **unproven**. A build or deploy alone does not prove the behavior.
- Statuses: `open`, `fixed` (behavior verified), `fixed-unverified` (repair exists, check
  missing), `wontfix`, `unknown`.
- Keep a short user excerpt and the source conversation (tool + thread/session ID). Do not
  copy tokens, full transcripts, personal history or screenshots into the repo.
- Older reports can be backfilled from conversation history with TibiDash
  `scripts/audit-bug-reports.py` (read-only, no model calls).

Entry format:

```text
#### BUG-001 — Short symptom

- **First reported:** YYYY-MM-DD · **Status:** `open` · **Area:** component
- **Daniel's words:** "short excerpt"
- **Sources:** tool + thread/session ID
- **Root cause:** established cause, or **unproven**
- **Fix commits:** hashes, or none
- **Bug-hunting notes:** diagnostics, related entries
```

## Bugs

None recorded yet.
