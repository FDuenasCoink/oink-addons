const { NV10 } = require('../dist');

const nv10 = new NV10({
  logPath: 'logs/nv10.log',
  logLevel: 1,
  maximumPorts: 10,
});

const connect = nv10.connect();
console.log(`Connect retorna: ${connect.statusCode} y ${connect.message}`);

let checkDevice = nv10.checkDevice();
console.log(`CheckDevice retorna: ${checkDevice.statusCode} y ${checkDevice.message}`);

let status = nv10.testStatus();
console.log(status);

if (connect.statusCode != 200) {
  process.exit(1);
}

const startReader = nv10.startReader();
console.log(`StartReader retorna: ${startReader.statusCode} y ${startReader.message}`);

const MAX_TIME = 10_000;
let total = 0;
let timmer = 0;
for (i = 0; i < 500; i++) {
  const bill = nv10.getBill();
  if (bill.statusCode === 302) continue;
  console.log(`GetBill retorna: ${bill.statusCode}, Bill: ${bill.bill}, ${bill.message}`);
  
  if (bill.statusCode >= 400 & bill.statusCode !== 508) {
    console.log(`Error. Saliendo...`);
    process.exit(1);
  }

  if (bill.statusCode === 508) {
    console.log('Error grave con el billete pero se debe guardar......');
    total += bill.bill;
    console.log('Sliendo...');
    process.exit(1);
  }

  if (bill.statusCode === 307) {
    timmer = Date.now();
    continue;
  }

  if (bill.statusCode === 308 || bill.statusCode === 312) {
    total += bill.bill;
    timmer = undefined;
    continue;
  }

  if (timmer) {
    const currTime = Date.now();
    const time = currTime - timmer;
    if (time < MAX_TIME) continue;
    const reject = nv10.reject();
    if (reject.statusCode === 206) {
      console.log('Se regreso el billete porque no se pudo apilar');
      continue;
    }
    console.log('Error, no se pudo regresar el billete, guardandolo y saliendo');
    process.exit(1);
  }
}

const stopReader = nv10.startReader();
console.log(`StopReader retorna: ${stopReader.statusCode} y ${stopReader.message}`);

console.log(`Total depositado: ${total}`);

checkDevice = nv10.checkDevice();
console.log(`CheckDevice retorna: ${checkDevice.statusCode} y ${checkDevice.message}`);

status = nv10.testStatus();
console.log(status);