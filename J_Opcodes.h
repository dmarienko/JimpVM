#ifndef __J_OPCODES_H
#define __J_OPCODES_H

/* Standard Opcodes */
#define OP_nop             0
#define OP_aconst_null     1
#define OP_iconst_m1       2
#define OP_iconst_0        3
#define OP_iconst_1        4
#define OP_iconst_2        5
#define OP_iconst_3        6
#define OP_iconst_4        7
#define OP_iconst_5        8
#define OP_lconst_0        9
#define OP_lconst_1        10
#define OP_fconst_0        11
#define OP_fconst_1        12
#define OP_fconst_2        13
#define OP_dconst_0        14
#define OP_dconst_1        15
#define OP_bipush          16
#define OP_sipush          17
#define OP_ldc             18
#define OP_ldc_w           19
#define OP_ldc2_w          20
#define OP_iload           21
#define OP_lload           22
#define OP_fload           23
#define OP_dload           24
#define OP_aload           25
#define OP_iload_0         26
#define OP_iload_1         27
#define OP_iload_2         28
#define OP_iload_3         29
#define OP_lload_0         30
#define OP_lload_1         31
#define OP_lload_2         32
#define OP_lload_3         33
#define OP_fload_0         34
#define OP_fload_1         35
#define OP_fload_2         36
#define OP_fload_3         37
#define OP_dload_0         38
#define OP_dload_1         39
#define OP_dload_2         40
#define OP_dload_3         41
#define OP_aload_0         42
#define OP_aload_1         43
#define OP_aload_2         44
#define OP_aload_3         45
#define OP_iaload          46
#define OP_laload          47
#define OP_faload          48
#define OP_daload          49
#define OP_aaload          50
#define OP_baload          51
#define OP_caload          52
#define OP_saload          53
#define OP_istore          54
#define OP_lstore          55
#define OP_fstore          56
#define OP_dstore          57
#define OP_astore          58
#define OP_istore_0        59
#define OP_istore_1        60
#define OP_istore_2        61
#define OP_istore_3        62
#define OP_lstore_0        63
#define OP_lstore_1        64
#define OP_lstore_2        65
#define OP_lstore_3        66
#define OP_fstore_0        67
#define OP_fstore_1        68
#define OP_fstore_2        69
#define OP_fstore_3        70
#define OP_dstore_0        71
#define OP_dstore_1        72
#define OP_dstore_2        73
#define OP_dstore_3        74
#define OP_astore_0        75
#define OP_astore_1        76
#define OP_astore_2        77
#define OP_astore_3        78
#define OP_iastore         79
#define OP_lastore         80
#define OP_fastore         81
#define OP_dastore         82
#define OP_aastore         83
#define OP_bastore         84
#define OP_castore         85
#define OP_sastore         86
#define OP_pop             87
#define OP_pop2            88
#define OP_dup             89
#define OP_dup_x1          90
#define OP_dup_x2          91
#define OP_dup2            92
#define OP_dup2_x1         93
#define OP_dup2_x2         94
#define OP_swap            95
#define OP_iadd            96
#define OP_ladd            97
#define OP_fadd            98
#define OP_dadd            99
#define OP_isub            100
#define OP_lsub            101
#define OP_fsub            102
#define OP_dsub            103
#define OP_imul            104
#define OP_lmul            105
#define OP_fmul            106
#define OP_dmul            107
#define OP_idiv            108
#define OP_ldiv            109
#define OP_fdiv            110
#define OP_ddiv            111
#define OP_irem            112
#define OP_lrem            113
#define OP_frem            114
#define OP_drem            115
#define OP_ineg            116
#define OP_lneg            117
#define OP_fneg            118
#define OP_dneg            119
#define OP_ishl            120
#define OP_lshl            121
#define OP_ishr            122
#define OP_lshr            123
#define OP_iushr           124
#define OP_lushr           125
#define OP_iand            126
#define OP_land            127
#define OP_ior             128
#define OP_lor             129
#define OP_ixor            130
#define OP_lxor            131
#define OP_iinc            132
#define OP_i2l             133
#define OP_i2f             134
#define OP_i2d             135
#define OP_l2i             136
#define OP_l2f             137
#define OP_l2d             138
#define OP_f2i             139
#define OP_f2l             140
#define OP_f2d             141
#define OP_d2i             142
#define OP_d2l             143
#define OP_d2f             144
#define OP_i2b             145
#define OP_i2c             146
#define OP_i2s             147
#define OP_lcmp            148
#define OP_fcmpl           149
#define OP_fcmpg           150
#define OP_dcmpl           151
#define OP_dcmpg           152
#define OP_ifeq            153
#define OP_ifne            154
#define OP_iflt            155
#define OP_ifge            156
#define OP_ifgt            157
#define OP_ifle            158
#define OP_if_icmpeq       159
#define OP_if_icmpne       160
#define OP_if_icmplt       161
#define OP_if_icmpge       162
#define OP_if_icmpgt       163
#define OP_if_icmple       164
#define OP_if_acmpeq       165
#define OP_if_acmpne       166
#define OP_goto            167
#define OP_jsr             168
#define OP_ret             169
#define OP_tableswitch     170
#define OP_lookupswitch    171
#define OP_ireturn         172
#define OP_lreturn         173
#define OP_freturn         174
#define OP_dreturn         175
#define OP_areturn         176
#define OP_return          177
#define OP_getstatic       178
#define OP_putstatic       179
#define OP_getfield        180
#define OP_putfield        181
#define OP_invokevirtual   182
#define OP_invokespecial   183
#define OP_invokestatic    184
#define OP_invokeinterface 185
#define OP_new             187
#define OP_newarray        188
#define OP_anewarray       189
#define OP_arraylength     190
#define OP_athrow          191
#define OP_checkcast       192
#define OP_instanceof      193
#define OP_monitorenter    194
#define OP_monitorexit     195
#define OP_wide            196
#define OP_multianewarray  197
#define OP_ifnull          198
#define OP_ifnonnull       199
#define OP_goto_w          200
#define OP_jsr_w           201
#define OP_breakpoint      202

