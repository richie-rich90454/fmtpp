// exercise every control form
function classify(n) {
  if (n < 0) {
    return -1;
  } else if (n === 0) {
    return 0;
  } else {
    return 1;
  }
}

function run(xs) {
  let total = 0;
  for (const x of xs) {
    total = total + classify(x);
  }
  while (total > 4) {
    total = total - 4;
  }
  switch (total) {
    case 0:
      return "zero";
    default:
      break;
  }
  try {
    check(total);
  } catch (e) {
    throw e;
  } finally {
    cleanup();
  }
  return total;
}

export { run };
