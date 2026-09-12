pragma solidity ^0.8.0;

import "./SafeMath.sol";

contract Token is ERC20, Ownable {
    uint256 public totalSupply;

    function balanceOf(address owner) public view returns (uint256) {
        return balances[owner];
    }

    function transfer(address to, uint256 amount) public returns (bool) {
        balances[msg.sender] -= amount;
        balances[to] += amount;
        return true;
    }
}
