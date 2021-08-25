export default ({ app }) => {
  app.router.beforeEach((to, from, next) => {
    if (to.path[to.path.length - 1] === '/') {
      next();
    } else {
      next(`${to.path}/`);
    }
  });
};
