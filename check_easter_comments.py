import os

# QA helper: ensure every project .hpp/.cpp source file contains an Easter egg comment.
# Third-party dependencies under lib/ are intentionally excluded.
# Easter egg: the script that hunts for hidden treasures in the source

missing = []
for root, dirs, files in os.walk('.'):
    # skip build artifacts, npm slots, specs, git metadata, and vendored deps
    if 'build' in root or 'npm' in root or '.trae' in root or '.git' in root or 'lib' in root:
        continue
    for f in files:
        if f.endswith('.hpp') or f.endswith('.cpp'):
            path = os.path.join(root, f)
            text = open(path, 'r', encoding='utf-8', errors='ignore').read()
            if 'Easter egg:' not in text and 'easter egg' not in text.lower():
                missing.append(path)

print('project files missing easter egg comment:', len(missing))
for m in missing:
    print(m)
