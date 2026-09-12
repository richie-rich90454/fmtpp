<template>
  <div class="app">
    <ul>
      <li v-for="t in todos">{{ t }}</li>
    </ul>
    <button @click="add">add</button>
  </div>
</template>

<script>
import { ref } from 'vue';

const todos = ref(['one']);

function add() {
  if (todos.value.length > 0) {
    todos.value.push('more');
  } else {
    todos.value = ['one'];
  }
  for (const t of todos.value) {
    show(t);
  }
  return todos.value.length;
}
</script>

<style>
.app {
  margin: 0 auto;
}
button {
  cursor: pointer;
}
</style>
