/* Readline tab completion
   Copyright (C) 2020 Markus Uhlin. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   - Neither the name of the author nor the names of its contributors may be
     used to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

#include "common.h"

/* names.h wants this header before itself */
#include "irc.h"
#include "events/names.h"

#include "config.h"
#include "dataClassify.h"
#include "io-loop.h"
#include "printtext.h"
#include "readline.h"
#include "readlineTabCompletion.h"
#include "strHand.h"
#include "terminal.h"

static void
auto_complete_query(volatile struct readline_session_context *ctx,
    const char *s)
{
    const wchar_t cmd[] = L"/query ";
    size_t i = 0;

    while (ctx->n_insert != 0)
	readline_handle_backspace(ctx);

    for (i = 0; i < wcslen(cmd); i++)
	readline_handle_key_exported(ctx, cmd[i]);
    for (i = 0; i < strlen(s); i++)
	readline_handle_key_exported(ctx, btowc(s[i]));
}

static void
auto_complete_setting(volatile struct readline_session_context *ctx,
    const char *s)
{
    const wchar_t cmd[] = L"/set ";
    size_t i = 0;

    while (ctx->n_insert != 0)
	readline_handle_backspace(ctx);

    for (i = 0; i < wcslen(cmd); i++)
	readline_handle_key_exported(ctx, cmd[i]);
    for (i = 0; i < strlen(s); i++)
	readline_handle_key_exported(ctx, btowc(s[i]));
}

static void
auto_complete_whois(volatile struct readline_session_context *ctx,
    const char *s)
{
    const wchar_t cmd[] = L"/whois ";
    size_t i = 0;

    while (ctx->n_insert != 0)
	readline_handle_backspace(ctx);

    for (i = 0; i < wcslen(cmd); i++)
	readline_handle_key_exported(ctx, cmd[i]);
    for (i = 0; i < strlen(s); i++)
	readline_handle_key_exported(ctx, btowc(s[i]));
}

static void
auto_complete_command(volatile struct readline_session_context *ctx,
    const char *s)
{
    while (ctx->n_insert != 0)
	readline_handle_backspace(ctx);

    readline_handle_key_exported(ctx, btowc('/'));

    for (size_t i = 0; i < strlen(s); i++)
	readline_handle_key_exported(ctx, btowc(s[i]));
}

static void
auto_complete_channel_user(volatile struct readline_session_context *ctx,
    const char *s)
{
    while (ctx->n_insert != 0)
	readline_handle_backspace(ctx);

    for (size_t i = 0; i < strlen(s); i++)
	readline_handle_key_exported(ctx, btowc(s[i]));

    readline_handle_key_exported(ctx, btowc(':'));
    readline_handle_key_exported(ctx, btowc(' '));
}

static bool
buf_contains_disallowed_chars(const volatile struct readline_session_context *ctx)
{
    char *s = readline_finalize_out_string_exported(ctx->buffer);
    const char reject[] =
	TXT_BLINK TXT_BOLD TXT_COLOR TXT_NORMAL TXT_REVERSE TXT_UNDERLINE;
    const bool yes_no = strpbrk(s, reject) != NULL;
    free(s);
    return yes_no;
}

static inline char *
get_search_var(const volatile struct readline_session_context *ctx)
{
    return (& (ctx->tc->search_var[0]));
}

static void
output_error(const char *msg)
{
    PRINTTEXT_CONTEXT ctx;

    printtext_context_init(&ctx, g_active_window, TYPE_SPEC1_WARN, true);
    printtext(&ctx, "%s", msg);
    term_beep();
}

static int
store_search_var(const volatile struct readline_session_context *ctx)
{
    char *s = readline_finalize_out_string_exported(ctx->buffer);

    const int store_res =
	sw_strcpy(ctx->tc->search_var, s, ARRAY_SIZE(ctx->tc->search_var));

    free(s);
    return (store_res != 0 ? -1 : 0);
}

PTAB_COMPLETION
readline_tab_comp_ctx_new(void)
{
    static TAB_COMPLETION ctx;

    memset(ctx.search_var, 0, ARRAY_SIZE(ctx.search_var));
    ctx.isInCirculationModeForQuery	= false;
    ctx.isInCirculationModeForSettings	= false;
    ctx.isInCirculationModeForWhois	= false;
    ctx.isInCirculationModeForCmds	= false;
    ctx.isInCirculationModeForChanUsers = false;
    ctx.matches = NULL;
    ctx.elmt = NULL;

    return (&ctx);
}

void
readline_tab_comp_ctx_destroy(PTAB_COMPLETION ctx)
{
    if (ctx) {
	if (!isNull(ctx->matches))
	    textBuf_destroy(ctx->matches);
    }
}

