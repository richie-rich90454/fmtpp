<template>
  <div class="app">
    <p>{{ message }}</p>
  </div>
</template>

<script>
import { ref } from 'vue';

const message = ref('hello');
</script>

<style>
.app {
  color: red;
}
</style>
