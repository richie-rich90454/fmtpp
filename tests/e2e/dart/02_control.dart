// exercise every control form
int classify(int n) {
  if (n < 0) {
    return -1;
  } else if (n == 0) {
    return 0;
  } else {
    return 1;
  }
}

int run(List<int> xs) {
  var total = 0;
  for (var x in xs) {
    total = total + classify(x);
  }
  while (total > 4) {
    total = total - 4;
  }
  switch (total) {
    case 0:
      return 0;
    default:
      break;
  }
  return total;
}
