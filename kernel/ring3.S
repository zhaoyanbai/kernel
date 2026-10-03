/*
 * ------------------------------------------------------------------------
 *   File Name: ring3.S
 *      Author: Zhao Yanbai
 *              2025-12-30 22:12:25 Tuesday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#define ASM
#include <syscall.h>

.extern ring3_c_text

# a allocatable 可分配，表示这个节在程序运行时需要加载到内存中
# x executable  可执行，表示这个节包含可执行的代码
# w writable    可写，通常用于数据节
# r read-only   只读
# M mergeable   可合并相同内容
# S strings     包含以null结尾的字符串
# G group       属于某个节组
#
# @progbits     包含了程序数据  目标文件中实际包含数据/代码，如.text、.data节
# @nobits       不包含程序数据  目标文件中不占空间，运行时分配零初始化内存，如.bss节
.section .ring3.entry, "ax", @progbits
.align 0x1000
ring3_entry:
    nop
    nop
    nop

    pushl   $5      #参数5
    pushl   $4      #参数4
    pushl   $3      #参数3
    pushl   $2      #参数2
    pushl   $1      #参数1
    pushl   $SYSC_TEST          # 系统调用号
    call    ring3_syscall
    addl    $0x18, %esp

    pushl   $5      #参数5
    pushl   $4      #参数4
    pushl   $3      #参数3
    pushl   $2      #参数2
    pushl   $1      #参数1
    pushl   $SYSC_RAND          # 系统调用号
    call    ring3_syscall
    addl    $0x18, %esp


    movl    %eax, %ecx
    andl    $0x000000FF, %ecx
    pushl   $5      #参数5
    pushl   $4      #参数4
    pushl   $3      #参数3
    pushl   $2      #参数2
    pushl   %ecx    #参数1
    pushl   $SYSC_WAIT          # 系统调用号
    call    ring3_syscall
    addl    $0x18, %esp


    call    ring3_c_text


    # 纯消耗CPU时间
    movl    $0xFF0F0F0F, %ecx
1:
    loop    1b


    nop
    nop
    jmp ring3_entry

#
# System V ABI
# 参数: 从右向左压栈，调用者清栈
# 返回值: eax; 64位在edx:eax
# 调用者保存: eax ecx edx
# 被调用者保存: ebx esi edi ebp esp
#

#
# 输入:
#   eax: 系统调用号
#   ebx: 参数1
#   ecx: 参数2
#   edx: 参数3
#   esi: 参数4
#   edi: 参数5
# 输出:
#   eax: 返回码
#
ring3_syscall:
    # 实现一个五个参数的完全版本，后续在lib/syscall.c里参数实现
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    pushl   %ebp

    # sysexit页 ret 时会得到的返回地址
    pushl   $1f

    movl    %esp, %ebp

    # 根据栈内容，准备寄存器参数
    movl    0x18(%esp), %eax
    movl    0x1C(%esp), %ebx
    movl    0x20(%esp), %ecx
    movl    0x24(%esp), %edx
    movl    0x28(%esp), %esi
    movl    0x2C(%esp), %edi

    # 系统调用能保证除 eax ecx edx 外的寄存器都不会被修改
    sysenter
1:
    # 此处不用恢复esp
    # 因为esp已经通过ebp传进系统调用
    # 系统调用返回前已经根据ebp设置好了esp

    #
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx
    ret
