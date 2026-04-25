#!/usr/bin/env python3
"""
push_to_github.py

Replacement for `git add . && git commit -m <msg> && git push origin main`
that uses the GitHub REST Git Data API directly with a personal access token,
so it works in environments where local `git push` / `git commit` is blocked.

Steps performed:
  1. Read GITHUB_PERSONAL_ACCESS_TOKEN from the environment.
  2. Enumerate every file that `git add .` would stage (tracked + untracked,
     respecting .gitignore, minus locally-deleted-but-not-removed entries).
  3. Upload each file as a blob to the target repository.
  4. Build a fresh tree from those blobs, mirroring the working directory.
  5. If the tree differs from the current branch tip, create a commit whose
     parent is the current remote tip and fast-forward the branch ref.

Usage:
    python3 scripts/push_to_github.py "commit message"
    python3 scripts/push_to_github.py            # default message

Configuration via environment (with sensible defaults for this repo):
    GITHUB_REPO_OWNER  default: flinger-bit
    GITHUB_REPO_NAME   default: private-fps-mod
    GITHUB_BRANCH      default: main
"""

from __future__ import annotations

import base64
import json
import os
import subprocess
import sys
import urllib.error
import urllib.request

OWNER  = os.environ.get("GITHUB_REPO_OWNER", "flinger-bit")
REPO   = os.environ.get("GITHUB_REPO_NAME",  "private-fps-mod")
BRANCH = os.environ.get("GITHUB_BRANCH",     "main")
API    = "https://api.github.com"

TOKEN = os.environ.get("GITHUB_PERSONAL_ACCESS_TOKEN")
if not TOKEN:
    sys.exit("ERROR: GITHUB_PERSONAL_ACCESS_TOKEN is not set in the environment.")


def gh(method: str, path: str, payload: dict | None = None) -> dict:
    url = API + path
    data = json.dumps(payload).encode() if payload is not None else None
    req = urllib.request.Request(
        url,
        data=data,
        method=method,
        headers={
            "Authorization": f"Bearer {TOKEN}",
            "Accept": "application/vnd.github+json",
            "Content-Type": "application/json",
            "User-Agent": "replit-agent-pusher",
            "X-GitHub-Api-Version": "2022-11-28",
        },
    )
    try:
        with urllib.request.urlopen(req) as r:
            body = r.read()
            return json.loads(body) if body else {}
    except urllib.error.HTTPError as e:
        body = e.read().decode(errors="replace")
        sys.exit(f"GitHub API {method} {path} -> HTTP {e.code}\n{body}")


def files_to_stage() -> list[str]:
    """Mirror `git add .`: tracked + untracked-not-ignored, minus working-tree deletions."""
    listed = subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard"],
        text=True,
    ).splitlines()
    deleted = set(
        subprocess.check_output(["git", "ls-files", "--deleted"], text=True).splitlines()
    )
    out = []
    for path in listed:
        if not path or path in deleted:
            continue
        if not os.path.isfile(path):
            continue
        out.append(path)
    return sorted(out)


def main() -> None:
    msg = sys.argv[1] if len(sys.argv) > 1 else "Push from Replit Agent (GitHub API)"

    print(f"Target: {OWNER}/{REPO}@{BRANCH}")
    print(f"Message: {msg}")

    # 1. Current branch tip on GitHub
    ref = gh("GET", f"/repos/{OWNER}/{REPO}/git/refs/heads/{BRANCH}")
    base_sha = ref["object"]["sha"]
    base_commit = gh("GET", f"/repos/{OWNER}/{REPO}/git/commits/{base_sha}")
    base_tree_sha = base_commit["tree"]["sha"]
    print(f"Remote tip: {base_sha[:7]} (tree {base_tree_sha[:7]})")

    # 2. Build full tree mirroring the working directory
    paths = files_to_stage()
    print(f"Staging {len(paths)} file(s)...")

    tree_entries: list[dict] = []
    for i, path in enumerate(paths, start=1):
        with open(path, "rb") as f:
            content = f.read()
        blob = gh(
            "POST",
            f"/repos/{OWNER}/{REPO}/git/blobs",
            {"content": base64.b64encode(content).decode(), "encoding": "base64"},
        )
        mode = "100755" if os.access(path, os.X_OK) else "100644"
        tree_entries.append(
            {"path": path, "mode": mode, "type": "blob", "sha": blob["sha"]}
        )
        if i % 10 == 0 or i == len(paths):
            print(f"  uploaded {i}/{len(paths)}  last: {path}")

    # 3. Build a *replacement* tree (no base_tree) so deletions are honored.
    new_tree = gh(
        "POST",
        f"/repos/{OWNER}/{REPO}/git/trees",
        {"tree": tree_entries},
    )
    new_tree_sha = new_tree["sha"]
    print(f"New tree:   {new_tree_sha[:7]}")

    if new_tree_sha == base_tree_sha:
        print("Nothing to push: working tree already matches remote.")
        return

    # 4. Create the commit whose parent is the current remote tip
    commit = gh(
        "POST",
        f"/repos/{OWNER}/{REPO}/git/commits",
        {"message": msg, "tree": new_tree_sha, "parents": [base_sha]},
    )
    new_sha = commit["sha"]
    print(f"New commit: {new_sha[:7]}")

    # 5. Fast-forward the branch ref
    gh(
        "PATCH",
        f"/repos/{OWNER}/{REPO}/git/refs/heads/{BRANCH}",
        {"sha": new_sha, "force": False},
    )
    print(f"Pushed {new_sha[:7]} to {OWNER}/{REPO}@{BRANCH}")


if __name__ == "__main__":
    main()
