org 7C00h

jmp start

fio: db 'Sorokin Dmitriy Andreevich',0
group: db 'NMT-323901',0

start:
;set video mode
mov ah, 00h
mov al, 13h
int 10h

; Выводим ФИО
mov si, fio
mov dl, 7
mov dh, 23
loop1:
mov bh, 0
mov ah, 02h
int 10h
mov al, [cs:si]
cmp al, 0
je next1
mov bl, 01h
mov bh, 0
mov ah, 0Eh
int 10h
inc dl
inc si
jmp loop1
next1:

; Выводим группу
mov si, group
mov dl, 15
mov dh, 24
loop2:
mov bh, 0
mov ah, 02h
int 10h
mov al, [cs:si]
cmp al, 0
je next2
mov bl, 01h
mov bh, 0
mov ah, 0Eh
int 10h
inc dl
inc si
jmp loop2
next2:

mov si, positions
loop3:
;draw pixels
mov ah, 0ch
mov bh, 0
mov cx, [cs:si]
cmp cx, 0
je next3
add cx, 160
add si, 2
mov dx, [cs:si]
add dx, 1
mov al, 01h
int 10h
add si, 2
jmp loop3

next3:

MOV CX, 0FH
MOV DX, 4240H
MOV AH, 86H
INT 15H
jmp exit

positions:
dw 5, 1
dw 6, 1
dw 7, 1
dw 8, 1
dw 9, 1
dw 10, 1
dw 11, 2
dw 12, 2
dw 13, 3
dw 14, 4
dw 14, 5
dw 14, 6
dw 13, 7
dw 12, 8
dw 11, 8
dw 10, 9
dw 9, 9
dw 8, 9
dw 7, 9
dw 6, 9
dw 5, 9
dw 4, 8
dw 3, 8
dw 2, 7
dw 1, 6
dw 1, 5
dw 1, 4
dw 2, 3
dw 3, 2
dw 4, 2
dw 0

exit:

db 510-($-$$) dup 0
dw 0AA55h
