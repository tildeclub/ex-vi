/*
 * This code contains changes by
 *      Gunnar Ritter, Freiburg i. Br., Germany, 2002. All rights reserved.
 *
 * Conditions 1, 2, and 4 and the no-warranty notice below apply
 * to these changes.
 *
 *
 * Copyright (c) 1980, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * Copyright(C) Caldera International Inc. 2001-2002. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *   Redistributions of source code and documentation must retain the
 *    above copyright notice, this list of conditions and the following
 *    disclaimer.
 *   Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *   All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed or owned by Caldera
 *      International, Inc.
 *   Neither the name of Caldera International, Inc. nor the names of
 *    other contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE
 * LIABLE FOR ANY DIRECT, INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef lint
#ifdef  DOSCCS
static char sccsid[] = "@(#)ex_vops2.c  1.34 (gritter) 1/12/05";
#endif
#endif

/* from ex_vops2.c      6.8 (Berkeley) 6/7/85 */

#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * Low level routines for operations sequences,
 * and mostly, insert mode (and a subroutine
 * to read an input line, including in the echo area.)
 */
extern char     *vUA1, *vUA2;           /* mjm: extern; also in ex_vops.c */
extern char     *vUD1, *vUD2;           /* mjm: extern; also in ex_vops.c */

/*
 * Obleeperate characters in hardcopy
 * open with \'s.
 */
void
bleep(register int i, char *cp)
{

        i -= column(cp);
        do
                putchar('\\' | QUOTE);
        while (--i >= 0);
        rubble = 1;
}

/*
 * Common code for middle part of delete
 * and change operating on parts of lines.
 */
int
vdcMID(void)
{
        register char *cp;

        squish();
        setLAST();
        if (FIXUNDO)
                vundkind = VCHNG, CP(vutmp, linebuf);
        if (wcursor < cursor)
                cp = wcursor, wcursor = cursor, cursor = cp;
        vUD1 = vUA1 = vUA2 = cursor; vUD2 = wcursor;
        return (column(wcursor + skipleft(linebuf, wcursor)));
}

/*
 * Take text from linebuf and stick it
 * in the VBSIZE buffer BUF.  Used to save
 * deleted text of part of line.
 */
void
takeout(cell *BUF)
{
        register char *cp;

        if (wcursor < linebuf)
                wcursor = linebuf;
        if (cursor == wcursor) {
                beep();
                return;
        }
        if (wcursor < cursor) {
                cp = wcursor;
                wcursor = cursor;
                cursor = cp;
        }
        setBUF(BUF);
        if ((BUF[0] & (QUOTE|TRIM)) == OVERBUF)
                beep();
}

/*
 * Are we at the end of the printed representation of the
 * line?  Used internally in hardcopy open.
 */
int
ateopr(void)
{
        register int i, c;
        register cell *cp = vtube[destline] + destcol;

        for (i = WCOLS - destcol; i > 0; i--) {
                c = *cp++;
                if (c == 0)
                        return (1);
                if (c != ' ' && (c & QUOTE) == 0)
                        return (0);
        }
        return (1);
}

void
showmode(int mode)
{
        int sdc = destcol, sdl = destline;
        char *ocurs, *str;

        if (value(SHOWMODE) == 0 || TCOLUMNS <= 20 || state == ONEOPEN
                        || state == HARDOPEN || vmacp != NULL)
                return;
        ocurs = cursor;
        fixech();
        vgoto(WECHO, TCOLUMNS - 20);
        switch (mode) {
        case 0:         str = catgets(catd, 1, 227,
                                        "               ");
                        break;
        case 'A':       /*FALLTHROUGH*/
        case 'a':       str = catgets(catd, 1, 228,
                                        "AAPPEND MODE");
                        break;
        case 'C':       /*FALLTHROUGH*/
        case 'c':       str = catgets(catd, 1, 229,
                                        "CCHANGE MODE");
                        break;
        case 'O':       /*FALLTHROUGH*/
        case 'o':       str = catgets(catd, 1, 230,
                                        "OOPEN MODE");
                        break;
        case 'R':       str = catgets(catd, 1, 231,
                                        "RREPLACE MODE");
                        break;
        case 'r':       str = catgets(catd, 1, 232,
                                        "rREPLACE 1 CHAR");
                        break;
        default:        str = catgets(catd, 1, 233,
                                        "IINSERT MODE");
        }
        if (value(TERSE))
                putchar(str[0]);
        else
                printf("%s",&str[1]);
        vgoto(sdl, sdc);
        cursor = ocurs;
        splitw = 0;
}

