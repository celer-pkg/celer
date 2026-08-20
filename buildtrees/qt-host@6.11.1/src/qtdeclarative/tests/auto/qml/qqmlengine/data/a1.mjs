import { read_from_b1 } from "./b1.mjs";
export let data_a1;
export function process_a1() {
    data_a1 = "DATA_IN_A1";
    console.error("In a1 -", data_a1);
    read_from_b1();
}
