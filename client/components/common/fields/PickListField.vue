<template>
  <FieldWrapper
    :label="label"
    :label-click-listener="labelClickListener"
  >
    <v-select
      v-if="isEditing"
      ref="input"
      v-model="localValue"
      dense
      :items="picklistItems"
      @blur="onSave"
      @change="onSave"
      @keyup.native="onKeyUp"
    >
      <template #item="{ item }">
        <span>{{ item }}</span>
      </template>
    </v-select>
    <div
      v-else
      @dblclick="onEdit"
    >
      {{ value | defaultValue }}
    </div>
  </FieldWrapper>
</template>

<script>
import FieldWrapper from '~/components/common/fields/FieldWrapper';

const defaultClass = 'field__input';

export default {
  components: {
    FieldWrapper,
  },
  data() {
    return {
      isEditing: false,
      localValue: this.value,
    };
  },
  model: {
    prop: 'value',
    event: 'change',
  },
  props: {
    label: {
      type: String,
      default: '',
    },
    picklistItems: {
      type: Array,
      required: true,
    },
    value: {
      type: [String, Object],
      default: '',
    },
  },
  watch: {
    value(newValue) {
      this.localValue = newValue;
    },
  },
  methods: {
    labelClickListener() {
      if (!this.isEditing) {
        this.onEdit();
      }
    },
    onEdit() {
      if (this.readonly) {
        return;
      }
      this.isEditing = true;
      this.$nextTick(() => {
        this.$refs.input.focus();
      });
    },
    onSave() {
      this.$emit('change', this.localValue);
      this.isEditing = false;
    },
    onKeyUp(event) {
      if (event.key === 'Escape') {
        this.onCancel();
      }
      if (event.key === 'Enter') {
        this.onSave();
      }
    },
  },
};
</script>