/*
 * Append.
 *
 * This routine handles the top level append, doing work
 * as each new line comes in, and arranging repeatability.
 * It also handles append with repeat counts, and calculation
 * of autoindents for new lines.
 */
bool    vaifirst;
bool    gobbled;
char    *ogcursor;

/*
 * The addtext() and addto() routines combined, accepting a single
 * cell character.
 */
void
addc(cell c)
{
        register cell *cp = INS;

        if (vglobp)
                return;
        if ((cp[0] & (QUOTE|TRIM)) != OVERBUF) {
                if (cellen(cp) + 2 >= VBSIZE) {
                        cp[0] = OVERBUF;
                        lastcmd[0] = 0;
                } else {
                        while (*cp)
                                cp++;
                        *cp++ = c;
                        *cp++ = 0;
                }
        }
}

void
vappend(int ch, int cnt, int indent)
/*      int ch;         /\* mjm: char --> int */
{
        register int i = 0;
        register char *gcursor;
        bool escape;
        int repcnt, savedoomed;
        short oldhold = hold;
#ifdef  SIGWINCH
        sigset_t set, oset;
#endif

        /*
         * Before a move in hardopen when the line is dirty
         * or we are in the middle of the printed representation,
         * we retype the line to the left of the cursor so the
         * insert looks clean.
         */
        if (ch != 'o' && state == HARDOPEN && (rubble || !ateopr())) {
                rubble = 1;
                gcursor = cursor;
                i = *gcursor;
                *gcursor = ' ';
                wcursor = gcursor;
                vmove(0);
                *gcursor = i;
        }
        vaifirst = indent == 0;

        showmode(ch);

        /*
         * Handle replace character by (eventually)
         * limiting the number of input characters allowed
         * in the vgetline routine.
         */
        if (ch == 'r')
                repcnt = 2;
        else
                repcnt = 0;

        /*
         * If an autoindent is specified, then
         * generate a mixture of blanks to tabs to implement
         * it and place the cursor after the indent.
         * Text read by the vgetline routine will be placed in genbuf,
         * so the indent is generated there.
         */
        if (value(AUTOINDENT) && indent != 0) {
                gcursor = genindent(indent);
                *gcursor = 0;
                vgotoCL(qcolumn(cursor + skipright(cursor, linebuf), genbuf));
        } else {
                gcursor = genbuf;
                *gcursor = 0;
                if (ch == 'o')
                        vfixcurs();
        }

        /*
         * Prepare for undo.  Pointers delimit inserted portion of line.
         */
        vUA1 = vUA2 = cursor;

        /*
         * If we are not in a repeated command and a ^@ comes in
         * then this means the previous inserted text.
         * If there is none or it was too long to be saved,
         * then beep() and also arrange to undo any damage done
         * so far (e.g. if we are a change.)
         */
        if ((vglobp && *vglobp == 0) || peekbr()) {
                if ((INS[0] & (QUOTE|TRIM)) == OVERBUF) {
                        beep();
                        if (!splitw)
                                ungetkey('u');
                        doomed = 0;
                        hold = oldhold;
                        showmode(0);
                        return;
                }
                /*
                 * Unread input from INS.
                 * An escape will be generated at end of string.
                 * Hold off n^^2 type update on dumb terminals.
                 */
                vglobp = INS;
                hold |= HOLDQIK;
        } else if (vglobp == 0)
                /*
                 * Not a repeated command, get
                 * a new inserted text for repeat.
                 */
                INS[0] = 0;

        /*
         * For wrapmargin to hack away second space after a '.'
         * when the first space caused a line break we keep
         * track that this happened in gobblebl, which says
         * to gobble up a blank silently.
         */
        gobblebl = 0;

#ifdef  SIGWINCH
        sigemptyset(&set);
        sigaddset(&set, SIGWINCH);
        sigprocmask(SIG_BLOCK, &set, &oset);
#endif
        /*
         * Text gathering loop.
         * New text goes into genbuf starting at gcursor.
         * cursor preserves place in linebuf where text will eventually go.
         */
        if (*cursor == 0 || state == CRTOPEN)
                hold |= HOLDROL;
        for (;;) {
                if (ch == 'r' && repcnt == 0)
                        escape = 0;
                else {
                        gcursor = vgetline(repcnt, gcursor, &escape, ch);

                        /*
                         * After an append, stick information
                         * about the ^D's and ^^D's and 0^D's in
                         * the repeated text buffer so repeated
                         * inserts of stuff indented with ^D as backtab's
                         * can work.
                         */
                        if (HADUP)
                                addc('^');
                        else if (HADZERO)
                                addc('0');
                        while (CDCNT > 0)
#ifndef BIT8
                                addc('\204'), CDCNT--;
#else
                                addc(OVERBUF), CDCNT--;
#endif

                        if (gobbled)
                                addc(' ');
                        addtext(ogcursor);
                }
                repcnt = 0;

                /*
                 * Smash the generated and preexisting indents together
                 * and generate one cleanly made out of tabs and spaces
                 * if we are using autoindent.
                 */
                if (!vaifirst && value(AUTOINDENT)) {
                        i = fixindent(indent);
                        if (!HADUP)
                                indent = i;
                        gcursor = strend(genbuf);
                }

                /*
                 * Limit the repetition count based on maximum
                 * possible line length; do output implied
                 * by further count (> 1) and cons up the new line
                 * in linebuf.
                 */
                cnt = vmaxrep(ch, cnt);
                CP(gcursor + skipright(ogcursor, gcursor), cursor);
                do {
                        CP(cursor, genbuf);
                        if (cnt > 1) {
                                int oldhold = hold;

                                Outchar = vinschar;
                                hold |= HOLDQIK;
                                printf("%s", genbuf);
                                hold = oldhold;
                                Outchar = vputchar;
                        }
                        cursor += gcursor - genbuf;
                } while (--cnt > 0);
                endim();
                vUA2 = cursor;
                if (escape != '\n')
                        CP(cursor, gcursor + skipright(ogcursor, gcursor));

                /*
                 * If doomed characters remain, clobber them,
                 * and reopen the line to get the display exact.
                 */
                if (state != HARDOPEN) {
                        DEPTH(vcline) = 0;
                        savedoomed = doomed;
                        if (doomed > 0) {
                                register int cind = cindent();

                                physdc(cind, cind + doomed);
                                doomed = 0;
                        }
                        i = vreopen(LINE(vcline), lineDOT(), vcline);
#ifdef TRACE
                        if (trace)
                                fprintf(trace, "restoring doomed from %d to %d\n", doomed, savedoomed);
#endif
                        if (ch == 'R')
                                doomed = savedoomed;
                }

                /*
                 * All done unless we are continuing on to another line.
                 */
                if (escape != '\n')
                        break;

                /*
                 * Set up for the new line.
                 * First save the current line, then construct a new
                 * first image for the continuation line consisting
                 * of any new autoindent plus the pushed ahead text.
                 */
                showmode(0);
                killU();
                addc(gobblebl ? ' ' : '\n');
                vsave();
                cnt = 1;
                if (value(AUTOINDENT)) {
#ifdef LISPCODE
                        if (value(LISP))
                                indent = lindent(dot + 1);
                        else
#endif
                             if (!HADUP && vaifirst)
                                indent = whitecnt(linebuf);
                        vaifirst = 0;
                        strcLIN(vpastwh(gcursor + 1));
                        gcursor = genindent(indent);
                        *gcursor = 0;
                        if (gcursor + strlen(linebuf) > &genbuf[LBSIZE - 2])
                                gcursor = genbuf;
                        CP(gcursor, linebuf);
                } else {
                        CP(genbuf, gcursor + skipright(ogcursor, gcursor));
                        gcursor = genbuf;
                }

                /*
                 * If we started out as a single line operation and are now
                 * turning into a multi-line change, then we had better yank
                 * out dot before it changes so that undo will work
                 * correctly later.
                 */
                if (FIXUNDO && vundkind == VCHNG) {
                        vremote(1, yank, 0);
                        undap1--;
                }

                /*
                 * Now do the append of the new line in the buffer,
                 * and update the display.  If slowopen
                 * we don't do very much.
                 */
                vdoappend(genbuf);
                vundkind = VMANYINS;
                vcline++;
                if (state != VISUAL)
                        vshow(dot, NOLINE);
                else {
                        i += LINE(vcline - 1);
                        vopen(dot, i);
                        if (value(SLOWOPEN))
                                vscrap();
                        else
                                vsync1(LINE(vcline));
                }
                strcLIN(gcursor);
                *gcursor = 0;
                cursor = linebuf;
                vgotoCL(qcolumn(cursor + skipleft(ogcursor, cursor), genbuf));
                showmode(ch);
        }

        /*
         * All done with insertion, position the cursor
         * and sync the screen.
         */
        showmode(0);
        hold = oldhold;
        if (cursor > linebuf)
                cursor += skipleft(linebuf, cursor);
        if (state != HARDOPEN)
                vsyncCL();
        else if (cursor > linebuf)
                back1();
        doomed = 0;
        wcursor = cursor;
        vmove(0);
#ifdef  SIGWINCH
        sigprocmask(SIG_SETMASK, &oset, NULL);
#endif
}

