import { $axios } from '~/utils/api';

const defaultConfig = {
  withCredentials: true,
  headers: { 'Content-Type': 'application/json' },
  // This rule is needed to fulfill all responses with a status lower than 500
  // to read and show error messages from the response body
  validateStatus: (status) => status < 500,
};

const validateResponse = (res) => {
  if (!res || !res.hasOwnProperty('data') || !res.hasOwnProperty('status')) {
    throw new Error('Something went wrong.');
  } else if (res.status >= 304) {
    throw new Error(res.data.message);
  } else {
    return res.data;
  }
};

export const post = async (url: string, payload?: any) => {
  return validateResponse(
    await $axios({
      url,
      method: 'POST',
      data: JSON.stringify(payload),
      ...defaultConfig,
    }),
  );
};

export const get = async (url: string) => {
  return validateResponse(
    await $axios({
      url,
      method: 'GET',
      ...defaultConfig,
    }),
  );
};

export const patch = async (url: string, payload?: any) => {
  return validateResponse(
    await $axios({
      url,
      method: 'PATCH',
      data: JSON.stringify(payload),
      ...defaultConfig,
    }),
  );
};

export const postFile = async (url, data: FormData) => {
  return validateResponse(
    await $axios({
      url,
      method: 'POST',
      withCredentials: true,
      headers: { 'Content-Type': 'multipart/form-data' },
      data,
    }),
  );
};

export const getFile = async (url) => {
  console.log($axios.getUri())
}
