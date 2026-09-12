# e2e-markers: else:, except:, def broken, surviving
def ok():
    return 1


x = 1
else:
    orphan_else()


try:
    a()
finally:
    b()
except E:
    late()


def broken(:
    pass


if x
    no_colon()


for i in items
    pass


while x
    pass


def no_suite():
class also_no_suite:
if lonely:
while solitary:
    pass


y = 2