/*
 * Subroutine for vgetline to back up a single character position,
 * backwards around end of lines (vgoto can't hack columns which are
 * less than 0 in general).
 */
void
back1(void)
{

        vgoto(destline - 1, WCOLS + destcol - 1);
}

#define gappend(c) { \
                int     _c = c; \
                xgappend(_c, &gcursor); \
        }

static void
xgappend(int c, char **gp)
{
        if (*gp >= &genbuf[MAXBSIZE-mb_cur_max-1]) {
                beep();
                return;
        }
#ifdef  MB
        if (mb_cur_max > 1 && !(c & INVBIT)) {
                char    mb[MB_LEN_MAX];
                int     i, n;
                n = wctomb(mb, c);
                for (i = 0; i < n; i++)
                        *(*gp)++ = mb[i];
        } else
#endif  /* MB */
                *(*gp)++ = c & 0377;
}

/*
 * Get a line into genbuf after gcursor.
 * Cnt limits the number of input characters
 * accepted and is used for handling the replace
 * single character command.  Aescaped is the location
 * where we stick a termination indicator (whether we
 * ended with an ESCAPE or a newline/return.
 *
 * We do erase-kill type processing here and also
 * are careful about the way we do this so that it is
 * repeatable.  (I.e. so that your kill doesn't happen,
 * when you repeat an insert if it was escaped with \ the
 * first time you did it.  commch is the command character
 * involved, including the prompt for readline.
 */
