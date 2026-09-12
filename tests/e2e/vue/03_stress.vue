<template>
  <div class="l1">
    <div class="l2">
      <section>
        <article>
          <div class="l5">
            <p>{{ deep }}</p>
          </div>
        </article>
      </section>
    </div>
  </div>
</template>

<script>
import { ref } from 'vue';

const deep = ref('down');

function nest(n) {
  if (n > 0) {
    for (let i = 0; i < n; i++) {
      while (n > i) {
        if (i === n) {
          return i;
        } else {
          n = n - 1;
        }
      }
    }
  }
  return n;
}
</script>

<style>
.l1 { color: red; }
</style>
