const { Dispenser } = require('../dist');

const dispenser = new Dispenser({
  logPath: 'logs/dispenser.log',
  logLevel: 1,
  maximumPorts: 10,
  maxInitAttempts: 4,
  shortTime: 0,
  longTime: 3,
});

const result = dispenser.dispenseCard();
console.log(result);
const stop = dispenser.onDispense((status) => {
  console.log({ status });
});

setTimeout(() => {
  console.log('Finalizando!');
  stop();
}, 2_000);