import {Action, Module, Mutation, VuexModule} from "vuex-module-decorators";
import * as axios from '~/utils/axios';
import {userStore} from '~/store';

export enum SimulationName {
  Duretz14_SimpleShear = 'Duretz14_SimpleShear',
  Huismans = 'Huismans',
  LithoScaleClement4 = 'LithoScaleClement4',
  LorenzoWilsonEllipse = 'LorenzoWilsonEllipse',
  MetamSoleRidge_v3 = 'MetamSoleRidge_v3',
  PlateauPierre = 'PlateauPierre',
  RiftingPaulineMD6 = 'RiftingPaulineMD6',
  StefanBending = 'StefanBending',
}

@Module({
  name: 'modules/simulation',
  stateFactory: true,
  namespaced: true,
})
export default class Simulation extends VuexModule {
  private _compiling: any | null = null;

  get compiling() {
    return this._compiling;
  }

  @Mutation
  _setCompiling(value: string | null) {
    this._compiling = value;
  }

  @Action
  async getCompilingCache() {
    try {
      const {compiling} = await axios.get('api/compiling-cache');
      this.context.commit('_setCompiling', compiling)
    } catch (e) {
      userStore.setError(e)
    }
  }
}
