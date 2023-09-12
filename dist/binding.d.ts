import { DispenserObj, DispenserOptions } from "./dispenser.interface";
import { ValidatorOptions, ValidatorObj } from "./interface";
import { NV10Obj, NV10Options } from "./nv10.interface";
export declare var Pelicano: {
    new (options: ValidatorOptions): ValidatorObj;
};
export declare var Azkoyen: {
    new (options: ValidatorOptions): ValidatorObj;
};
export declare var Dispenser: {
    new (options: DispenserOptions): DispenserObj;
};
export declare var NV10: {
    new (options: NV10Options): NV10Obj;
};
