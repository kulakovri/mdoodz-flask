import { Store } from 'vuex';
import { getModule } from 'vuex-module-decorators';

import user from '~/store/modules/user';
import simulation from '~/store/modules/simulation';

let userStore: user;
let simulationStore: simulation;

function initialiseStores(store: Store<any>): void {
  userStore = getModule(user, store);
  simulationStore = getModule(simulation, store);
}

export {
  initialiseStores,
  userStore,
  simulationStore,
};
