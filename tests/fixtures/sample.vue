<template>
  <div class="app">
    <p>{{ message }}</p>
    <ul>
      <li v-for="t in todos">{{ t }}</li>
    </ul>
    <button @click="add">add</button>
  </div>
</template>

<script>
import { ref } from 'vue';

const message = ref('hello');
// todos list below
const todos = ref(['one', 'two']);

function add() {
  todos.value.push('todo ' + todos.value.length);
  return todos.value.length;
}

function reset() {
  todos.value = [];
  return 0;
}

function count() {
  if (todos.value.length > 0) {
    return todos.value.length;
  }
  return 0;
}
</script>

<style>
.app {
  color: red;
}
.list {
  margin: 0;
}
/* braces { } in comments stay put */
button {
  cursor: pointer;
}
p {
  line-height: 1.5;
}
.small {
  font-size: 12px;
}
</style>
