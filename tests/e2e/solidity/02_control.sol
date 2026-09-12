pragma solidity ^0.8.0;

import "./SafeMath.sol";

// exercise control forms
contract Loops {
    function classify(int256 n) public pure returns (int256) {
        if (n < 0) {
            return -1;
        } else if (n == 0) {
            return 0;
        } else {
            return 1;
        }
    }

    function run(uint256 n) public pure returns (uint256) {
        uint256 total = 0;
        for (uint256 i = 0; i < n; i++) {
            total = total + i;
        }
        while (total > 4) {
            total = total - 4;
        }
        require(total >= 0, "bad");
        return total;
    }
}
