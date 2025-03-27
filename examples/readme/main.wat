(module
  (func $write_str (import "imports" "write_str") (param i32) (param i32))
  (func $write_f64 (import "imports" "write_f64") (param f64))
  (func $exit      (import "imports" "exit") (param i32))
  (memory 16)
  (export "memory" (memory 0))
  (data (i32.const 0) "print$a0asumb$a1fibnprint_123456789xnot_call_should_callyfnfnresundefUnexpected number\nUnexpected function\nerror: `` is not defined\n")
  (global $i32_stash (mut i32) (i32.const 0))
  (global $env_base i32 (i32.const 168))
  (global $env_top (mut i32) (i32.const 168))
  (global $value_f64 (mut f64) (f64.const 0))
  (global $value_fun (mut i32) (i32.const 0))
  (type $return_i32 (func (result i32)))
  (table 8 funcref)
  (elem (i32.const 0) $f0 $f1 $f2 $f3 $f4 $f5 $f6 $f7)
(func $f0 (result i32)
i64.const 21474836483  ;; $a0
call $get_value
call $typecheck_f64
call $write_f64
i32.const 0
)
(func $f1 (result i32)
i64.const 34359738369  ;; a
i64.const 21474836483  ;; $a0
call $get_value
call $set_value
i64.const 51539607553  ;; b
i64.const 55834574851  ;; $a1
call $get_value
call $set_value
i64.const 34359738369  ;; a
call $get_value
call $typecheck_f64
i64.const 51539607553  ;; b
call $get_value
call $typecheck_f64
f64.add
global.set $value_f64
i32.const 0
)
(func $f2 (result i32)
i64.const 81604378625  ;; n
i64.const 21474836483  ;; $a0
call $get_value
call $set_value
i64.const 81604378625  ;; n
call $get_value
call $typecheck_f64
f64.const 3.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.lt
f64.convert_i32_u
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.abs
i64.reinterpret_f64
i64.eqz
i32.eqz
if
f64.const 1.000000
global.set $value_f64
i32.const 0
global.set $i32_stash
else
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 81604378625  ;; n
call $get_value
call $typecheck_f64
f64.const 1.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.sub
global.set $value_f64
i32.const 0
call $set_value
i64.const 68719476739  ;; fib
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
call $typecheck_f64
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 81604378625  ;; n
call $get_value
call $typecheck_f64
f64.const 2.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.sub
global.set $value_f64
i32.const 0
call $set_value
i64.const 68719476739  ;; fib
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
call $typecheck_f64
f64.add
global.set $value_f64
i32.const 0
global.set $i32_stash
end
global.get $i32_stash
)
(func $f3 (result i32)
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 123456789.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
)
(func $f4 (result i32)
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 1029384756.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
)
(func $f5 (result i32)
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 0.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
)
(func $f6 (result i32)
i64.const 261993005059  ;; res
i32.const 7
global.set $value_fun
i32.const 1
call $set_value
i64.const 261993005059  ;; res
call $get_value
)
(func $f7 (result i32)
global.get $env_top
i64.const 85899345935  ;; print_123456789
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
)
  (func $get_value (param $text_id i64) (result i32)
    (local $ptr i32)
    (local.set $ptr (global.get $env_top))
    (block $break
      (loop $continue
        (local.set
          $ptr
          (i32.sub
            (local.get $ptr)
            (i32.const 24)))
        (i32.lt_u
          (local.get $ptr)
          (global.get $env_base))
        (br_if $break)
        (i64.ne
          (i64.load (local.get $ptr))
          (local.get $text_id))
        (br_if $continue)
        (if
          (i32.eqz (; flag == 0 ? f64 : fun ;)
            (i32.load
              (i32.add
                (i32.const 8)
                (local.get $ptr))))
          (then
            (global.set
              $value_f64
              (f64.load
                (i32.add
                  (local.get $ptr)
                  (i32.const 16))))
            (return (i32.const 0)))
          (else
            (global.set
              $value_fun
              (i32.load
                (i32.add
                  (local.get $ptr)
                  (i32.const 20))))
            (return (i32.const 1))))))
    (call $write_str (i32.const 107) (i32.const 8)) ;; "error: `"
    (call $write_str
          (i32.wrap_i64 (i64.shr_u (local.get $text_id) (i64.const 32)))
          (i32.wrap_i64 (local.get $text_id)))
    (call $write_str (i32.const 115) (i32.const 17)) ;; "` is not defined"
    (call $exit (i32.const 1))
    (unreachable))
  (func $set_value (param $text_id i64) (param $flag i32)
    (i64.store (; store text_id ;)
      (global.get $env_top)
      (local.get $text_id))
    (if
      (local.get $flag)
      (then (; fun ;)
        (i32.store (; store flag ;)
          (i32.add (global.get $env_top) (i32.const 8))
          (i32.const 1))
        (i32.store (; store funcref;)
          (i32.add (global.get $env_top) (i32.const 20))
          (global.get $value_fun)))
      (else (; f64 ;)
        (i32.store (; store flag ;)
          (i32.add (global.get $env_top) (i32.const 8))
          (i32.const 0))
        (f64.store (; store f64;)
          (i32.add (global.get $env_top) (i32.const 16))
          (global.get $value_f64))))
    (global.set $env_top (; update env_top ;)
      (i32.add
        (global.get $env_top)
        (i32.const 24))))
  (func $typecheck_f64 (param $flag i32) (result f64)
    (if
      (local.get $flag)
      (then
        (call $write_str (i32.const 87) (i32.const 20)) ;; "Unexpected function"
        (call $exit (i32.const 1))
        (unreachable)))
    (return (global.get $value_f64)))
  (func $typecheck_fun (param $flag i32) (result i32)
    (if
      (i32.eqz (local.get $flag))
      (then
        (call $write_str (i32.const 69) (i32.const 18)) ;; "Unexpected number"
        (call $exit (i32.const 1))
        (unreachable)))
    (return (global.get $value_fun)))
  (func (export "main")
i64.const 5  ;; print
i32.const 0
global.set $value_fun
i32.const 1
call $set_value
i64.const 34359738369  ;; a
f64.const 1.000000
global.set $value_f64
i32.const 0
call $set_value
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 34359738369  ;; a
call $get_value
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
i64.const 38654705667  ;; sum
i32.const 1
global.set $value_fun
i32.const 1
call $set_value
global.get $env_top
i64.const 21474836483  ;; $a0
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 10.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 55834574851  ;; $a1
f64.const 20.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 38654705667  ;; sum
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 34359738369  ;; a
call $get_value
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
i64.const 68719476739  ;; fib
i32.const 2
global.set $value_fun
i32.const 1
call $set_value
global.get $env_top
i64.const 21474836483  ;; $a0
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 15.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 68719476739  ;; fib
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
i64.const 85899345935  ;; print_123456789
i32.const 3
global.set $value_fun
i32.const 1
call $set_value
global.get $env_top
i64.const 85899345935  ;; print_123456789
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
i64.const 150323855361  ;; x
f64.const 1.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.const 2.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.lt
f64.convert_i32_u
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.abs
i64.reinterpret_f64
i64.eqz
i32.eqz
if
f64.const 7.000000
global.set $value_f64
i32.const 0
global.set $i32_stash
else
f64.const 13.000000
global.set $value_f64
i32.const 0
global.set $i32_stash
end
global.get $i32_stash
call $set_value
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 150323855361  ;; x
call $get_value
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
i64.const 154618822664  ;; not_call
i32.const 4
global.set $value_fun
i32.const 1
call $set_value
i64.const 188978561025  ;; _
f64.const 0.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.abs
i64.reinterpret_f64
i64.eqz
i32.eqz
if
global.get $env_top
i64.const 154618822664  ;; not_call
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
global.set $i32_stash
else
f64.const 0
global.set $value_f64
i32.const 0
global.set $i32_stash
end
global.get $i32_stash
call $set_value
i64.const 188978561025  ;; _
f64.const 1.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.abs
i64.reinterpret_f64
i64.eqz
i32.eqz
if
f64.const 1
global.set $value_f64
i32.const 0
global.set $i32_stash
else
global.get $env_top
i64.const 154618822664  ;; not_call
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
global.set $i32_stash
end
global.get $i32_stash
call $set_value
i64.const 193273528331  ;; should_call
i32.const 5
global.set $value_fun
i32.const 1
call $set_value
i64.const 188978561025  ;; _
f64.const 1.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.abs
i64.reinterpret_f64
i64.eqz
i32.eqz
if
global.get $env_top
i64.const 193273528331  ;; should_call
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
global.set $i32_stash
else
f64.const 0
global.set $value_f64
i32.const 0
global.set $i32_stash
end
global.get $i32_stash
call $set_value
i64.const 188978561025  ;; _
f64.const 0.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.abs
i64.reinterpret_f64
i64.eqz
i32.eqz
if
f64.const 1
global.set $value_f64
i32.const 0
global.set $i32_stash
else
global.get $env_top
i64.const 193273528331  ;; should_call
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
global.set $i32_stash
end
global.get $i32_stash
call $set_value
i64.const 150323855361  ;; x
f64.const 3.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 240518168577  ;; y
f64.const 1.200000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.const -1
f64.mul
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.const 2.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.mul
global.set $value_f64
i32.const 0
call $set_value
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 150323855361  ;; x
call $get_value
call $typecheck_f64
f64.const 2.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.div
global.set $value_f64
i32.const 0
call $typecheck_f64
i64.const 240518168577  ;; y
call $get_value
call $typecheck_f64
f64.const 3.000000
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.mul
global.set $value_f64
i32.const 0
call $typecheck_f64
f64.add
global.set $value_f64
i32.const 0
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
i64.const 244813135876  ;; fnfn
i32.const 6
global.set $value_fun
i32.const 1
call $set_value
global.get $env_top
global.get $env_top
i64.const 244813135876  ;; fnfn
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 274877906949  ;; undef
call $get_value
call $set_value
i64.const 5  ;; print
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
drop
))
