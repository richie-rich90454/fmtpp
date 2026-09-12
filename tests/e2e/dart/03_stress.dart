// deep nesting stress
int deep(int n) {
  var r = 0;
  if (n > 0) {
    for (var i = 0; i < n; i++) {
      while (r < i) {
        if (r % 2 == 0) {
          for (var j = 0; j < 2; j++) {
            if (j == i) {
              r = r + 1;
            } else {
              r = r + 2;
            }
          }
        } else {
          r = r + 1;
        }
      }
    }
  }
  return r;
}
