open Core.Std

(* 8 registers *)

let orthography r v =
  ((0b1111 land 13) lsl (32-4)) lor
    ((0b111 land r) lsl (32-4-3)) lor
      (0x1FFFFFF land v)

let prepare op a b c =
  ((0b1111 land op) lsl (32-4)) lor
    ((0b111 land a) lsl 6) lor
      ((0b111 land b) lsl 3) lor
	(0b111 land c)
	  
let conditional_move a b c = prepare 0 a b c
let array_index a b c = prepare 1 a b c
let array_amendment a b c = prepare 2 a b c
let add a b c = prepare 3 a b c
let multiply a b c = prepare 4 a b c
let division a b c = prepare 5 a b c
let notand a b c = prepare 6 a b c

let halt = prepare 7 0 0 0
let allocation b c = prepare 8 0 b c
let abandonment c = prepare 9 0 0 c
let output c = prepare 10 0 0 c
			     
let load b c = prepare 12 0 b c
			     

let output_word ch wd =
  Out_channel.output_byte ch (0xFF land (wd lsr 24));
  Out_channel.output_byte ch (0xFF land (wd lsr 16));
  Out_channel.output_byte ch (0xFF land (wd lsr 8));
  Out_channel.output_byte ch (0xFF land (wd lsr 0))

	      
	      
