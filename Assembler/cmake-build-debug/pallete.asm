push 0
pop ax
outer_cycle:
push 0
pop bx

inner_cycle:

push 0
pop cx

vert_cycle:
push bx
push 1000
mul
push cx
push 8
mul
push ax
add
add
push 10
mul
push cx
add
pop dx
pix dx

inc cx
push 8
push cx
jne vert_cycle

inc bx
push 64
push bx
jne inner_cycle

inc ax
push ax
push 8
jne outer_cycle

draw
end