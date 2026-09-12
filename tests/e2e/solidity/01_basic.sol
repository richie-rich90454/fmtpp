pragma solidity ^0.8.0;

// a tiny counter
contract Counter {
    uint256 public value;

    function bump() public {
        value = value + 1;
    }

    function get() public view returns (uint256) {
        return value;
    }
}
