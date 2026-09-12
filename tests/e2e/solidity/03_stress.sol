pragma solidity ^0.8.0;

// deep nesting stress
contract Deep {
    function run(uint256 n) public pure returns (uint256) {
        uint256 r = 0;
        if (n > 0) {
            for (uint256 i = 0; i < n; i++) {
                while (r < i) {
                    if (r % 2 == 0) {
                        for (uint256 j = 0; j < 2; j++) {
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
}
