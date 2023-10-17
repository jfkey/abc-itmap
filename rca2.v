module rca2 (a0, b0, a1, b1, s0, s1, s2);
//-------------Input Ports Declarations-----------------------------
input a0, b0, a1, b1;
//-------------Output Ports Declarations-----------------------------
// output s0, s1, s2;
output s1;
//-------------Wires-----------------------------------------------
wire c0, c1;
//-------------Logic-----------------------------------------------
// assign s0 = a0 ^ b0 ;
// assign c0 = a0 & b0 ;
// assign s1 = a1 ^ b1 ^ c0;
//assign s2 = (a1 & b1) | (c0 & (a1 ^ b1));
assign c0 = a0 ^ b0 ;
assign c1 = a1 & b1 ;
assign s1 = c0 & c1 ;
endmodule

