import { ValidatorOptions, ValidatorObj } from "./interface";
import { join } from 'path';

/* eslint-disable @typescript-eslint/no-var-requires */
const addons = require('node-gyp-build')(join(__dirname, '..'));

export var Pelicano: {
  new (options: ValidatorOptions): ValidatorObj
} = addons.Pelicano;

export var Azkoyen: {
  new (options: ValidatorOptions): ValidatorObj
} = addons.Azkoyen;

