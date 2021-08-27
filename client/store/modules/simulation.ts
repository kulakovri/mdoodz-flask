import {Module, VuexModule} from "vuex-module-decorators";

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
  private _currentSimulationName: SimulationName | null = null;
}