char *
vgetline(int cnt, char *gcursor, bool *aescaped, int commch)
{
        register int c, ch;
        register char *cp;
        int x, y, iwhite, backsl=0;
        cell *iglobp;
        char cstr[2];
        int (*OO)() = Outchar;

        /*
         * Clear the output state and counters
         * for autoindent backwards motion (counts of ^D, etc.)
         * Remember how much white space at beginning of line so
         * as not to allow backspace over autoindent.
         */
        *aescaped = 0;
        ogcursor = gcursor;
        flusho();
        CDCNT = 0;
        HADUP = 0;
        HADZERO = 0;
        gobbled = 0;
        iwhite = whitecnt(genbuf);
        iglobp = vglobp;

        /*
         * Carefully avoid using vinschar in the echo area.
         */
        if (splitw)
                Outchar = vputchar;
        else {
                Outchar = vinschar;
                vprepins();
        }
        for (;;) {
                backsl = 0;
                if (gobblebl)
                        gobblebl--;
                if (cnt != 0) {
                        cnt--;
                        if (cnt == 0)
                                goto vadone;
                }
                c = getkey();
                if (c != ATTN)
                        c &= (QUOTE|TRIM);
                ch = c;
                maphopcnt = 0;
                if (vglobp == 0 && Peekkey == 0 && commch != 'r')
                        while ((ch = map(c, immacs)) != c) {
                                c = ch;
                                if (!value(REMAP))
                                        break;
                                if (++maphopcnt > 256)
                                        error(catgets(catd, 1, 234,
                                                "Infinite macro loop"));
                        }
                if (!iglobp) {

                        /*
                         * Erase-kill type processing.
                         * Only happens if we were not reading
                         * from untyped input when we started.
                         * Map users erase to ^H, kill to -1 for switch.
                         */
                        if (c == tty.c_cc[VERASE])
                                c = CTRL('h');
                        else if (c == tty.c_cc[VKILL])
                                c = -1;
                        if (c == ATTN)
                                goto case_ATTN;
                        switch (c) {

                        /*
                         * ^?           Interrupt drops you back to visual
                         *              command mode with an unread interrupt
                         *              still in the input buffer.
                         *
                         * ^\           Quit does the same as interrupt.
                         *              If you are a ex command rather than
                         *              a vi command this will drop you
                         *              back to command mode for sure.
                         */
                        case QUIT:
case_ATTN:
                                ungetkey(c);
                                goto vadone;

                        /*
                         * ^H           Backs up a character in the input.
                         *
                         * BUG:         Can't back around line boundaries.
                         *              This is hard because stuff has
                         *              already been saved for repeat.
                         */
                        case CTRL('h'):
bakchar:
                                cp = gcursor + skipleft(ogcursor, gcursor);
                                if (cp < ogcursor) {
                                        if (splitw) {
                                                /*
                                                 * Backspacing over readecho
                                                 * prompt. Pretend delete but
                                                 * don't beep.
                                                 */
                                                ungetkey(c);
                                                goto vadone;
                                        }
                                        beep();
                                        continue;
                                }
                                goto vbackup;

                        /*
                         * ^W           Back up a white/non-white word.
                         */
                        case CTRL('w'):
                                wdkind = 1;
                                for (cp = gcursor; cp > ogcursor
                                                && isspace(cp[-1]&0377); cp--)
                                        continue;
                                for (c = wordch(cp - 1);
                                    cp > ogcursor && wordof(c, cp - 1); cp--)
                                        continue;
                                goto vbackup;

                        /*
                         * users kill   Kill input on this line, back to
                         *              the autoindent.
                         */
                        case -1:
                                cp = ogcursor;
vbackup:
                                if (cp == gcursor) {
                                        beep();
                                        continue;
                                }
                                endim();
                                *cp = 0;
                                c = cindent();
                                vgotoCL(qcolumn(cursor +
                                        skipleft(linebuf, cursor), genbuf));
                                if (doomed >= 0)
                                        doomed += c - cindent();
                                gcursor = cp;
                                continue;

                        /*
                         * \            Followed by erase or kill
                         *              maps to just the erase or kill.
                         */
                        case '\\':
                                x = destcol, y = destline;
                                putchar('\\');
                                vcsync();
                                c = getkey();
                                if (c == tty.c_cc[VERASE]
                                    || c == tty.c_cc[VKILL])
                                {
                                        vgoto(y, x);
                                        if (doomed >= 0)
                                                doomed++;
                                        goto def;
                                }
                                ungetkey(c), c = '\\';
                                backsl = 1;
                                break;

                        /*
                         * ^Q           Super quote following character
                         *              Only ^@ is verboten (trapped at
                         *              a lower level) and \n forces a line
                         *              split so doesn't really go in.
                         *
                         * ^V           Synonym for ^Q
                         */
                        case CTRL('q'):
                        case CTRL('v'):
                                x = destcol, y = destline;
                                putchar('^');
                                vgoto(y, x);
                                c = getkey();
                                if (c != NL) {
                                        if (doomed >= 0)
                                                doomed++;
                                        goto def;
                                }
                                break;
                        }
                }

                /*
                 * If we get a blank not in the echo area
                 * consider splitting the window in the wrapmargin.
                 */
                if (c != NL && !splitw) {
                        if (c == ' ' && gobblebl) {
                                gobbled = 1;
                                continue;
                        }
                        if (value(WRAPMARGIN) &&
                                (outcol >= OCOLUMNS - value(WRAPMARGIN) ||
                                 backsl && outcol==0) &&
                                commch != 'r') {
                                /*
                                 * At end of word and hit wrapmargin.
                                 * Move the word to next line and keep going.
                                 */
                                wdkind = 1;
                                gappend(c);
                                if (backsl)
                                        gappend(getkey());
                                *gcursor = 0;
                                /*
                                 * Find end of previous word if we are past it.
                                 */
                                for (cp=gcursor; cp>ogcursor
                                                && isspace(cp[-1]&0377); cp--)
                                        ;
                                if (outcol+(backsl?OCOLUMNS:0) - (gcursor-cp) >= OCOLUMNS - value(WRAPMARGIN)) {
                                        /*
                                         * Find beginning of previous word.
                                         */
                                        for (; cp>ogcursor && !isspace(cp[-1]&0377); cp--)
                                                ;
                                        if (cp <= ogcursor) {
                                                /*
                                                 * There is a single word that
                                                 * is too long to fit.  Just
                                                 * let it pass, but beep for
                                                 * each new letter to warn
                                                 * the luser.
                                                 */
                                                c = *--gcursor;
                                                *gcursor = 0;
                                                beep();
                                                goto dontbreak;
                                        }
                                        /*
                                         * Save it for next line.
                                         */
                                        macpush(cp, 0);
                                        cp--;
                                }
                                macpush("\n", 0);
                                /*
                                 * Erase white space before the word.
                                 */
                                while (cp > ogcursor && isspace(cp[-1]&0377))
                                        cp--;   /* skip blank */
                                gobblebl = 3;
                                goto vbackup;
                        }
                dontbreak:;
                }

                /*
                 * Word abbreviation mode.
                 */
                cstr[0] = c;
                if (anyabbrs && gcursor > ogcursor && !wordch(cstr) && wordch(gcursor-1)) {
                                int wdtype, abno;

                                cstr[1] = 0;
                                wdkind = 1;
                                cp = gcursor + skipleft(ogcursor, gcursor);
                                for (wdtype = wordch(cp - 1);
                                    cp > ogcursor && wordof(wdtype, cp - 1); cp--)
                                        ;
                                *gcursor = 0;
                                for (abno=0; abbrevs[abno].mapto; abno++) {
                                        if (!abbrevs[abno].hadthis &&
                                                eq(cp, abbrevs[abno].cap)) {
                                                abbrevs[abno].hadthis++;
                                                macpush(cstr, 0);
                                                macpush(abbrevs[abno].mapto, 0);
                                                goto vbackup;
                                        }
                                }
                }

#ifdef  BIT8
                if (c == OVERBUF)
                        goto btrp;
#endif
                switch (c) {

                /*
                 * ^M           Except in repeat maps to \n.
                 */
                case CR:
                        if (vglobp)
                                goto def;
                        c = '\n';
                        /* presto chango ... */

                /*
                 * \n           Start new line.
                 */
                case NL:
                        *aescaped = c;
                        goto vadone;

                /*
                 * escape       End insert unless repeat and more to repeat.
                 */
                case ESCAPE:
                        if (lastvgk)
                                goto def;
                        goto vadone;

                /*
                 * ^D           Backtab.
                 * ^T           Software forward tab.
                 *
                 *              Unless in repeat where this means these
                 *              were superquoted in.
                 */
                case CTRL('d'):
                case CTRL('t'):
                        if (vglobp)
                                goto def;
                        /* fall into ... */

                /*
                 * ^D|QUOTE     Is a backtab (in a repeated command).
                 */
#ifndef BIT8
                case CTRL('d') | QUOTE:
#else
btrp:
#endif
                        *gcursor = 0;
                        cp = vpastwh(genbuf);
                        c = whitecnt(genbuf);
                        if (ch == CTRL('t')) {
                                /*
                                 * ^t just generates new indent replacing
                                 * current white space rounded up to soft
                                 * tab stop increment.
                                 */
                                if (cp != gcursor)
                                        /*
                                         * BUG:         Don't hack ^T except
                                         *              right after initial
                                         *              white space.
                                         */
                                        continue;
                                cp = genindent(iwhite = backtab(c + value(SHIFTWIDTH) + 1));
                                ogcursor = cp;
                                goto vbackup;
                        }
                        /*
                         * ^D works only if we are at the (end of) the
                         * generated autoindent.  We count the ^D for repeat
                         * purposes.
                         */
                        if (c == iwhite && c != 0)
                                if (cp == gcursor) {
                                        iwhite = backtab(c);
                                        CDCNT++;
                                        ogcursor = cp = genindent(iwhite);
                                        goto vbackup;
                                } else if (&cp[1] == gcursor &&
                                    (*cp == '^' || *cp == '0')) {
                                        /*
                                         * ^^D moves to margin, then back
                                         * to current indent on next line.
                                         *
                                         * 0^D moves to margin and then
                                         * stays there.
                                         */
                                        HADZERO = *cp == '0';
                                        ogcursor = cp = genbuf;
                                        HADUP = 1 - HADZERO;
                                        CDCNT = 1;
                                        endim();
                                        back1();
                                        vputchar(' ');
                                        goto vbackup;
                                }
                        if (vglobp && vglobp - iglobp >= 2 &&
                            (vglobp[-2] == '^' || vglobp[-2] == '0')
                            && gcursor == ogcursor + 1)
                                goto bakchar;
                        continue;

                default:
                        /*
                         * Possibly discard control inputs.
                         */
                        if (!vglobp && junk(c)) {
                                beep();
                                continue;
                        }
def:
                        if (!backsl) {
                                /* int cnt; */
                                putchar(c);
                                flush();
                        }
                        if (gcursor > &genbuf[LBSIZE - 2])
                                error(catgets(catd, 1, 235, "Line too long"));
                        gappend(c & TRIM);
                        vcsync();
                        if (value(SHOWMATCH) && !iglobp)
                                if (c == ')' || c == '}')
                                        lsmatch(gcursor);
                        continue;
                }
        }
vadone:
        *gcursor = 0;
        if (Outchar != termchar)
                Outchar = OO;
        endim();
        return (gcursor);
}

