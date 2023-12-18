module dec ( 
    \count[0] , \count[1] , \count[2] , \count[3] , \count[4] , \count[5] ,
    \count[6] , \count[7] ,
    \selectp1[0] , \selectp1[1] , \selectp1[2] , \selectp1[3] ,
    \selectp1[4] , \selectp1[5] , \selectp1[6] , \selectp1[7] ,
    \selectp1[8] , \selectp1[9] , \selectp1[10] , \selectp1[11] ,
    \selectp1[12] , \selectp1[13] , \selectp1[14] , \selectp1[15] ,
    \selectp1[16] , \selectp1[17] , \selectp1[18] , \selectp1[19] ,
    \selectp1[20] , \selectp1[21] , \selectp1[22] , \selectp1[23] ,
    \selectp1[24] , \selectp1[25] , \selectp1[26] , \selectp1[27] ,
    \selectp1[28] , \selectp1[29] , \selectp1[30] , \selectp1[31]      );
  input  \count[0] , \count[1] , \count[2] , \count[3] , \count[4] ,
    \count[5] , \count[6] , \count[7] ;
  output \selectp1[0] , \selectp1[1] , \selectp1[2] , \selectp1[3] ,
    \selectp1[4] , \selectp1[5] , \selectp1[6] , \selectp1[7] ,
    \selectp1[8] , \selectp1[9] , \selectp1[10] , \selectp1[11] ,
    \selectp1[12] , \selectp1[13] , \selectp1[14] , \selectp1[15] ,
    \selectp1[16] , \selectp1[17] , \selectp1[18] , \selectp1[19] ,
    \selectp1[20] , \selectp1[21] , \selectp1[22] , \selectp1[23] ,
    \selectp1[24] , \selectp1[25] , \selectp1[26] , \selectp1[27] ,
    \selectp1[28] , \selectp1[29] , \selectp1[30] , \selectp1[31]  ;
  wire n265, n266, n267, n268, n269, n270, n272, n273, n275, n276, n278,
    n280, n281, n283, n284, n286, n288, n290, n291, n293, n295, n296, n298,
    n300, n302, n304, n306, n308, n309;
  assign n265 = ~\count[4]  & ~\count[5] ;
  assign n266 = ~\count[6]  & \count[7] ;
  assign n267 = n265 & n266;
  assign n268 = ~\count[0]  & ~\count[2] ;
  assign n269 = ~\count[1]  & ~\count[3] ;
  assign n270 = n268 & n269;
  assign \selectp1[0]  = n267 & n270;
  assign n272 = \count[0]  & ~\count[2] ;
  assign n273 = n269 & n272;
  assign \selectp1[1]  = n267 & n273;
  assign n275 = \count[1]  & ~\count[3] ;
  assign n276 = n268 & n275;
  assign \selectp1[2]  = n267 & n276;
  assign n278 = n272 & n275;
  assign \selectp1[3]  = n267 & n278;
  assign n280 = ~\count[0]  & \count[2] ;
  assign n281 = n269 & n280;
  assign \selectp1[4]  = n267 & n281;
  assign n283 = \count[0]  & \count[2] ;
  assign n284 = n269 & n283;
  assign \selectp1[5]  = n267 & n284;
  assign n286 = n275 & n280;
  assign \selectp1[6]  = n267 & n286;
  assign n288 = n275 & n283;
  assign \selectp1[7]  = n267 & n288;
  assign n290 = ~\count[1]  & \count[3] ;
  assign n291 = n268 & n290;
  assign \selectp1[8]  = n267 & n291;
  assign n293 = n272 & n290;
  assign \selectp1[9]  = n267 & n293;
  assign n295 = \count[1]  & \count[3] ;
  assign n296 = n268 & n295;
  assign \selectp1[10]  = n267 & n296;
  assign n298 = n272 & n295;
  assign \selectp1[11]  = n267 & n298;
  assign n300 = n280 & n290;
  assign \selectp1[12]  = n267 & n300;
  assign n302 = n283 & n290;
  assign \selectp1[13]  = n267 & n302;
  assign n304 = n280 & n295;
  assign \selectp1[14]  = n267 & n304;
  assign n306 = n283 & n295;
  assign \selectp1[15]  = n267 & n306;
  assign n308 = \count[4]  & ~\count[5] ;
  assign n309 = n266 & n308;
  assign \selectp1[16]  = n270 & n309;
  assign \selectp1[17]  = n273 & n309;
  assign \selectp1[18]  = n276 & n309;
  assign \selectp1[19]  = n278 & n309;
  assign \selectp1[20]  = n281 & n309;
  assign \selectp1[21]  = n284 & n309;
  assign \selectp1[22]  = n286 & n309;
  assign \selectp1[23]  = n288 & n309;
  assign \selectp1[24]  = n291 & n309;
  assign \selectp1[25]  = n293 & n309;
  assign \selectp1[26]  = n296 & n309;
  assign \selectp1[27]  = n298 & n309;
  assign \selectp1[28]  = n300 & n309;
  assign \selectp1[29]  = n302 & n309;
  assign \selectp1[30]  = n304 & n309;
  assign \selectp1[31]  = n306 & n309;
   
endmodule


