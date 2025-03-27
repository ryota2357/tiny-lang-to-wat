(module
  (func $write_str (import "imports" "write_str") (param i32) (param i32))
  (func $write_f64 (import "imports" "write_f64") (param f64))
  (func $exit      (import "imports" "exit") (param i32))
  (memory 16)
  (export "memory" (memory 0))
  (data (i32.const 0) "print$a0fibnUnexpected number\nUnexpected function\nerror: `` is not defined\n")
  (global $i32_stash (mut i32) (i32.const 0))
  (global $env_base i32 (i32.const 120))
  (global $env_top (mut i32) (i32.const 120))
  (global $value_f64 (mut f64) (f64.const 0))
  (global $value_fun (mut i32) (i32.const 0))
  (type $return_i32 (func (result i32)))
  (table 2 funcref)
  (elem (i32.const 0) $f0 $f1)
(func $f0 (result i32)
i64.const 21474836483  ;; $a0
call $get_value
call $typecheck_f64
call $write_f64
i32.const 0
)
(func $f1 (result i32)
i64.const 47244640257  ;; n
i64.const 21474836483  ;; $a0
call $get_value
call $set_value
i64.const 47244640257  ;; n
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
i64.const 47244640257  ;; n
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
i64.const 34359738371  ;; fib
call $get_value
call $typecheck_fun
call_indirect (type $return_i32)
global.set $i32_stash
global.set $env_top
global.get $i32_stash
call $typecheck_f64
global.get $env_top
i64.const 21474836483  ;; $a0
i64.const 47244640257  ;; n
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
i64.const 34359738371  ;; fib
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
    (call $write_str (i32.const 50) (i32.const 8)) ;; "error: `"
    (call $write_str
          (i32.wrap_i64 (i64.shr_u (local.get $text_id) (i64.const 32)))
          (i32.wrap_i64 (local.get $text_id)))
    (call $write_str (i32.const 58) (i32.const 17)) ;; "` is not defined"
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
        (call $write_str (i32.const 30) (i32.const 20)) ;; "Unexpected function"
        (call $exit (i32.const 1))
        (unreachable)))
    (return (global.get $value_f64)))
  (func $typecheck_fun (param $flag i32) (result i32)
    (if
      (i32.eqz (local.get $flag))
      (then
        (call $write_str (i32.const 12) (i32.const 18)) ;; "Unexpected number"
        (call $exit (i32.const 1))
        (unreachable)))
    (return (global.get $value_fun)))
  (func (export "main")
i64.const 5  ;; print
i32.const 0
global.set $value_fun
i32.const 1
call $set_value
i64.const 34359738371  ;; fib
i32.const 1
global.set $value_fun
i32.const 1
call $set_value
global.get $env_top
i64.const 21474836483  ;; $a0
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 1.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 2.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 3.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 4.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 5.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 6.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 7.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 8.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 9.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
global.get $env_top
i64.const 21474836483  ;; $a0
f64.const 30.000000
global.set $value_f64
i32.const 0
call $set_value
i64.const 34359738371  ;; fib
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
))
