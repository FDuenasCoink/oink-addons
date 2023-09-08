import { PelicanoOptions, PelicanoObj } from "./interface";
import { join } from 'path';

/* eslint-disable @typescript-eslint/no-var-requires */
const addon = require('node-gyp-build')(join(__dirname, '..'));
console.log(addon);
export var Pelicano: {
  new (options: PelicanoOptions): PelicanoObj
} = addon.Pelicano;

