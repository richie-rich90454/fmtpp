import re

text = open('include/fmtpp/EasterEggs.hpp', 'r', encoding='utf-8').read()
eggs = re.findall(r'"(.*?)"', text)
print('count:', len(eggs))
print('unique:', len(set(eggs)))
for i, egg in enumerate(eggs):
    if not egg:
        print('empty at', i)
dups = []
seen = set()
for e in eggs:
    if e in seen:
        dups.append(e)
    seen.add(e)
print('duplicates:', dups)
