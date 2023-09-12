import { DispenserObj, DispenserOptions } from "./dispenser.interface";
import { ValidatorOptions, ValidatorObj } from "./interface";
import { join } from 'path';
import { NV10Obj, NV10Options } from "./nv10.interface";

/* eslint-disable @typescript-eslint/no-var-requires */
const addons = require('node-gyp-build')(join(__dirname, '..'));

export var Pelicano: {
  new (options: ValidatorOptions): ValidatorObj
} = addons.Pelicano;

export var Azkoyen: {
  new (options: ValidatorOptions): ValidatorObj
} = addons.Azkoyen;

export var Dispenser: {
  new (options: DispenserOptions): DispenserObj
} = addons.Dispenser;

export var NV10: {
  new (options: NV10Options): NV10Obj
} = addons.NV10;