#ifdef DISASSEMBLER
char* disassembed[] = {
  "OP_nop            ",
  "OP_aconst_null    ",
  "OP_iconst_m1      ",
  "OP_iconst_0       ",
  "OP_iconst_1       ",
  "OP_iconst_2       ",
  "OP_iconst_3       ",
  "OP_iconst_4       ",
  "OP_iconst_5       ",
  "OP_lconst_0       ",
  "OP_lconst_1       ",
  "OP_fconst_0       ",
  "OP_fconst_1       ",
  "OP_fconst_2       ",
  "OP_dconst_0       ",
  "OP_dconst_1       ",
  "OP_bipush         ",
  "OP_sipush         ",
  "OP_ldc            ",
  "OP_ldc_w          ",
  "OP_ldc2_w         ",
  "OP_iload          ",
  "OP_lload          ",
  "OP_fload          ",
  "OP_dload          ",
  "OP_aload          ",
  "OP_iload_0        ",
  "OP_iload_1        ",
  "OP_iload_2        ",
  "OP_iload_3        ",
  "OP_lload_0        ",
  "OP_lload_1        ",
  "OP_lload_2        ",
  "OP_lload_3        ",
  "OP_fload_0        ",
  "OP_fload_1        ",
  "OP_fload_2        ",
  "OP_fload_3        ",
  "OP_dload_0        ",
  "OP_dload_1        ",
  "OP_dload_2        ",
  "OP_dload_3        ",
  "OP_aload_0        ",
  "OP_aload_1        ",
  "OP_aload_2        ",
  "OP_aload_3        ",
  "OP_iaload         ",
  "OP_laload         ",
  "OP_faload         ",
  "OP_daload         ",
  "OP_aaload         ",
  "OP_baload         ",
  "OP_caload         ",
  "OP_saload         ",
  "OP_istore         ",
  "OP_lstore         ",
  "OP_fstore         ",
  "OP_dstore         ",
  "OP_astore         ",
  "OP_istore_0       ",
  "OP_istore_1       ",
  "OP_istore_2       ",
  "OP_istore_3       ",
  "OP_lstore_0       ",
  "OP_lstore_1       ",
  "OP_lstore_2       ",
  "OP_lstore_3       ",
  "OP_fstore_0       ",
  "OP_fstore_1       ",
  "OP_fstore_2       ",
  "OP_fstore_3       ",
  "OP_dstore_0       ",
  "OP_dstore_1       ",
  "OP_dstore_2       ",
  "OP_dstore_3       ",
  "OP_astore_0       ",
  "OP_astore_1       ",
  "OP_astore_2       ",
  "OP_astore_3       ",
  "OP_iastore        ",
  "OP_lastore        ",
  "OP_fastore        ",
  "OP_dastore        ",
  "OP_aastore        ",
  "OP_bastore        ",
  "OP_castore        ",
  "OP_sastore        ",
  "OP_pop            ",
  "OP_pop2           ",
  "OP_dup            ",
  "OP_dup_x1         ",
  "OP_dup_x2         ",
  "OP_dup2           ",
  "OP_dup2_x1        ",
  "OP_dup2_x2        ",
  "OP_swap           ",
  "OP_iadd           ",
  "OP_ladd           ",
  "OP_fadd           ",
  "OP_dadd           ",
  "OP_isub           ",
  "OP_lsub           ",
  "OP_fsub           ",
  "OP_dsub           ",
  "OP_imul           ",
  "OP_lmul           ",
  "OP_fmul           ",
  "OP_dmul           ",
  "OP_idiv           ",
  "OP_ldiv           ",
  "OP_fdiv           ",
  "OP_ddiv           ",
  "OP_irem           ",
  "OP_lrem           ",
  "OP_frem           ",
  "OP_drem           ",
  "OP_ineg           ",
  "OP_lneg           ",
  "OP_fneg           ",
  "OP_dneg           ",
  "OP_ishl           ",
  "OP_lshl           ",
  "OP_ishr           ",
  "OP_lshr           ",
  "OP_iushr          ",
  "OP_lushr          ",
  "OP_iand           ",
  "OP_land           ",
  "OP_ior            ",
  "OP_lor            ",
  "OP_ixor           ",
  "OP_lxor           ",
  "OP_iinc           ",
  "OP_i2l            ",
  "OP_i2f            ",
  "OP_i2d            ",
  "OP_l2i            ",
  "OP_l2f            ",
  "OP_l2d            ",
  "OP_f2i            ",
  "OP_f2l            ",
  "OP_f2d            ",
  "OP_d2i            ",
  "OP_d2l            ",
  "OP_d2f            ",
  "OP_i2b            ",
  "OP_i2c            ",
  "OP_i2s            ",
  "OP_lcmp           ",
  "OP_fcmpl          ",
  "OP_fcmpg          ",
  "OP_dcmpl          ",
  "OP_dcmpg          ",
  "OP_ifeq           ",
  "OP_ifne           ",
  "OP_iflt           ",
  "OP_ifge           ",
  "OP_ifgt           ",
  "OP_ifle           ",
  "OP_if_icmpeq      ",
  "OP_if_icmpne      ",
  "OP_if_icmplt      ",
  "OP_if_icmpge      ",
  "OP_if_icmpgt      ",
  "OP_if_icmple      ",
  "OP_if_acmpeq      ",
  "OP_if_acmpne      ",
  "OP_goto           ",
  "OP_jsr            ",
  "OP_ret            ",
  "OP_tableswitch    ",
  "OP_lookupswitch   ",
  "OP_ireturn        ",
  "OP_lreturn        ",
  "OP_freturn        ",
  "OP_dreturn        ",
  "OP_areturn        ",
  "OP_return         ",
  "OP_getstatic      ",
  "OP_putstatic      ",
  "OP_getfield       ",
  "OP_putfield       ",
  "OP_invokevirtual  ",
  "OP_invokespecial  ",
  "OP_invokestatic   ",
  "OP_invokeinterface",
  "??????????????????",
  "OP_new            ",
  "OP_newarray       ",
  "OP_anewarray      ",
  "OP_arraylength    ",
  "OP_athrow         ",
  "OP_checkcast      ",
  "OP_instanceof     ",
  "OP_monitorenter   ",
  "OP_monitorexit    ",
  "OP_wide           ",
  "OP_multianewarray ",
  "OP_ifnull         ",
  "OP_ifnonnull      ",
  "OP_goto_w         ",
  "OP_jsr_w          ",
  "OP_breakpoint     "
};
#endif

#endif









