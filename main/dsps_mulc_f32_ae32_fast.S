    .text
    .align  4
    .global dsps_mulc_f32_ae32_fast
    .type   dsps_mulc_f32_ae32_fast,@function

dsps_mulc_f32_ae32_fast: 
// input   - a2
// output   - a3
// len      - a4
// C        - a5
// step_in  - a6
// step_out - a7

    entry	a1, 64
    
    slli 	a6, a6, 4  	// a6 - step_in<<2
    slli 	a7, a7, 4  	// a7 - step_out<<2
	srli    a4, a4, 2  	// a4 - len>>2

    wfr		f0, a5		// a5 - load to the f0

    loopnez a4, loop_end_mulc_f32_ae32_fast
        lsi      f1, a2, 0      // load input_ptr+0 to f1
		lsi      f2, a2, 4      // load input_ptr+1 to f2
		lsi      f3, a2, 8      // load input_ptr+2 to f3
		lsi      f4, a2, 12     // load input_ptr+3 to f4

        mul.s    f5, f1, f0     // f2 = f1 + f0
		mul.s    f6, f2, f0     // f6 = f2 + f0
		mul.s    f7, f3, f0     // f7 = f3 + f0
		mul.s    f8, f4, f0     // f8 = f4 + f0

        ssi	     f5, a3, 0      // store f2 to output_ptr+0
		ssi	     f6, a3, 4      // store f6 to output_ptr+1
		ssi	     f7, a3, 8      // store f7 to output_ptr+2
		ssi	     f8, a3, 12     // store f8 to output_ptr+3

        add.n    a2, a2, a6     // input_ptr+=step_in;
        add.n    a3, a3, a7     // output_ptr+=step_out;

loop_end_mulc_f32_ae32_fast:

    movi.n	a2, 0 // return status ESP_OK
    retw.n