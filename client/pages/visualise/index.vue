<template>
  <v-row>
    <v-col cols="12" md="8">
      <v-card class="mb-4">
        <v-btn @click="visualise">Visualise</v-btn>
        <v-divider></v-divider>
        <div
          v-show="image"
          class="lightbox__image"
          :style="{ backgroundImage: image }"
        ></div>
      </v-card>

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
      image: null,
    };
  },
  computed: {
    simulationNames() {
      return Object.values(SimulationName);
    }
  },
  methods: {
    async visualise() {
      try {
        await axios.get('api/visualise');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
  },
};
</script>

<style scoped lang="scss">
.lightbox__image {
  width: 100%;
  height: 100%;
  min-width: 479px;
  min-height: 1000px;
  align-self: flex-start;
  background-repeat: no-repeat;
  background-size: contain;
  background-color: #444444;

  @media (max-width: 767px) {
    min-width: unset;
    width: calc(100% - 100px);
  }
}
</style>

