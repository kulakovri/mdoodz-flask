export class Logger {
  static log(...args): void {
    // eslint-disable-next-line @typescript-eslint/ban-ts-comment
    // @ts-ignore
    if (process.env.NODE_ENV !== 'production' || window.isDebug) {
      console.log(...args);
    }
  }
}
