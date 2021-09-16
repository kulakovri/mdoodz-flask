<template>
  <div>
    <v-row>
      <v-col cols="12" md="8">
        <v-card class="mb-4">
          <v-btn @click="visualise">Visualise</v-btn>
          <v-divider></v-divider>
        </v-card>
        <v-carousel hide-delimiters :show-arrows="false" v-model="pickedImageNumber">
          <v-carousel-item
            v-for="image in images" :key="image.i" :src="image.link" :transition="false" :reverse-transition="false"
          ></v-carousel-item>
        </v-carousel>
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
      images: [],
    };
  },
  computed: {
    simulationNames() {
      return Object.values(SimulationName);
    }
  },
  async beforeMount() {
    await this.getImageCount();
    this.getImages();
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
        this.imagesCount = await axios.get('api/images');
      } catch (ex) {
        userStore.setError(ex);
      }
    },
    getImages() {
      this.images = [...Array(this.imagesCount)].map((_, i) => {
        return {
          number: i,
          link: `http://localhost:4000/api/images?number=${i}`,
        }
      })
    },
  },
  watch: {
    pickedImageNumber() {

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

