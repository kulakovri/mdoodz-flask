<template>
  <v-row>
    <v-col cols="12" md="8">
      <v-card class="mb-4">
        <div class="d-flex justify-space-between align-center">
          <v-card-title class="py-1 text-subtitle-1 font-weight-medium"
          >Compilation Options</v-card-title
          >
        </div>
        <v-divider></v-divider>
        <v-card-text class="black--text">
          <PickListField
            v-model="pickedSimulationName"
            label="Simulation Name"
            :picklist-items="simulationNames"
          ></PickListField>
          <PickListField
            v-model="opt"
            label="Optimisation"
            :picklist-items="optValues"
          ></PickListField>
          <PickListField
            v-model="mkl"
            label="MKL"
            :picklist-items="mklValues"
          ></PickListField>
        </v-card-text>
      </v-card>
    </v-col>
    <v-btn @click="compile">Compile</v-btn>
  </v-row>
</template>

<script>
import navigation from '~/plugins/mixins/navigation';
import PickListField from "~/components/common/fields/PickListField";
import {userStore, simulationStore} from '~/store';
import * as axios from '~/utils/axios';
import {SimulationName} from "../../store/modules/simulation";

export default {
  mixins: [navigation],
  components: {PickListField},
  data() {
    return {
      pickedSimulationName: '',
      opt: '',
      optValues: ['yes', 'no'],
      mkl: '',
      mklValues: ['yes', 'no'],
      localValue: this.value,
    };
  },
  computed: {
    simulationNames() {
      return Object.values(SimulationName);
    }
  },
  methods: {
    async test() {
      console.log(this.pickedSimulationName);
    },
    async compile() {
      try {
        await axios.get('mdoodz/compile', {
          opt: this.opt,
          mkl: this.pickedSimulationName,
          modelName: this.pickedSimulationName,
        });
      } catch (ex) {
        userStore.setError(ex);
      }
    },
  },
};
</script>
