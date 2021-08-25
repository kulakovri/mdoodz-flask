<template>
  <div>
    <v-btn @click="doSomething">Button</v-btn>
    <v-btn @click="compile">Compile</v-btn>
    <v-btn @click="clean">Clean</v-btn>
    <v-btn @click="runSimulation">Run Simulation</v-btn>
    <v-btn @click="visualise">Visualise 1st Output</v-btn>
    <v-btn @click="getOutputFiles">Get Output Files Output</v-btn>
  </div>
</template>

<script>
import navigation from '~/plugins/mixins/navigation';
import { userStore } from '~/store';
import * as axios from '~/utils/axios';

export default {
  mixins: [navigation],
  methods: {
    async doSomething() {
      await axios.get('mdoodz');
    },
    async compile() {
      try {
        await axios.post('api/mdoodz/compile', {
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
        await axios.get('api/mdoodz/clean');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async runSimulation() {
      try {
        await axios.post('api/mdoodz/run-simulation', {
          modelName: 'RiftingPaulineMD6',
        });
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async visualise() {
      try {
        await axios.get('api/visualisation/first');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async getOutputFiles() {
      try {
        await axios.get('api/mdoodz/output');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
  },
};
</script>
