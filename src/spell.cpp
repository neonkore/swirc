/* spell.cpp
   Copyright (C) 2023 Markus Uhlin. All rights reserved.

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

#include <hunspell/hunspell.h>

#include <climits>
#include <clocale>
#include <cwchar>
#include <cwctype>
#include <stdexcept>
#include <string>

#include "assertAPI.h"
#include "config.h"
#include "errHand.h"
#include "libUtils.h"
#include "main.h"
#include "nestHome.h"
#include "printtext.h"
#include "spell.h"
#include "strHand.h"
#ifdef UNIX
#include "swircpaths.h"
#endif

#define MAXWORDLEN 50

const char	 g_aff_suffix[] = ".aff";
const char	 g_dic_suffix[] = ".dic";

static Hunhandle *hh = nullptr;

suggestion::suggestion()
{
	this->word = nullptr;
	this->wide_word = nullptr;
}

suggestion::suggestion(const char *word)
{
	char			*lang, *encoding;
	std::string		 orig_locale("");
	std::string		 tmp_locale("");
	struct locale_info	*li;

	this->word = nullptr;
	this->wide_word = nullptr;

	if (word == nullptr || strings_match(word, ""))
		throw std::runtime_error("no word");

	li = get_locale_info(LC_CTYPE);
	if (li->lang_and_territory == nullptr || li->codeset == nullptr) {
		free_locale_info(li);
		throw std::runtime_error("get locale info error");
	} else if ((encoding = Hunspell_get_dic_encoding(hh)) == nullptr) {
		free_locale_info(li);
		throw std::runtime_error("get dic encoding error");
	}

	lang = li->lang_and_territory;
	(void) orig_locale.assign(lang).append(".").append(li->codeset);
	(void) tmp_locale.assign(lang).append(".").append(encoding);
	free_locale_info(li);

	this->word = sw_strdup(word);

	/*
	 * wide word
	 */
	mutex_lock(&g_puts_mutex);
	if (xsetlocale(LC_CTYPE, tmp_locale.c_str()) == nullptr)
		debug("temporary locale error");
	const size_t size = strlen(word) + 1;
	this->wide_word = static_cast<wchar_t *>(xcalloc(size,
	    sizeof(wchar_t)));
	if (xmbstowcs(this->wide_word, word, size - 1) == g_conversion_failed)
		this->wide_word[0] = L'\0';
	if (xsetlocale(LC_CTYPE, orig_locale.c_str()) == nullptr)
		debug("original locale error");
	mutex_unlock(&g_puts_mutex);
}

suggestion::~suggestion()
{
	if (this->word) {
		free(this->word);
		this->word = nullptr;
	}
	if (this->wide_word) {
		free(this->wide_word);
		this->wide_word = nullptr;
	}
}

const char *
suggestion::get_word(void) const
{
	return (this->word);
}

const wchar_t *
suggestion::get_wide_word(void) const
{
	return (this->wide_word);
}

void
spell_init(void)
{
	std::string aff("");
	std::string dic("");

	if (config_bool("spell_syswide", true)) {
#if defined(UNIX)
		aff.append(HUNSPELL_PATH);
		dic.append(HUNSPELL_PATH);
#elif defined(WIN32)
		aff.append(g_home_dir);
		dic.append(g_home_dir);
#endif
	} else {
		aff.append(g_home_dir);
		dic.append(g_home_dir);
	}

	aff.append(SLASH).append(Config("spell_lang")).append(g_aff_suffix);
	dic.append(SLASH).append(Config("spell_lang")).append(g_dic_suffix);

	if (hh)
		Hunspell_destroy(hh);
	if ((hh = Hunspell_create(aff.c_str(), dic.c_str())) == nullptr)
		printtext_print("err", "%s: error", __func__);
}

void
spell_deinit(void)
{
	if (hh)
		Hunspell_destroy(hh);
	hh = nullptr;
}

void
spell_destroy_suggs(std::vector<sugg_ptr> *suggs)
{
	if (suggs != nullptr) {
		std::vector<sugg_ptr>::iterator it;
		sugg_ptr ptr;

		for (it = suggs->begin(); it != suggs->end(); ++it) {
			ptr = *it;
			delete ptr;
		}
		delete suggs;
	}
}

