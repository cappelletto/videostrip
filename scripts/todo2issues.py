#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Create GitHub issues from TODO.md using gh CLI.

Usage examples:
  python tools/todo2issues.py --todo TODO.md --owner cappelletto \
      --milestone "v0.9.x — Integration & Performance Foundation" \
      --labels "integration,docs" \
      --dry-run

  python tools/todo2issues.py --todo TODO.md --owner cappelletto \
      --cross-target videostrip-meshroom
"""
import argparse, os, re, subprocess, sys, tempfile
from pathlib import Path
from typing import List, Tuple, Optional

# Import dataclass early to avoid undefined name error
try:
    from dataclasses import dataclass
except ImportError:
    print("ERROR: Python 3.7+ required (dataclasses).", file=sys.stderr)
    sys.exit(1)

SECTION_MESHROOM = "videostrip-meshroom"
SECTION_CORE = "videostrip"
SECTION_CROSS = "cross"

REPO_HEADER_RE = re.compile(
    r"^#\s*(?:(?:📦\s*New repo:\s*`(?P<meshroom>videostrip-meshroom)`)|"
    r"(?:🔧\s*Existing repo:\s*`(?P<core>videostrip)`)|"
    r"(?:🔁\s*Cross-repo coordination))\s*$"
)

ISSUE_TITLE_RE = re.compile(r"^###\s*Issue:\s*\*\*(?P<title>.+?)\*\*\s*$")
DESC_HEADER_RE = re.compile(r"^\s*\*\*Description:\*\*\s*$|^\s*\*\*Description\*\*\s*$")
ACPT_HEADER_RE = re.compile(r"^\s*\*\*Acceptance Criteria:\*\*\s*$|^\s*\*\*Acceptance Criteria\*\*\s*$")
HR_RE = re.compile(r"^\s*-{3,}\s*$")

def die(msg: str, code: int = 1):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(code)

def run(cmd: List[str], check=True, capture=False, input_data: Optional[bytes]=None):
    return subprocess.run(cmd, check=check,
                          stdout=subprocess.PIPE if capture else None,
                          stderr=subprocess.PIPE if capture else None,
                          input=input_data)

def ensure_gh():
    try:
        run(["gh", "--version"])
        run(["gh", "auth", "status"])
    except subprocess.CalledProcessError:
        die("gh is not installed or not authenticated. Run: gh auth login")

def parse_args():
    ap = argparse.ArgumentParser()
    ap.add_argument("--todo", default="TODO.md", help="Path to TODO.md")
    ap.add_argument("--owner", required=True, help="GitHub owner/org (e.g., cappelletto)")
    ap.add_argument("--milestone", default="", help="Milestone title to attach (optional)")
    ap.add_argument("--labels", default="", help="Comma-separated labels to apply (optional)")
    ap.add_argument("--cross-target", default="videostrip", help="Repo to use for cross-repo items (default: videostrip)")
    ap.add_argument("--dry-run", action="store_true", help="Print actions without creating issues")
    return ap.parse_args()

def current_section_target(line: str) -> Optional[str]:
    m = REPO_HEADER_RE.match(line.strip())
    if not m:
        return None
    if m.group("meshroom"):
        return SECTION_MESHROOM
    if m.group("core"):
        return SECTION_CORE
    return SECTION_CROSS

@dataclass
class Issue:
    repo: str
    title: str
    description: str
    criteria: List[str]

def parse_todo(todo_path: Path, owner: str, cross_target: str) -> List[Issue]:
    text = todo_path.read_text(encoding="utf-8").splitlines()
    issues: List[Issue] = []
    section = None
    i = 0
    while i < len(text):
        line = text[i]

        sec = current_section_target(line)
        if sec is not None:
            section = sec
            i += 1
            continue

        mtitle = ISSUE_TITLE_RE.match(line.strip())
        if mtitle and section:
            title = mtitle.group("title").strip()
            # advance to Description header
            i += 1
            while i < len(text) and not DESC_HEADER_RE.match(text[i].strip()):
                i += 1
            if i >= len(text): break
            i += 1  # skip Description header
            # collect description until Acceptance header or HR or next issue/section
            desc_lines = []
            while i < len(text):
                if ACPT_HEADER_RE.match(text[i].strip()) or ISSUE_TITLE_RE.match(text[i].strip()) \
                   or current_section_target(text[i]) is not None or HR_RE.match(text[i].strip()):
                    break
                desc_lines.append(text[i])
                i += 1

            # move to Acceptance header if not there yet
            while i < len(text) and not ACPT_HEADER_RE.match(text[i].strip()):
                # Stop if new issue/section
                if ISSUE_TITLE_RE.match(text[i].strip()) or current_section_target(text[i]) is not None:
                    break
                i += 1

            criteria: List[str] = []
            if i < len(text) and ACPT_HEADER_RE.match(text[i].strip()):
                i += 1  # skip Acceptance header
                while i < len(text):
                    s = text[i].strip()
                    if not s:
                        i += 1
                        break
                    if s.startswith("* "):
                        criteria.append(s[2:].strip())
                        i += 1
                        continue
                    if s.startswith("- "):
                        criteria.append(s[2:].strip())
                        i += 1
                        continue
                    if ISSUE_TITLE_RE.match(text[i].strip()) or current_section_target(text[i]) is not None or HR_RE.match(text[i].strip()):
                        break
                    # otherwise treat as continuation of previous bullet
                    if criteria:
                        criteria[-1] += " " + text[i].strip()
                    i += 1

            # map section → repo
            if section == SECTION_MESHROOM:
                repo = f"{owner}/videostrip-meshroom"
            elif section == SECTION_CORE:
                repo = f"{owner}/videostrip"
            else:
                repo = f"{owner}/{cross_target}"

            issues.append(Issue(repo=repo,
                                title=title,
                                description="\n".join(desc_lines).strip(),
                                criteria=criteria))
            continue

        i += 1
    return issues

def build_body(desc: str, criteria: List[str]) -> str:
    parts = []
    if desc:
        # Normalize single-line "**Description:**" markers if they remained
        d = desc
        parts.append("**Description**\n" + d.strip())
    if criteria:
        parts.append("\n**Expected Output / Acceptance Criteria**")
        for c in criteria:
            # render as GitHub checkbox
            parts.append(f"- [ ] {c}")
    return "\n".join(parts).strip() + "\n"

def create_issue(repo: str, title: str, body: str, milestone: str, labels: str, dry: bool):
    # Write body to temp file to avoid shell quoting issues
    with tempfile.NamedTemporaryFile("w", delete=False, encoding="utf-8", suffix=".md") as tf:
        tf.write(body)
        tmp = tf.name
    cmd = ["gh", "issue", "create", "--repo", repo, "--title", title, "--body-file", tmp]
    if milestone:
        cmd += ["--milestone", milestone]
    if labels:
        cmd += ["--label", labels]

    if dry:
        print("DRY-RUN:", " ".join(cmd))
        print(f"--- BODY ({tmp}) ---\n{body}\n--- END BODY ---\n")
    else:
        try:
            cp = run(cmd, capture=True)
            print(cp.stdout.decode("utf-8").strip())
        finally:
            try:
                os.unlink(tmp)
            except OSError:
                pass

def main():
    args = parse_args()
    ensure_gh()
    todo = Path(args.todo)
    if not todo.exists():
        die(f"TODO file not found: {todo}")
    issues = parse_todo(todo, args.owner, args.cross_target)
    if not issues:
        die("No issues parsed from TODO.md. Check formatting.")

    print(f"Found {len(issues)} issues. Creating…")
    for it in issues:
        body = build_body(it.description, it.criteria)
        create_issue(it.repo, it.title, body, args.milestone, args.labels, args.dry_run)
    print("Done.")

if __name__ == "__main__":
    main()