<template>
  <v-row>
    <v-col cols="12" md="8">
      <v-card class="mb-4">
        <div class="d-flex justify-space-between align-center">
          <v-card-title class="py-1 text-subtitle-1 font-weight-medium"
          >Compilation Options
          </v-card-title
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
      <v-btn @click="compile">Compile</v-btn>
      <v-btn @click="clean">Clean</v-btn>
    </v-col>
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
      pickedSimulationName: SimulationName.PlateauPierre,
      opt: 'yes',
      optValues: ['yes', 'no'],
      mkl: 'yes',
      mklValues: ['yes', 'no'],
    };
  },
  computed: {
    simulationNames() {
      return Object.values(SimulationName);
    },
  },
  async beforeMount() {
    await simulationStore.getCompilingCache();
    this.pickedSimulationName = simulationStore.compiling?.simulation_name || this.pickedSimulationName;
    this.opt = simulationStore.compiling?.opt || this.opt;
    this.mkl = simulationStore.compiling?.mkl || this.mkl;
  },
  methods: {
    async compile() {
      try {
        await axios.post('api/compile', {
          opt: this.opt,
          mkl: this.mkl,
          simulation_name: this.pickedSimulationName,
        });
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async clean() {
      try {
        await axios.post('api/clean');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
  },
};
</script>
