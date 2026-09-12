<?php
// strings hold braces { like these } safely
function open_msg(): string {
    return "{ hello }";
}

function show(string $name): string {
    $s = "a { b } $name";
    // line comment with } brace
    /* block comment with { } inside */
    echo $s;
    return $s;
}