//lint -sem(get_mbs, r_null)
static char *
get_mbs(const wchar_t *wcs)
{
	char	*out;
	size_t	 bytes_convert, size;

	size		= size_product(wcslen(wcs) + 1, MB_LEN_MAX);
	out		= static_cast<char *>(xmalloc(size));
	bytes_convert	= wcstombs(out, wcs, size - 1);

	if (bytes_convert == g_conversion_failed) {
		free(out);
		return nullptr;
	}

	out[bytes_convert] = '\0';
	return out;
}

std::vector<sugg_ptr> *
spell_get_suggs(const char *mbs, const wchar_t *wcs)
{
	char**			 list;
	int			 nsuggs;
	std::vector<sugg_ptr>	*suggs;
	sugg_ptr		 ptr;

	if (mbs == nullptr && wcs == nullptr)
		return nullptr;
	else if (mbs) {
		if (strcmp(mbs, "") == STRINGS_MATCH ||
		    (nsuggs = Hunspell_suggest(hh, &list, mbs)) < 1)
			return nullptr;
	} else if (wcs) {
		char	*word;

		if (wcscmp(wcs, L"") == STRINGS_MATCH ||
		    wcslen(wcs) > MAXWORDLEN)
			return nullptr;

		if ((word = get_mbs(wcs)) == nullptr ||
		    (nsuggs = Hunspell_suggest(hh, &list, word)) < 1) {
			free(word);
			return nullptr;
		}

		free(word);
	} else
		sw_assert_not_reached();

	suggs = new std::vector<sugg_ptr>();

	for (int i = 0; i < nsuggs; i++) {
		try {
			ptr = new suggestion(list[i]);
			suggs->push_back(ptr);
		} catch (const std::runtime_error &e) {
			delete ptr;
			debug("%s: %s", __func__, e.what());
		} catch (...) {
			delete ptr;
			debug("%s: unknown error", __func__);
		}
	}

	Hunspell_free_list(hh, &list, nsuggs);
	return suggs;
}

void
spell_test1(const char *word)
{
	std::vector<sugg_ptr> *suggs;
	std::vector<sugg_ptr>::iterator it;
	sugg_ptr ptr;

	printtext_print(nullptr, " -- %s is %s", word, (spell_word(word) ?
	    "correct" : "incorrect"));
	suggs = spell_get_suggs(word, nullptr);
	if (suggs != nullptr) {
		for (it = suggs->begin(); it != suggs->end(); ++it) {
			ptr = *it;
			printtext_print(nullptr, "%s", ptr->get_word());
		}
		spell_destroy_suggs(suggs);
	}
}

void
spell_test2(const wchar_t *word)
{
	std::vector<sugg_ptr> *suggs;
	std::vector<sugg_ptr>::iterator it;
	sugg_ptr ptr;

	printtext_print(nullptr, " -- %ls is %s", word, (spell_wide_word(word) ?
	    "correct" : "incorrect"));
	suggs = spell_get_suggs(nullptr, word);
	if (suggs != nullptr) {
		for (it = suggs->begin(); it != suggs->end(); ++it) {
			ptr = *it;
			printtext_print(nullptr, "%ls", ptr->get_wide_word());
		}
		spell_destroy_suggs(suggs);
	}
}

bool
spell_word(const char *word)
{
	if (hh == nullptr || word == nullptr ||
	    strcmp(word, "") == STRINGS_MATCH)
		return false;
	return (Hunspell_spell(hh, word) != 0 ? true : false);
}

void
spell_word_readline(volatile struct readline_session_context *ctx)
{
	UNUSED_PARAM(ctx);
}

bool
spell_wide_word(const wchar_t *word)
{
	bool	 ret;
	char	*mbs;

	if (hh == nullptr || word == nullptr ||
	    wcscmp(word, L"") == STRINGS_MATCH ||
	    wcslen(word) > MAXWORDLEN)
		return false;

	if ((mbs = get_mbs(word)) == nullptr)
		return false;

	ret = (Hunspell_spell(hh, mbs) != 0 ? true : false);
	free(mbs);

	return ret;
}
