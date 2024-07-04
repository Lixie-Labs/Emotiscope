    .text
    .align  4
    .global dsps_mul_f32_ae32_fast
    .type   dsps_mul_f32_ae32_fast,@function

dsps_mul_f32_ae32_fast: 
// input1   - a2
// input2   - a3
// output   - a4
// len      - a5
// step     - a6

    entry	a1, 64

    slli 	a6, a6, 4  	// a6 - step<<5 (1 --> 16)

	srli 	a5, a5, 2  	// a4 - len>>2

    loopnez a5, loop_end_mul_f32_ae32_fast
		lsi      f0, a2, 0      // load input1_ptr+0 to f0
		lsi      f1, a2, 4      // load input1_ptr+1 to f1
		lsi      f2, a2, 8      // load input1_ptr+2 to f2
		lsi      f3, a2, 12     // load input1_ptr+3 to f3

		lsi      f4, a3, 0      // load input2_ptr+0 to f4
		lsi      f5, a3, 4      // load input2_ptr+1 to f5
		lsi      f6, a3, 8      // load input2_ptr+2 to f6
		lsi      f7, a3, 12     // load input2_ptr+3 to f7

        mul.s    f9,  f0, f4	 // f9  = f0 * f4
		mul.s    f10, f1, f5	 // f10 = f1 * f5
		mul.s    f11, f2, f6	 // f11 = f2 * f6
		mul.s    f12, f3, f7	 // f12 = f3 * f7

        ssi	     f9,  a4, 0      // store f9  to output_ptr+0
		ssi	     f10, a4, 4      // store f10 to output_ptr+1
		ssi	     f11, a4, 8      // store f11 to output_ptr+2
		ssi	     f12, a4, 12     // store f12 to output_ptr+3

        add.n    a2, a2, a6     // input_ptr1+=step;
		add.n    a3, a3, a6     // input_ptr2+=step;
		add.n    a4, a4, a6     // output_ptr+=step;
				
loop_end_mul_f32_ae32_fast:

    movi.n	a2, 0 // return status ESP_OK
    retw.n