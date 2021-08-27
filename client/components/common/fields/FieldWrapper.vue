<template>
  <div class="field mb-2" @dblclick="labelClickListener">
    <div class="field__label mb-0 grey--text text-lighten-1 text-uppercase">
      {{ label }}
    </div>
    <div class="field__input-wrapper">
      <slot></slot>
    </div>
  </div>
</template>

<script>
export default {
  props: {
    label: {
      type: String,
      required: true,
    },
    labelClickListener: {
      type: Function,
      required: true,
    },
  },
};
</script>

<style lang="scss">
.field {
  cursor: pointer;
}

.field__label {
  font-size: 10px;
  line-height: 1em;
}

.field__prominent {
  font-size: 20px;
}

.v-icon.v-icon.field__input-icon,
.v-icon.v-icon.field__input-icon--hidden {
  display: none;
}

.field__input-wrapper {
  min-height: 20px;
  display: flex;
  justify-content: space-between;
  align-items: center;

  &:hover {
    background-color: rgba(25, 118, 210, 0.1);
  }

  &:hover > .v-icon.v-icon.field__input-icon {
    display: inline-flex;
  }

  .field__input {
    width: 100%;
  }
}
</style>
