<template>
  <!-- comment with { braces } inside -->
  <div class="app">
    <p>{{ "{ not a block }" }}</p>
  </div>
</template>

<script>
import { ref } from 'vue';

const s = "{ kept }";
// line comment with } brace
function show() {
  return s;
}
</script>

<style>
/* css comment with { } inside */
.app { color: red; }
</style>
