push 0
pop ax
outer_cycle:
push 10
pop bx

inner_cycle:

call dist

push cx
push 800

ja black

push 900
push cx

ja black
jbe white

cont:
inc bx
push 32
push bx
jne inner_cycle

inc ax
push ax
push 16
jne outer_cycle

draw
end

dist:
push 33
push ax
sub
pop cx
push cx
push cx
mul
push 33
push bx
sub
pop cx
push cx
push cx
mul
add
pop cx
ret

black:
jmp cont

white:
push bx
push 1000
mul
push ax
add
push 10
mul
push 2
add
pop dx
pix dx

push bx
push 63
sub
push 1000
mul
push ax
add
push 10
mul
push 2
add
pop dx
pix dx

push bx
push 1000
mul
push ax
push 63
sub
add
push 10
mul
push 2
add
pop dx
pix dx

push bx
push 63
sub
push 1000
mul
push ax
push 63
sub
add
push 10
mul
push 2
add
pop dx
pix dx

push ax
push 1000
mul
push bx
add
push 10
mul
push 2
add
pop dx
pix dx

push ax
push 63
sub
push 1000
mul
push bx
add
push 10
mul
push 2
add
pop dx
pix dx

push ax
push 1000
mul
push bx
push 63
sub
add
push 10
mul
push 2
add
pop dx
pix dx

push ax
push 63
sub
push 1000
mul
push bx
push 63
sub
add
push 10
mul
push 2
add
pop dx
pix dx

jmp cont