<template>
  <v-alert
    v-if="error"
    class="error-alert"
    type="error"
    action-label="error-alert"
    >{{ error }}
  </v-alert>
</template>

<script>
export default {
  props: {
    error: {
      type: String,
      default: '',
    },
  },
};
</script>

<style scoped>
.error-alert {
  position: fixed;
  top: 50px;
  right: 50px;
  z-index: 1000;
}
</style>
