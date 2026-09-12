pragma solidity ^0.8.0;

contract Broken( {
    function value() public pure returns (uint256) {
        return 1;
    }
}

contract Program {
    function run() public pure {
        uint256 x = = 5;
        if x > 0
            ping();
    }
}
