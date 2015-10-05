/* pi32cipher-asm.S */
/*
    This file is part of the ARM-Crypto-Lib.
    Copyright (C) 2006-2015 Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


	.syntax unified
	.cpu cortex-m0
	.thumb

.global mu
mu:
	push {a1, v1, v2, v3, v4, lr}
	adr v1, mu_const
	ldm v1, {v1, v2, v3, v4}
	ldm a2, {a1, a2, a3, a4}

	adds v1, v1, a1
	adds v1, v1, a2
	adds v1, v1, a3

	adds v2, v2, a1
	adds v2, v2, a2
	adds v2, v2, a4

	adds v3, v3, a1
	adds v3, v3, a3
	adds v3, v3, a4

	adds v4, v4, a2
	adds v4, v4, a3
	adds v4, v4, a4

	movs a1, #(32 - 5)
	rors v1, v1, a1

	movs a1, #(32 - 11)
	rors v2, v2, a1

	movs a1, #(32 - 17)
	rors v3, v3, a1

	movs a1, #(32 - 23)
	rors v4, v4, a1

	movs a1, v1
	eors a1, a1, v2
	eors a1, a1, v3
	eors a1, a1, v4

	eors v1, v1, a1
	eors v2, v2, a1
	eors v3, v3, a1
	eors v4, v4, a1

	pop {a1}
	stm a1!, {v3, v4}
	stm a1!, {v1, v2}
	pop {v1, v2, v3, v4, pc}

.global ny
ny:
/*	push {a1, v1, v2, v3, v4, lr} */
	adr v1, ny_const
	ldm v1, {v1, v2, v3, v4}
	ldm a2, {a1, a2, a3, a4}

	adds v1, v1, a1
	adds v1, v1, a3
	adds v1, v1, a4

	adds v2, v2, a2
	adds v2, v2, a3
	adds v2, v2, a4

	adds v3, v3, a1
	adds v3, v3, a2
	adds v3, v3, a3

	adds v4, v4, a1
	adds v4, v4, a2
	adds v4, v4, a4

	movs a1, #(32 - 3)
	rors v1, v1, a1

	movs a1, #(32 - 10)
	rors v2, v2, a1

	movs a1, #(32 - 19)
	rors v3, v3, a1

	movs a1, #(32 - 29)
	rors v4, v4, a1

	movs a1, v1
	eors a1, a1, v2
	eors a1, a1, v3
	eors a1, a1, v4

	eors v1, v1, a1
	eors v2, v2, a1
	eors v3, v3, a1
	eors v4, v4, a1
/*
	pop {a1}
	stm a1!, {v1, v2, v3, v4}
	pop {v1, v2, v3, v4, pc}
*/
	bx lr

	.align 4
mu_const:
	.word 0xF0E8E4E2, 0xE1D8D4D2, 0xD1CCCAC9, 0xC6C5C3B8
ny_const:
	.word 0xB4B2B1AC, 0xAAA9A6A5, 0xA39C9A99, 0x9695938E

.global ast
ast:
	push {a1, v1, v2, v3, v4, lr}
	sub SP, SP, #(1 * 4 * 4)
	mov v2, a3
	mov r0, SP
	bl mu
/*	add r0, SP, #16 */
	mov r1, v2
	bl ny
	pop {r0, r1, r2, r3} /*, v1, v2, v3, v4} */
	adds r0, r0, v1
	adds r1, r1, v2
	adds r2, r2, v3
	adds r3, r3, v4
	pop {v1}
	stm v1!, {r1, r2, r3}
	stm v1!, {r0}
	pop {v1, v2, v3, v4, pc}

/*
static void e1(
        word_t *dest,
        const word_t c[4],
        const word_t *i )
{
    uint8_t n = PI_N - 1;
    {
//        word_t t[4];
//        memcpy(t, c, sizeof(word_t) * 4);
        ast(dest, c, i);
    }
    do {
        i = &i[4];
        ast(&dest[4], dest, i);
        dest = &dest[4];
    } while (--n);
}
*/
.global e1
e1:
	push {v1, v2, v3, v4, lr}
	mov v1, a1
	mov v3, a3
	bl ast
	movs v2, #3
1:
	mov a2, v1
	adds v1, #16
	mov a1, v1
	adds v3, #16
	mov a3, v3
	bl ast
	subs v2, #1
	bne 1b
	pop {v1, v2, v3, v4, pc}

/*
static void e2(
        word_t *dest,
        const word_t c[4],
        const word_t *i )
{
    uint8_t n = PI_N - 1;
    {
        word_t t[4];
        memcpy(t, c, sizeof(word_t) * 4);
        ast(&dest[4 * n], &i[4 * n], t);
    }
    while (n--) {
        ast(&dest[4 * n], &i[4 * n], &dest[4 * (n + 1)]);
    }
}
*/
.global e2
e2:
	push {v1, v2, v3, v4, lr}
	adds a1, a1, #(4 * 4 * 3)
	adds a3, a3, #(4 * 4 * 3)
	mov v1, a1
	mov v3, a3
	mov a3, a2
	mov a2, v3
	bl ast
	movs v2, #3
1:
	mov a3, v1
	subs v1, #16
	mov a1, v1
	subs v3, #16
	mov a2, v3
	bl ast
	subs v2, #1
	bne 1b
	pop {v1, v2, v3, v4, pc}

/*
static void pi(
        word_t *a )
{
    uint8_t r = PI_ROUNDS;
    word_t t[4 * 4];
    const word_t *c = (const word_t *)pi_const;
    do {
        e1(t, c, a);
        c = &c[4];
        e2(a, c, t);
        c = &c[4];
    } while (--r);
}
*/
.global pi
pi:
	push {v1, v2, v3, lr}
	sub SP, SP, #(4 * 4 * 4)
	adr v2, pi_const
	mov v1, a1
	movs v3, #3
1:
	mov a1, SP
	mov a2, v2
	adds v2, v2, #16
	mov a3, v1
	bl e1
	mov a1, v1
	mov a2, v2
	adds v2, v2, #16
	mov a3, SP
	bl e2
	subs v3, #1
	bne 1b
	add SP, SP, #(4 * 4 * 4)
	pop {v1, v2, v3, pc}

	.align 4
pi_const:
	.word 0x8D8B8778, 0x7472716C, 0x6A696665, 0x635C5A59
	.word 0x5655534E, 0x4D4B473C, 0x3A393635, 0x332E2D2B
	.word 0x271E1D1B, 0x170FF0E8, 0xE4E2E1D8, 0xD4D2D1CC
	.word 0xCAC9C6C5, 0xC3B8B4B2, 0xB1ACAAA9, 0xA6A5A39C
	.word 0x9A999695, 0x938E8D8B, 0x87787472, 0x716C6A69
	.word 0x6665635C, 0x5A595655, 0x534E4D4B, 0x473C3A39
