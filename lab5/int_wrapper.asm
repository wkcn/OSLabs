[bits 16]
;给C++ 函数做中断包装
;_func -> func
%macro INT_WRAPPER 1 
[global %1]
[extern _%1]
%1:
	pushad
	sti
	call %1
	popad
	iret
%endmacro

;INT_WRAPPER int_33h
;INT_WRAPPER int_34h
;INT_WRAPPER int_35h
;INT_WRAPPER int_36h
