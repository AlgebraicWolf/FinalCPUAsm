circle:
push 0
push 0
pop ax
pop bx

outer_cycle:
inner_cycle:

push 32
push ax
sub
push 32
sub
push ax
mul

push 32
push bx
sub
push 32
push bx
sub
mul

add

push 900
push 1200
ja continue
jb continue

push ax
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

continue:
inc bx
push bx
push 16
jne inner_cycle

inc ax
push ax
push 16
jne outer_cycle