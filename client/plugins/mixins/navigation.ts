import Vue from 'vue';

export default Vue.extend({
  methods: {
    async goToHome() {
      await this.$router.push('/');
    },
  },
});
