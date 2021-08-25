import { Plugin } from '@nuxt/types';

import { initializeAxios } from '~/utils/api';

const accessor: Plugin = ({ $axios, app }) => {
  initializeAxios($axios, app);
};

export default accessor;
