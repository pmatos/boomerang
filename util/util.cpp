/**
 * \file
 * \brief Contains miscellaneous functions that don't belong to any particular
 *        subsystem of UQBT.
 *
 * \authors
 * Copyright (C) 2000-2001, The University of Queensland
 * \authors
 * Copyright (C) 2001, Sun Microsystems, Inc
 * \authors
 * Copyright (C) 2002, Trent Waddington
 *
 * \copyright
 * See the file "LICENSE.TERMS" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "util.h"

#include <unistd.h>
#include <fcntl.h>

#include <iomanip>          // For setw
#include <string>
#include <sstream>

#include <cctype>
#include <cstdio>
#include <cstring>

/**
 * \brief           Append an int to a string.
 * \param[in] s     The string to append to.
 * \param[in] i     The integer whose ascii representation is to be appended.
 * \returns         A copy of the modified string.
 */
std::string operator+(const std::string &s, int i)
{
	static char buf[50];
	std::string ret(s);

	sprintf(buf, "%d", i);
	return ret.append(buf);
}

/**
 * \brief           Return a string the same as the input string, but with the
 *                  first character capitalised.
 * \param[in] s     The string to capitalise.
 * \returns         A copy of the modified string.
 */
std::string initCapital(const std::string &s)
{
	std::string res(s);
	res[0] = toupper(res[0]);
	return res;
}

/**
 * \brief           Check if a file name has the given extension.
 * \param[in] s     String representing a file name (e.g. string("foo.c")).
 * \param[in] ext   The extension (e.g. ".o").
 * \retval true     The file name has the extension.
 * \retval false    Otherwise.
 */
bool hasExt(const std::string &s, const char *ext)
{
	std::string tailStr = std::string(".") + std::string(ext);
	size_t i = s.rfind(tailStr);
	if (i == std::string::npos) {
		return false;
	} else {
		size_t sLen = s.length();
		size_t tailStrLen = tailStr.length();
		return ((i + tailStrLen) == sLen);
	}
}

/**
 * \brief           Change the extension of the given file name.
 * \param[in] s     String representing the file name to be modified (e.g.
 *                  string("foo.c")).
 * \param[in] ext   The new extension (e.g. ".o").
 * \returns         The converted string (e.g. "foo.o").
 */
std::string changeExt(const std::string &s, const char *ext)
{
	size_t i = s.rfind(".");
	if (i == std::string::npos) {
		return s + ext;
	} else {
		return s.substr(0, i) + ext;
	}
}

/**
 * \brief           Returns a copy of a string with all occurences of match
 *                  replaced with rep.  (Simple version of s/match/rep/g)
 * \param[in] in    The source string.
 * \param[in] match The search string.
 * \param[in] rep   The string to replace match with.
 * \returns         The updated string.
 */
std::string searchAndReplace(const std::string &in, const std::string &match,
                             const std::string &rep)
{
	std::string result;
	size_t l, n = 0;
	while ((l = in.find(match, n)) != std::string::npos) {
		result.append(in.substr(n, l - n));
		result.append(rep);
		l += match.length();
		n = l;
	}
	result.append(in.substr(n));
	return result;
}

/**
 * \brief           Uppercase a C string.
 * \param[in] s     The string to start with.
 * \param[out] d    The string to write to (can be the same string).
 */
void upperStr(const char *s, char *d)
{
	size_t len = strlen(s);
	for (size_t i = 0; i < len; ++i)
		d[i] = toupper(s[i]);
	d[len] = '\0';
}

int lockFileRead(const char *fname)
{
	int fd = open(fname, O_RDONLY);  /* get the file descriptor */
	struct flock fl;
	fl.l_type   = F_RDLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
	fl.l_start  = 0;        /* Offset from l_whence         */
	fl.l_len    = 0;        /* length, 0 = to EOF           */
	fl.l_pid    = getpid(); /* our PID                      */
	fcntl(fd, F_SETLKW, &fl);  /* set the lock, waiting if necessary */
	return fd;
}

int lockFileWrite(const char *fname)
{
	int fd = open(fname, O_WRONLY);  /* get the file descriptor */
	struct flock fl;
	fl.l_type   = F_WRLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
	fl.l_start  = 0;        /* Offset from l_whence         */
	fl.l_len    = 0;        /* length, 0 = to EOF           */
	fl.l_pid    = getpid(); /* our PID                      */
	fcntl(fd, F_SETLKW, &fl);  /* set the lock, waiting if necessary */
	return fd;
}

void unlockFile(int fd)
{
	struct flock fl;
	fl.l_type   = F_UNLCK;  /* tell it to unlock the region */
	fcntl(fd, F_SETLK, &fl); /* set the region to unlocked */
	close(fd);
}

void escapeXMLChars(std::string &s)
{
	std::string bad = "<>&";
	const char *replace[] = { "&lt;", "&gt;", "&amp;" };
	for (size_t i = 0; i < s.size(); ++i) {
		size_t n = bad.find(s[i]);
		if (n != std::string::npos) {
			s.replace(i, 1, replace[n]);
		}
	}
}

/**
 * \brief           Turn things like newline, return, tab into \\n, \\r, \\t etc.
 * \note            Assumes a C or C++ back end...
 */
char *escapeStr(const char *str)
{
	std::ostringstream out;
	const char unescaped[] = "ntvbrfa\"";
	const char escaped[] = "\n\t\v\b\r\f\a\"";
	bool escapedSucessfully;

	// test each character
	for (; *str; ++str) {
		if (isprint(*str) && *str != '\"') {
			// it's printable, so just print it
			out << *str;
		} else { // in fact, this shouldn't happen, except for "
			// maybe it's a known escape sequence
			escapedSucessfully = false;
			for (size_t i = 0; escaped[i] && !escapedSucessfully; ++i) {
				if (*str == escaped[i]) {
					out << "\\" << unescaped[i];
					escapedSucessfully = true;
				}
			}
			if (!escapedSucessfully) {
				// it isn't so just use the \xhh escape
				out << "\\x" << std::hex << std::setfill('0') << std::setw(2) << (int)*str;
				out << std::setfill(' ');
			}
		}
	}

	char *ret = new char[out.str().size() + 1];
	strcpy(ret, out.str().c_str());
	return ret;
}
