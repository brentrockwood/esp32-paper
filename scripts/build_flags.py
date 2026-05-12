import os
import re
import subprocess
Import("env")

def git(*args):
    return subprocess.check_output(
        ["git"] + list(args), stderr=subprocess.DEVNULL
    ).decode().strip()

try:
    raw = git("remote", "get-url", "origin")
    # Normalize SSH (git@github.com:user/repo.git) → HTTPS
    url = re.sub(r"^git@([^:]+):", r"https://\1/", raw)
    url = re.sub(r"\.git$", "", url)
except Exception:
    url = os.path.basename(os.getcwd())

try:
    commit = git("rev-parse", "--short", "HEAD")
except Exception:
    commit = "unknown"

env.Append(CPPDEFINES=[
    ("REPO_URL", '\\"' + url + '\\"'),
    ("GIT_HASH", '\\"' + commit + '\\"'),
])
