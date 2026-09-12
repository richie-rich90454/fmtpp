pragma solidity ^0.8.0;

// strings hold braces { like these } safely
contract Msgs {
    function open() public pure returns (string memory) {
        return "{ hello }";
    }

    function show() public pure returns (string memory) {
        string memory s = "a { b } c";
        // line comment with } brace
        /* block comment with { } inside */
        return s;
    }
}
