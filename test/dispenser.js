const { Dispenser } = require('../dist');

const dispenser = new Dispenser({
  logPath: 'logs/dispenser.log',
  logLevel: 1,
  maximumPorts: 10,
  maxInitAttempts: 4,
  shortTime: 0,
  longTime: 3,
});

function getDispenserFlags() {
  const dispenserFlags = dispenser.getDispenserFlags();
  console.log(`GetDispenserFlags retorna - Hay una tarjeta atorada: ${dispenserFlags.rficCardInG}`);
  console.log(`GetDispenserFlags retorna - Caja de reciclaje llena: ${dispenserFlags.recyclingBoxF}`);
  console.log(`GetDispenserFlags retorna - Hay una tarjeta en puerta: ${dispenserFlags.cardInG}`);
  console.log(`GetDispenserFlags retorna - Hay tarjetas en el dispensador: ${dispenserFlags.cardsInD}`);
  console.log(`GetDispenserFlags retorna - El dispensador esta lleno: ${dispenserFlags.dispenserF}`);
  return dispenserFlags;
}

const connectRes = dispenser.connect();
console.log(`Connect retorna: ${connectRes.statusCode} y ${connectRes.message}`);

const errorCodes = [404, 500, 503];
if (errorCodes.includes(connectRes.statusCode)) {
  const checkDeviceRes = dispenser.checkDevice();
  console.log(`CheckDevice retorna: ${checkDeviceRes.statusCode} y ${checkDeviceRes.message}`)
  process.exit(1);
}

getDispenserFlags();

const dispenseCardRes = dispenser.dispenseCard();
console.log(`DispenseCard retorna: ${dispenseCardRes.statusCode} y ${dispenseCardRes.message}`);

const recycleStatus = [203, 301, 305];
if (recycleStatus.includes(dispenseCardRes.statusCode)) {
  const recycleRes = dispenser.recycleCard();
  console.log(`RecycleCard retorna: ${recycleRes.statusCode}  y ${recycleRes.message}`);
}

const endProcess = dispenser.endProcess();
console.log(`EndProcess retorna: ${endProcess.statusCode} y ${endProcess.message}`);

getDispenserFlags();

const status = dispenser.testStatus();
console.log(status);