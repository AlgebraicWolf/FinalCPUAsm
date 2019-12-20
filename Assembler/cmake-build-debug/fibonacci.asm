in
pop ax

call fib
push bx
out
end

fact:
push ax
push 0
je ret_1
pop dx
push 1
je ret_1
pop dx
push ax
push -1
add
pop ax
call fib
push bx
mul
pop bx
ret

ret_1:
push 2
pop ax
push 1
pop bx
pop dx
pop dx
ret