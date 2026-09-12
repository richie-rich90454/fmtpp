<?php
// exercise every control form
function classify(int $n): int {
    if ($n < 0) {
        return -1;
    } elseif ($n == 0) {
        return 0;
    } else {
        return 1;
    }
}

function run(array $xs): int {
    $total = 0;
    foreach ($xs as $x) {
        $total = $total + classify($x);
    }
    while ($total > 4) {
        $total = $total - 4;
    }
    switch ($total) {
        case 0:
            return 0;
        default:
            break;
    }
    try {
        check($total);
    } catch (Exception $e) {
        return -1;
    }
    return $total;
}