char    *vsplitpt;

/*
 * Append the line in buffer at lp
 * to the buffer after dot.
 */
void
vdoappend(char *lp)
{
        register int oing = inglobal;

        vsplitpt = lp;
        inglobal = 1;
        ignore(append(vgetsplit, dot));
        inglobal = oing;
}

/*
 * Subroutine for vdoappend to pass to append.
 */
int
vgetsplit(void)
{

        if (vsplitpt == 0)
                return (EOF);
        strcLIN(vsplitpt);
        vsplitpt = 0;
        return (0);
}

/*
 * Vmaxrep determines the maximum repetitition factor
 * allowed that will yield total line length less than
 * LBSIZE characters and also does hacks for the R command.
 */
int
vmaxrep(int ch, register int cnt)
{
        register int len, replen;

        if (cnt > LBSIZE - 2)
                cnt = LBSIZE - 2;
        replen = strlen(genbuf);
        if (ch == 'R') {
                len = strlen(cursor);
                if (replen < len)
                        len = replen;
#ifdef  MB
                if (mb_cur_max > 1) {
                        char    *cp, *gp;
                        int     c, g;
                        for (gp = genbuf, g = 0; *gp; g++)
                                gp += wskipright(genbuf, gp);
                        for (cp = cursor, c = 0; c < g; c++)
                                cp += wskipright(cursor, cp);
                        CP(cursor, cp);
                } else
#endif  /* MB */
                        CP(cursor, cursor + len);
                vUD2 += len;
        }
        len = strlen(linebuf);
        if (len + cnt * replen <= LBSIZE - 2)
                return (cnt);
        cnt = (LBSIZE - 2 - len) / replen;
        if (cnt == 0) {
                vsave();
                error(catgets(catd, 1, 236, "Line too long"));
        }
        return (cnt);
}
