<template>
  <div>
    <v-btn @click="compile">Compile</v-btn>
    <v-btn @click="clean">Clean</v-btn>
    <v-btn @click="runSimulation">Run Simulation</v-btn>
    <v-btn @click="visualise">Visualise 1st Output</v-btn>
  </div>
</template>

<script>
import navigation from '~/plugins/mixins/navigation';
import { userStore } from '~/store';
import * as axios from '~/utils/axios';

export default {
  mixins: [navigation],
  methods: {
    async compile() {
      try {
        await axios.get('mdoodz/compile', {
          opt: true,
          mkl: true,
          modelName: 'RiftingPaulineMD6',
        });
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async clean() {
      try {
        await axios.post('mdoodz/clean');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async runSimulation() {
      try {
        await axios.post('mdoodz/run-simulation');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async visualise() {
      try {
        await axios.get('/visualise');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
  },
};
</script>
