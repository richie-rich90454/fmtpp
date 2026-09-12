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

// Vault holds deposits with events and a guard.
contract Vault {
    mapping(address => uint256) public deposits;

    event Deposited(address indexed who, uint256 amount);

    modifier positive(uint256 amount) {
        require(amount > 0, "zero");
        _;
    }

    function deposit() public payable positive(msg.value) {
        deposits[msg.sender] += msg.value;
        emit Deposited(msg.sender, msg.value);
    }

    function level(address who) public view returns (string memory) {
        if (deposits[who] == 0) {
            return "none";
        }
        return "some";
    }

    // total kept simple { no overflow worries here }
    function empty() public pure returns (bool) {
        return true;
    }

    function ping(uint256 n) public pure returns (uint256) {
        uint256 total = n;
        while (total > 10) {
            total = total - 10;
        }
        return total;
    }
}
