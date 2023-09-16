import { AzkoyenOptions, IAzkoyen } from "./azkoyen.interface";
import { IDispenser, DispenserOptions } from "./dispenser.interface";
import { INV10, NV10Options } from "./nv10.interface";
import { IPelicano, PelicanoOptions } from "./pelicano.interface";
import { join } from 'path';

/* eslint-disable @typescript-eslint/no-var-requires */
const addons = require('node-gyp-build')(join(__dirname, '..'));

export var Pelicano: {
  new (options: PelicanoOptions): IPelicano
} = addons.Pelicano;

export var Azkoyen: {
  new (options: AzkoyenOptions): IAzkoyen
} = addons.Azkoyen;

export var Dispenser: {
  new (options: DispenserOptions): IDispenser
} = addons.Dispenser;

export var NV10: {
  new (options: NV10Options): INV10
} = addons.NV10;