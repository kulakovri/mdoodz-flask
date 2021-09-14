<template>
  <v-row>
    <v-col cols="12" md="8">
      <v-card class="mb-4">
        <div class="d-flex justify-space-between align-center">
          <v-card-title class="py-1 text-subtitle-1 font-weight-medium"
          >Simulation Options
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
        </v-card-text>
      </v-card>
      <v-btn @click="runSimulation">Run Simulation</v-btn>
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
    };
  },
  computed: {
    simulationNames() {
      return Object.values(SimulationName);
    }
  },
  async beforeMount() {
    await simulationStore.getCompilingCache();
    this.pickedSimulationName = simulationStore.compiling?.simulation_name || this.pickedSimulationName;
  },
  methods: {
    async runSimulation() {
      try {
        await axios.post('api/run-simulation', {
          opt: this.opt,
          mkl: this.mkl,
          simulation_name: this.pickedSimulationName,
        });
      } catch (ex) {
        userStore.setError(ex);
      }
    },
  },
};
</script>
