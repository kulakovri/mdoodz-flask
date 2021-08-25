import { Action, Module, Mutation, VuexModule } from 'vuex-module-decorators';

const errorTimoutInMilliSeconds = 5000;

@Module({
  name: 'modules/user',
  stateFactory: true,
  namespaced: true,
})
export default class User extends VuexModule {
  private _error: string | null = null;

  get error() {
    return this._error || '';
  }

  @Action
  resetState(): void {
    this.context.commit('_resetState');
  }

  @Mutation
  _setError(value: string | null) {
    this._error = value;
  }

  @Action
  setError(value: any) {
    // eslint-disable-next-line no-prototype-builtins
    if (typeof value === 'object' && value.hasOwnProperty('message')) {
      this.context.commit('_setError', value.message);
    } else {
      this.context.commit('_setError', value);
    }
    setTimeout(() => {
      this.context.commit('_setError', null);
    }, errorTimoutInMilliSeconds);
  }
}
