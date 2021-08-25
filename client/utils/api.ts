import { NuxtAxiosInstance } from '@nuxtjs/axios';

import { userStore } from '~/store';

let $axios: NuxtAxiosInstance;

export function initializeAxios(axiosInstance: NuxtAxiosInstance, app) {
  $axios = axiosInstance;

  $axios.interceptors.response.use((response) => {
    const { data, status, config } = response;
    if (status >= 400) {
      if (!data || !data.message) {
        userStore.setError(`Request failed with status ${status}.`);
      }
    }
    return response;
  });
}

export { $axios };
