import Vue from 'vue';

const filters = {
  defaultValue(value: string) {
    if (value == undefined || value === '') {
      return '–';
    } else {
      return value;
    }
  },
};

function addFilters(VueInstance: any) {
  for (const filter in filters) {
    VueInstance.filter(filter, filters[filter]);
  }
}

const plugin = {
  install(Vue: any) {
    addFilters(Vue);
  },
};

Vue.use(plugin);
