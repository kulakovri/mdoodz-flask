<template>
  <div>
    <v-row>
      <v-col cols="12" md="8">
        <v-card class="mb-4">
          <v-btn @click="visualise">Visualise</v-btn>
          <v-divider></v-divider>
        </v-card>
        <v-img :src="image"></v-img>
        <v-slider v-model="pickedImageNumber" :max="imagesCount"></v-slider>
      </v-col>
    </v-row>
  </div>
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
      pickedImageNumber: 0,
      imagesCount: 0,
      image: '',
    };
  },
  computed: {
    simulationNames() {
      return Object.values(SimulationName);
    }
  },
  async beforeMount() {
    await this.getImageCount();
    await this.getImage();
  },
  methods: {
    async visualise() {
      try {
        await axios.post('api/visualise');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async getImageCount() {
      try {
        this.imagesCount = await axios.get('api/images') - 1;
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    async getImage() {
      try {
        this.image = `http://localhost:4000/api/images?number=${this.pickedImageNumber}`; // TODO remove hardcoded port
      } catch (ex) {
        userStore.setError(ex);
      }
    }
  },
  watch: {
    pickedImageNumber() {
      this.getImage();
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

