open Core.Std

module Um = Emit
       
(* To compile: corebuild brain.byte *)
       
let position = ref 0
let places = Stack.create ()

let swap_in c = Um.output_word c (Um.array_amendment 1 3 0)
let swap_out c = Um.output_word c (Um.array_index 0 1 3)
let process_char c ch =
  match ch with
  | '+' -> Um.output_word c (Um.add 0 0 1); 1
  | '-' -> Um.output_word c (Um.add 0 0 2); 1
  | '>' -> swap_in c; Um.output_word c (Um.add 3 3 1); swap_out c; 3
  | '<' -> swap_in c; Um.output_word c (Um.add 3 3 2); swap_out c; 3
  | '.' -> Um.output_word c (Um.output 0); 1
  | '[' -> Stack.push places (!position); 0
  | ']' -> (match Stack.pop places with
	    | Some jump ->
	       (Um.output_word c (Um.orthography 6 (!position + 4));
		Um.output_word c (Um.orthography 5 jump); 
		Um.output_word c (Um.conditional_move 6 5 0);
		Um.output_word c (Um.load 4 6);
		4)
	    | None -> (* ERROR *) 0)
  | _ -> 0
	   
let () =
  let c = Out_channel.create "emit.umz" ~binary:true in
  
  (* set up the constants 1 [reg 1] and -1 [reg 2] *)
  Um.output_word c (Um.orthography 1 0x01);
  Um.output_word c (Um.orthography 2 0x10001);
  Um.output_word c (Um.orthography 3 0x0FFFF);
  Um.output_word c (Um.multiply 2 2 3);
  
  (* allocate a platter with lots of space for the cells *)
  Um.output_word c (Um.allocation 1 3);
  
  (* set register 3 to the start of that array *)
  Um.output_word c (Um.orthography 3 0);

  position := 6;
  
  let rec loop () =
    match In_channel.input_char In_channel.stdin with
    | None -> ()
    | Some ch -> position := !position + process_char c ch; loop ()
  in loop ()
