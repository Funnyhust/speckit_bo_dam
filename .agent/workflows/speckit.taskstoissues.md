---
description: Convert items in tasks.md into GitHub Issues (or structured format suitable for issue tracking)
---
1. Read the command definition from `.gemini/commands/speckit.taskstoissues.toml`.
2. Parse the `prompt` field from the TOML file.
3. Parse `tasks.md` to identify task items.
4. Format these items as issue definitions (Title, Body, Labels).
