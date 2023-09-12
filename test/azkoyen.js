const { Azkoyen } = require('../dist');

const azkoyen = new Azkoyen({
  maxCritical: 4,
  warnToCritical: 10,
  maximumPorts: 10,
  logLevel: 1,
  logPath: 'logs/pelicano.log',
});

const connect = azkoyen.connect();
console.log(`Connect retorna: ${connect.statusCode} y ${connect.message}`);

let checkDevice = azkoyen.checkDevice();
console.log(`CheckDevice retorna: ${checkDevice.statusCode} y ${checkDevice.message}`);

if (checkDevice.statusCode !== 200 && checkDevice.statusCode !== 301) {
  console.log('Error detectado, no se puede continuar');
  process.exit(1);
}

const startReader = azkoyen.startReader();
console.log(`StartReader retorna: ${startReader.statusCode} y ${startReader.message}`);

let total = 0;
for (let i = 0; i < 400; i++) {
  const coin = azkoyen.getCoin();
  if (coin.statusCode === 303) continue;
  console.log(`GetCoin retorna. StatusCode: ${coin.statusCode} Event: ${coin.event} Coin: ${coin.coin} Message: ${coin.message} Remaining: ${coin.remaining}`);
  if (coin.remaining > 1) {
    const coinLost = azkoyen.getLostCoins();
    console.log({ coinLost });
    Object.entries(coinLost).forEach(([coin, quantity]) => {
      const coinTotal = Number(coin) * quantity;
      total += coinTotal;
    });
  }
  if (coin.statusCode === 402 || coin.statusCode === 403) {
    console.log('Error detectado se detiene el loop');
    break;
  }
  total += coin.coin;
}

checkDevice = azkoyen.checkDevice();
console.log(`CheckDevice retorna: ${checkDevice.statusCode} y ${checkDevice.message}`);

const stopReader = azkoyen.stopReader();
console.log(`StopReader retorna: ${stopReader.statusCode} y ${stopReader.message}`);

console.log(`Total depositado: $${total}`);

const status = azkoyen.testStatus();
console.log({ status });