void
readline_tab_comp_ctx_reset(PTAB_COMPLETION ctx)
{
    if (ctx) {
	memset(ctx->search_var, 0, ARRAY_SIZE(ctx->search_var));
	ctx->isInCirculationModeForQuery     = false;
	ctx->isInCirculationModeForSettings  = false;
	ctx->isInCirculationModeForWhois     = false;
	ctx->isInCirculationModeForCmds	     = false;
	ctx->isInCirculationModeForChanUsers = false;
	if (!isNull(ctx->matches))
	    textBuf_destroy(ctx->matches);
	ctx->matches = NULL;
	ctx->elmt = NULL;
    }
}

void
readline_handle_tab(volatile struct readline_session_context *ctx)
{
    if (ctx->n_insert == 0 || ctx->insert_mode ||
	buf_contains_disallowed_chars(ctx)) {
	output_error("no magic");
	readline_tab_comp_ctx_reset(ctx->tc);
	return;
    } else if (ctx->tc->isInCirculationModeForQuery) {
	if (ctx->tc->elmt == textBuf_tail(ctx->tc->matches)) {
	    output_error("no more matches");
	    readline_tab_comp_ctx_reset(ctx->tc);
	} else {
	    ctx->tc->elmt = ctx->tc->elmt->next;
	    auto_complete_query(ctx, ctx->tc->elmt->text);
	}

	return;
    } else if (ctx->tc->isInCirculationModeForSettings) {
	if (ctx->tc->elmt == textBuf_tail(ctx->tc->matches)) {
	    output_error("no more matches");
	    readline_tab_comp_ctx_reset(ctx->tc);
	} else {
	    ctx->tc->elmt = ctx->tc->elmt->next;
	    auto_complete_setting(ctx, ctx->tc->elmt->text);
	}

	return;
    } else if (ctx->tc->isInCirculationModeForWhois) {
	if (ctx->tc->elmt == textBuf_tail(ctx->tc->matches)) {
	    output_error("no more matches");
	    readline_tab_comp_ctx_reset(ctx->tc);
	} else {
	    ctx->tc->elmt = ctx->tc->elmt->next;
	    auto_complete_whois(ctx, ctx->tc->elmt->text);
	}

	return;
    } else if (ctx->tc->isInCirculationModeForCmds) {
	if (ctx->tc->elmt == textBuf_tail(ctx->tc->matches)) {
	    output_error("no more matches");
	    readline_tab_comp_ctx_reset(ctx->tc);
	} else {
	    ctx->tc->elmt = ctx->tc->elmt->next;
	    auto_complete_command(ctx, ctx->tc->elmt->text);
	}

	return;
    } else if (ctx->tc->isInCirculationModeForChanUsers) {
	if (ctx->tc->elmt == textBuf_tail(ctx->tc->matches)) {
	    output_error("no more matches");
	    readline_tab_comp_ctx_reset(ctx->tc);
	} else {
	    ctx->tc->elmt = ctx->tc->elmt->next;
	    auto_complete_channel_user(ctx, ctx->tc->elmt->text);
	}

	return;
    }

    if (store_search_var(ctx) == -1) {
	output_error("cannot store search variable");
	return;
    }

    const bool is_command = (ctx->tc->search_var[0] == '/');
    const bool n_insert_greater_than_one = ctx->n_insert > 1;

    if (!strncmp(get_search_var(ctx), "/query ", 7)) {
	if (!is_irc_channel(ACTWINLABEL))
	    return;
	/* TODO: Add code */;
    } else if (!strncmp(get_search_var(ctx), "/set ", 5)) {
	char *p = & (ctx->tc->search_var[5]);

	if ((ctx->tc->matches = get_list_of_matching_settings(p)) == NULL) {
	    output_error("no magic");
	    return;
	}

	ctx->tc->elmt = textBuf_head(ctx->tc->matches);
	auto_complete_setting(ctx, ctx->tc->elmt->text);

	ctx->tc->isInCirculationModeForSettings = true;
	return;
    } else if (!strncmp(get_search_var(ctx), "/whois ", 7)) {
	if (!is_irc_channel(ACTWINLABEL))
	    return;
	/* TODO: Add code */;
    } else if (is_command) {
	char *p = & (ctx->tc->search_var[1]);

	if (!n_insert_greater_than_one ||
	    (ctx->tc->matches = get_list_of_matching_commands(p)) == NULL) {
	    output_error("no magic");
	    return;
	}

	ctx->tc->elmt = textBuf_head(ctx->tc->matches);
	auto_complete_command(ctx, ctx->tc->elmt->text);

	ctx->tc->isInCirculationModeForCmds = true;
	return;
    } else if (is_irc_channel(ACTWINLABEL)) {
	ctx->tc->matches =
	    get_list_of_matching_channel_users(ACTWINLABEL, get_search_var(ctx));

	if (ctx->tc->matches == NULL) {
	    output_error("no magic");
	    return;
	}

	ctx->tc->elmt = textBuf_head(ctx->tc->matches);
	auto_complete_channel_user(ctx, ctx->tc->elmt->text);

	ctx->tc->isInCirculationModeForChanUsers = true;
	return;
    }
}
