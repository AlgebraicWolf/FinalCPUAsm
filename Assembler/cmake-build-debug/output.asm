call main
end

main:
push 8
push 2
mul
push [0]
pop cx
pop [cx+2]
push [0]
pop cx
push [cx+2]
push [0]
pop cx
pop [cx+1]
push [0]
pop cx
push [cx+1]
out
pop [cx+1]
ret

