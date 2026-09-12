// exercise every control form
function classify(n: number): number {
  if (n < 0) {
    return -1;
  } else if (n === 0) {
    return 0;
  } else {
    return 1;
  }
}

enum Kind {
  Low,
  High,
}

function run(xs: number[]): number {
  let total = 0;
  for (const x of xs) {
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

export { run, Kind };
