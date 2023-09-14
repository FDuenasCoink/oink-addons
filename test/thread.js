const { Azkoyen } = require('../dist');

const azkoyen = new Azkoyen({
  maxCritical: 4,
  warnToCritical: 10,
  maximumPorts: 10,
  logLevel: 1,
  logPath: 'logs/pelicano.log',
});


function run() {
  return new Promise(resolve => {
    const stopFn = azkoyen.onCoin((message) => {
      console.log({ message });
    });
    setTimeout(() => {
      console.log('voy a parar todo!');
      stopFn();
      resolve();
    }, 1_000);
  });
}

function delay() {
  return new Promise(resolve => {
    setTimeout(resolve, 0);
  }); 
}

(async () => {
  await run();
})();
